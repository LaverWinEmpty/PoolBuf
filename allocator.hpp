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
    ┌──────────┬──────┬──────┬──────┬──────┬──────┬ ─ ─ ┬──────┬──────┐
    │ metadata │ data │ 0x00 │      │ data │ 0x00 │ ... │ 0x00 │      │
    ├──────────┼──────┼──────┼──────┴──────┴──────┴ ─ ─ ┴──────┴──────┘
    │          │      │      └─> padding
    │          │      └────────> metadata address
    │          └───────────────> instance
    │
    ├ ─ 8 byte: used object counter
    ├ ─ 8 byte: next object pointer
    ├ ─ 8 byte: next block pointer
    ├ ─ 8 byte: prev block pointer
    ├ ─ 8 byte: parent pool pointer
    ├ ─ n byte: padding
    └─> total 40 byte
*/


/*
    base
*/
class MemPoolInfo {
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
    struct Segment {
        void*    pool;
        void*    head;
        size_t   used;
        Segment* next;
        Segment* prev;
    };

public:
    template<size_t N> struct Chunk {
        static constexpr size_t SIZE = sizeof(void*) * ((N + sizeof(void*) - 1) / sizeof(void*));
        union {
            Block<SIZE> data;
            void*       next;
        };
        Segment* block;
    };

protected:
    MemPoolInfo();

public:
    const Usage& GetUsage() const;

protected:
    Usage info;
};

template<size_t SIZE, class Mtx = DisableLock, size_t COUNT = EConfig::MEMORY_POOL_CHUNK_COUNT_DEFAULT,
         size_t ALIGN = EConfig::MEMORY_POOL_ALIGNMENT_DEFAULT>
class Allocator: public MemPoolInfo {
public:
    static constexpr size_t ALIGNMENT = Aligner<ALIGN>::POWER_OF_TWO;

public:
    using Chunk          = Chunk<SIZE>;
    using AlignedSegment = Aligner<ALIGNMENT, MemPoolInfo::Segment>::Type;
    using AlignedChunk   = Aligner<ALIGNMENT, MemPoolInfo::Chunk<SIZE>>::Type;

public:
    static constexpr size_t CHUNK_SIZE       = MemPoolInfo::Chunk<SIZE>::SIZE;
    static constexpr size_t CHUNK_COUNT      = COUNT;
    static constexpr size_t BLOCK_TOTAL_SIZE = sizeof(AlignedSegment) + sizeof(AlignedChunk) * CHUNK_COUNT;

public:
    using LockType = Mtx;

public:
    Allocator();
    ~Allocator();

private:
    void* GetChunck();
    void  ReleaseChunk(void*);
    bool  NewBlock();
    void  FreeBlock();

public:
    void*  Malloc();
    void   Free(void*);
    size_t Expand(size_t = 1);
    size_t Reduce();

public:
    template<typename T, typename... Args> T* New(Args&&...);
    template<typename T> void                 Delete(T* ptr);

private:
    std::stack<Segment*> freeable;
    std::set<Segment*>   all;
    Segment*             top = nullptr;
};

template<typename T, class Lock = SpinLock, class Allocator = Allocator<sizeof(T), void,
    EConfig::MEMORY_POOL_CHUNK_COUNT_DEFAULT, EConfig::MEMORY_POOL_ALIGNMENT_DEFAULT>>
class Pool {
    using AllocatorType = Allocator;
    using value_type = T;


public:
    template<typename U> Pool(const Pool<U>&) noexcept;
    Pool() = default;
    void* allocate(size_t = 0);
    void  deallocate(void*, size_t = 0);

private:
    static AllocatorType allocator;
};

template<typename T, class Lock, class Allocator>
template<typename U>
Pool<T, Lock, Allocator>::Pool(const Pool<U>&) noexcept {}

template<typename T, class Lock, class Allocator> void* Pool<T, Lock, Allocator>::allocate(size_t) {
    return allocator.Malloc();
}

template<typename T, class Lock, class Allocator> void Pool<T, Lock, Allocator>::deallocate(void* p, size_t) {
    allocator.Free(p);
}

template<typename T, class Lock, class Allocator>
Pool<T, Lock, Allocator>::AllocatorType Pool<T, Lock, Allocator>::allocator;


#include "allocator.ipp"
#endif