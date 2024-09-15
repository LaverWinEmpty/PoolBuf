#ifndef LWE_UTILITIES_ALLOCATOR_HPP
#define LWE_UTILITIES_ALLOCATOR_HPP

#include <set>
#include <stack>

#include "config.h"
#include "lock.hpp"
#include "type.h"
#include "singleton.hpp"

/*
    MemoryPool<64, 8, 8>
    ┌────┬────┬────┐
    │ 40 │ 72 │ 72 │
    ├────┼────┼────┤
    │ 72 │ 72 │ 72 │
    ├────┼────┼────┤
    │ 72 │ 72 │ 72 │
    └────┴────┴────┘
    = 616 byte

    MemoryPool<32, 8, 64>
    ┌────┬────┬────┐
    │ 64 │ 64 │ 64 │
    ├────┼────┼────┤
    │ 64 │ 64 │ 64 │
    ├────┼────┼────┤
    │ 64 │ 64 │ 64 │
    └────┴────┴────┘
    = 576 byte

    detail
    head
    ↓
    ┌──────────┬───────┬─────┬─────┬───────┬─────┬ ─ ─ ┬─────┬─────┐
    │ metadata │ data  │ 0x0 │     │ chunk │ 0x0 │ ... │ 0x0 │     │
    ├──────────┼───────┼─────┼─────┼───────┴─────┴ ─ ─ ┴─────┴─────┘
    │          │       │     ├ ─ ─ ┴─> padding
    │          │       ├ ─ ─ ┴───────> metadata address
    │          └ ─ ─ ─ ┴─────────────> chunk instance
    │
    ├ ─ 8 byte: used object counter
    ├ ─ 8 byte: next object pointer
    ├ ─ 8 byte: next block pointer
    ├ ─ 8 byte: prev block pointer
    ├ ─ 8 byte: parent pool pointer
    └─> total 40 + padding byte
*/

/*
    base
*/
class Memory {
public:
    struct Segment {
        void*    pool;
        void*    head;
        size_t   used;
        Segment* next;
        Segment* prev;
    };

public:
    template<size_t N> struct Chunk {
        static constexpr size_t SIZE = Aligner<N>::ALIGN_TO_POINTER;
        union {
            Block<SIZE> data;
            void*       next;
        };
        Segment* block;
    };

public:
    struct Usage {
        struct {
            size_t usable;
            size_t used;
            size_t total;
            size_t byte;
        } chunk;

        struct {
            size_t used;
            size_t full;
            size_t empty;
            size_t total;
            size_t byte;
        } block;
    };

public:
    template<size_t SIZE,
            size_t COUNT = EConfig::MEMORY_POOL_CHUNK_COUNT_DEFAULT,
            size_t ALIGN = EConfig::MEMORY_POOL_ALIGNMENT_DEFAULT>
    struct Setting {
        static constexpr size_t CHUNK_COUNT = COUNT;
        static constexpr size_t CHUNK_SIZE  = Chunk<SIZE>::SIZE;
        static constexpr size_t ALIGNMENT   = Aligner<ALIGN>::POWER_OF_TWO;
    };

protected:
    Memory();

public:
    const Usage& GetUsage() const;

protected:
    Segment*             top = nullptr;
    std::stack<Segment*> freeable;
    std::set<Segment*>   all;
    Usage                info;
};

template<size_t SIZE,
    size_t COUNT = EConfig::MEMORY_POOL_CHUNK_COUNT_DEFAULT,
    size_t ALIGN = EConfig::MEMORY_POOL_ALIGNMENT_DEFAULT>
class Pool: public Memory {
    using Setter = Setting<SIZE, COUNT, ALIGN>;;

public:
    using Chunk          = Chunk<SIZE>;
    using AlignedSegment = Aligner<Setter::ALIGNMENT, Memory::Segment>::Type;
    using AlignedChunk   = Aligner<Setter::ALIGNMENT, Memory::Chunk<SIZE>>::Type;

public:
    static constexpr size_t CHUNK_SIZE       = Setter::CHUNK_SIZE;
    static constexpr size_t CHUNK_COUNT      = Setter::CHUNK_COUNT;
    static constexpr size_t ALIGNMENT        = Setter::ALIGNMENT;
    static constexpr size_t BLOCK_TOTAL_SIZE = sizeof(AlignedSegment) + sizeof(AlignedChunk) * CHUNK_COUNT;

public:
    Pool();
    ~Pool();

private:
    void* GetChunck();
    void  ReleaseChunk(void*);
    bool  NewBlock();
    void  FreeBlock();

public:
    template<class Lock = DisableLock> void*  Malloc();
    template<class Lock = DisableLock> void   Free(void*);
    template<class Lock = DisableLock> size_t Expand(size_t = 1);
    template<class Lock = DisableLock> size_t Reduce();

public:
    template<typename T, class Lock = DisableLock, typename... Args> T* New(Args&&...);
    template<typename T, class Lock = DisableLock> void                 Delete(T* ptr);
};

template<typename T,
    size_t COUNT = EConfig::MEMORY_POOL_CHUNK_COUNT_DEFAULT,
    size_t ALIGN = EConfig::MEMORY_POOL_ALIGNMENT_DEFAULT>
class Allocator: public Pool<sizeof(T), COUNT, ALIGN> {
public:
    using value_type = T;
    template<typename U> Allocator(Allocator<U>&);
    Allocator();
    template<class Lock = DisableLock> T*   allocate(size_t = 0);
    template<class Lock = DisableLock> void deallocate(T*, size_t = 0);
};

template<typename T, size_t COUNT, size_t ALIGN>
template<class Lock>
T* Allocator<T, COUNT, ALIGN>::allocate(size_t) {
    [[maybe_unuesd]] TypeLock<Allocator<T, COUNT, ALIGN>, Lock> _;

    return static_cast<T*>(this->Malloc());
}

template<typename T, size_t COUNT, size_t ALIGN>
template<class Lock>
void Allocator<T, COUNT, ALIGN>::deallocate(T* p, size_t) {
    [[maybe_unuesd]] TypeLock<Allocator<T, COUNT, ALIGN>, Lock> _;
    this->Free(p);
}

template<typename T, size_t COUNT, size_t ALIGN> template<typename U> Allocator<T, COUNT, ALIGN>::Allocator(Allocator<U>&) {}

template<typename T, size_t COUNT, size_t ALIGN> Allocator<T, COUNT, ALIGN>::Allocator() {}

#include "allocator.ipp"
#endif