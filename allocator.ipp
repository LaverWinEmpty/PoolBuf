#include "allocator.hpp"
#include "mutex"

#ifdef LWE_UTILITIES_ALLOCATOR_HPP

const MemoryInfo::Usage& MemoryInfo::GetUsage() const { return info; }

template <size_t S, size_t N, size_t A>
template<typename T, typename... Args>
T* MemoryPool<S, N, A>::New(Args&&... args) {
    T* ptr = static_cast<T*>(Malloc());
    new(ptr) T(std::forward<Args>(args)...);
    return ptr;
}

template <size_t S, size_t N, size_t A>
template<typename T>
void MemoryPool<S, N, A>::Delete(T* ptr) {
    ptr->~T();
    Free(ptr);
}


template <size_t S, size_t N, size_t A> void* MemoryPool<S, N, A>::Malloc() {
    if (top == nullptr) {
        if (freeable.empty()) {
            if (Expand() == false) {
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

template <size_t S, size_t N, size_t A> void MemoryPool<S, N, A>::Free(void* p) {
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
            
            top = top->next;
            top->prev = nullptr;
        }

        if (block->next != nullptr) {
            block->next->prev = block->prev;
            block->next = nullptr;
        }

        if (block->prev != nullptr) {
            block->prev->next = block->next;
            block->prev = nullptr;
        }

        freeable.push(block);

        ++info.block.full;
        --info.block.used;
    }
}

template <size_t S, size_t N, size_t A> bool MemoryPool<S, N, A>::Expand() {
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

        cursor->next   = &next->data;
        cursor->block = newBlock;
        cursor         = next;
    }
    cursor->next   = nullptr;
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

template <size_t S, size_t N, size_t A> void MemoryPool<S, N, A>::Reduce() {
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

template <size_t S, size_t N, size_t A> MemoryPool<S, N, A>::MemoryPool() {}

template <size_t S, size_t N, size_t A> MemoryPool<S, N, A>::~MemoryPool() {
    for (typename std::set<Segment*>::iterator itr = all.begin(); itr != all.end(); ++itr) {
        _aligned_free(*itr);
    }
}

template <typename T, size_t N, size_t A, class Mtx>
Allocator<T, N, A, Mtx>::PoolType Allocator<T, N, A, Mtx>::pool;

template <typename T, size_t N, size_t A, class Mtx>
T* Allocator<T, N, A, Mtx>::allocate(size_t unused) {
    TypeLock<PoolType, Mtx> lock;
    return pool.New<T>();
}

template <typename T, size_t N, size_t A, class Mtx>
void Allocator<T, N, A, Mtx>::deallocate(T* p, size_t unused) {
    TypeLock<PoolType, Mtx> lock;
    pool.Delete<T>(p);
}

template <typename T, size_t N, size_t A, class Mtx>
template <typename... Args>T* Allocator<T, N, A, Mtx>::New(Args&&... args) {
    TypeLock<PoolType, Mtx> lock;
    return pool.New<T>(args...);
}

template <typename T, size_t N, size_t A, class Mtx> void Allocator<T, N, A, Mtx>::Delete(T* p) {
    TypeLock<PoolType, Mtx> lock;
    pool.Delete<T>(p);
}


template <typename T, size_t N, size_t A, class Mtx> bool Allocator<T, N, A, Mtx>::Expand() {
    TypeLock<PoolType, Mtx> lock;
    return pool.Expand();
}

template <typename T, size_t N, size_t A, class Mtx> void Allocator<T, N, A, Mtx>::Reduce() {
    TypeLock<PoolType, Mtx> lock;
    pool.Reduce();
}

template <typename T, size_t N, size_t A, class Mtx>
const MemoryInfo::Usage& Allocator<T, N, A, Mtx>::GetUsage() {
    return pool.GetUsage();
}

#endif