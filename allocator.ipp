#include "allocator.hpp"

#ifdef LWE_UTILITIES_ALLOCATOR_HPP

const MemPoolInfo::Usage& MemPoolInfo::GetUsage() const { return info; }

template <size_t S, class M, size_t N, size_t A> void* Allocator<S, M, N, A>::Malloc() {
    TypeLock<Allocator<S, M, N, A>, M> locking;
    return GetChunck();
}

template <size_t S, class M, size_t N, size_t A> void Allocator<S, M, N, A>::Free(void* ptr) {
    TypeLock<Allocator<S, M, N, A>, M> locking;
    ReleaseChunk(ptr);
}

template <size_t S, class M, size_t N, size_t A>
template <typename T, typename... Args>
T* Allocator<S, M, N, A>::New(Args&&... args) {
    TypeLock<Allocator<S, M, N, A>, M> locking;

    T* ptr = static_cast<T*>(GetChunck());
    new (ptr) T(std::forward<Args>(args)...);
    return ptr;
}

template <size_t S, class M, size_t N, size_t A>
template <typename T>
void Allocator<S, M, N, A>::Delete(T* ptr) {
    TypeLock<Allocator<S, M, N, A>, M> locking;

    ptr->~T();
    ReleaseChunk(ptr);
}

template <size_t S, class M, size_t N, size_t A>
size_t Allocator<S, M, N, A>::Expand(size_t count) {
    size_t created = 0;

    while (created < count) {
        TypeLock<Allocator<S, M, N, A>, M> locking;
        if (NewBlock() == false) {
            break;
        }
        ++created;
    }
    return created;
}

template <size_t S, class M, size_t N, size_t A> size_t Allocator<S, M, N, A>::Reduce() {
    TypeLock<Allocator<S, M, N, A>, M> locking;

    size_t before = freeable.size();
    FreeBlock();
    return before - freeable.size();
}

template <size_t S, class M, size_t N, size_t A> void* Allocator<S, M, N, A>::GetChunck() {
    if (top == nullptr) {
        if (freeable.empty()) {
            if (NewBlock() == false) {
                return nullptr;
            }
        }

        else {
            top = freeable.top();
            freeable.pop();
        }
    }

    void* ret = top->head;
    top->head = *static_cast<void**>(ret);

    ++top->used;

    ++info.chunk.used;
    --info.chunk.usable;

    // first
    if (top->used == 1) {
        --info.block.full;
        ++info.block.used;
    }

    // last
    if (top->used == N) {
        Segment* fulled = top;

        // new head
        top = top->next;
        if (top) top->prev = nullptr;

        // unlink
        fulled->next = nullptr;
        fulled->prev = nullptr;

        ++info.block.empty;
        --info.block.used;
    }

    return ret;
}

template <size_t S, class M, size_t N, size_t A> void Allocator<S, M, N, A>::ReleaseChunk(void* p) {
    Chunk*   ptr   = static_cast<Chunk*>(p);
    Segment* block = ptr->block;

    // check
    if (this != block->pool) {
        throw std::runtime_error("IT DOES NOT BELONG.");
    }

    ptr->next   = block->head; // linking
    block->head = p;           // push

    --block->used;
    --info.chunk.used;
    ++info.chunk.usable;

    // empty -> usable
    if (block->used == N - 1) {
        if (!top) {
            top = block;
        } else {
            block->next       = top;   // new head
            top               = block; // stack push
            block->next->prev = block; // double linked
        }

        --info.block.empty;
        ++info.block.used;
    }

    // full -> unlink
    if (block->used == 0) {
        if (block == top) {
            if (block->next == nullptr) {
                return;
            }

            top       = top->next;
            top->prev = nullptr;
        }

        if (block->next != nullptr) {
            block->next->prev = block->prev;
            block->next       = nullptr;
        }

        if (block->prev != nullptr) {
            block->prev->next = block->next;
            block->prev       = nullptr;
        }

        freeable.push(block);

        ++info.block.full;
        --info.block.used;
    }
}

template <size_t S, class M, size_t N, size_t A> bool Allocator<S, M, N, A>::NewBlock() {
    Segment* newBlock = static_cast<Segment*>(_aligned_malloc(BLOCK_TOTAL_SIZE, ALIGNMENT));
    if (!newBlock) {
        return false;
    }
    all.insert(newBlock);

    AlignedChunk* cursor =
        reinterpret_cast<AlignedChunk*>(reinterpret_cast<AlignedSegment*>(newBlock) + 1);

    newBlock->pool = this;
    newBlock->next = nullptr;
    newBlock->prev = nullptr;
    newBlock->used = 0;
    newBlock->head = (&cursor->data);

    for (size_t index = 0; index < CHUNK_COUNT - 1; ++index) {
        AlignedChunk* next = cursor + 1;

        cursor->next  = &next->data;
        cursor->block = newBlock;
        cursor        = next;
    }
    cursor->next  = nullptr;
    cursor->block = newBlock;

    info.block.total += 1;
    info.block.full += 1;
    info.chunk.total += CHUNK_COUNT;
    info.chunk.usable += CHUNK_COUNT;

    info.block.byte += BLOCK_TOTAL_SIZE - sizeof(AlignedSegment);
    info.chunk.byte += CHUNK_SIZE * CHUNK_COUNT;

    if (top)
        freeable.push(newBlock);
    else
        top = newBlock;

    return true;
}

template <size_t S, class M, size_t N, size_t A> void Allocator<S, M, N, A>::FreeBlock() {
    while (freeable.empty() == false) {
        all.erase(freeable.top());

        _aligned_free(freeable.top());
        freeable.pop();

        info.block.full -= 1;
        info.block.total -= 1;
        info.chunk.usable -= CHUNK_COUNT;
        info.chunk.total -= CHUNK_COUNT;

        info.block.byte -= BLOCK_TOTAL_SIZE - sizeof(AlignedSegment);
        info.chunk.byte -= CHUNK_SIZE * CHUNK_COUNT;
    }
}

template <size_t S, class M, size_t N, size_t A>
Allocator<S, M, N, A>::Allocator() : MemPoolInfo() {}

template <size_t S, class M, size_t N, size_t A> Allocator<S, M, N, A>::~Allocator() {
    for (typename std::set<Segment*>::iterator itr = all.begin(); itr != all.end(); ++itr) {
        _aligned_free(*itr);
    }
}

MemPoolInfo::MemPoolInfo() : info{0} {}

#endif