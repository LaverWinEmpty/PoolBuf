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
    template<size_t SIZE, class Lock = DisableLock, size_t COUNT = EConfig::MEMORY_POOL_CHUNK_COUNT_DEFAULT,
             size_t ALIGN = EConfig::MEMORY_POOL_ALIGNMENT_DEFAULT>
    struct Setting {
        using LockType                      = Lock;
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

template<size_t SIZE, class Setter = Memory::Setting<SIZE>>
class Allocator: public Memory {
public:
    using Chunk          = Chunk<SIZE>;
    using AlignedSegment = Aligner<Setting::ALIGNMENT, Memory::Segment>::Type;
    using AlignedChunk   = Aligner<Setting::ALIGNMENT, Memory::Chunk<SIZE>>::Type;
    using LockType       = Setter::LockType;

public:
    static constexpr size_t ALIGNMENT        = Setter::ALIGNMENT;
    static constexpr size_t CHUNK_SIZE       = Memory::Chunk<SIZE>::SIZE;
    static constexpr size_t CHUNK_COUNT      = Setter::COUNT;
    static constexpr size_t BLOCK_TOTAL_SIZE = sizeof(AlignedSegment) + sizeof(AlignedChunk) * CHUNK_COUNT;

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
};

template<typename T, class Setter = Memory::Setting<sizeof(T)>> class Pool {
    using AllocatorType = Allocator<sizeof(T), Setter>;
    using value_type = T;


public:
    template<typename U> Pool(const Pool<U>&) noexcept;
    Pool() = default;
    void* allocate(size_t = 0);
    void  deallocate(void*, size_t = 0);

private:
    static AllocatorType allocator;
};

template<typename T, class Setting>
template<typename U>
Pool<T, Setting>::Pool(const Pool<U>&) noexcept {}

template<typename T, class Setting> void* Pool<T, Setting>::allocate(size_t) {
    return allocator.Malloc();
}

template<typename T, class Setting> void Pool<T, Setting>::deallocate(void* p, size_t) {
    allocator.Free(p);
}

template<typename T, class Setting>
Pool<T, Setting>::AllocatorType Pool<T, Setting>::allocator;


#include "allocator.ipp"
#endif