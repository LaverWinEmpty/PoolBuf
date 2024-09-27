#ifndef LWE_UTILITIES_ALLOCATOR_HPP
#define LWE_UTILITIES_ALLOCATOR_HPP

#include <set>
#include <stack>

#include "config.h"
#include "lock.hpp"
#include "type.h"

/*
    Lock
    - Allocate    : ptr = malloc(sizeof(T))
    - Deallocate  : free(ptr)
    - Construct   : ptr = new T
    - Deconstruct : delete ptr
    - Expand      : create block
    - Reduce      : release block

    MemoryPool<64, 8, 8>
    ┌────┬────┬────┐
    │ 40 │ 72 │ 72 │
    ├────┼────┼────┤
    │ 72 │ 72 │ 72 │
    ├────┼────┼────┤
    │ 72 │ 72 │ 72 │
    └────┴────┴────┘
    = 616 byte
    (64 + 8 + 72)

    MemoryPool<32, 8, 64>
    ┌────┬────┬────┐
    │ 64 │ 64 │ 64 │
    ├────┼────┼────┤
    │ 64 │ 64 │ 64 │
    ├────┼────┼────┤
    │ 64 │ 64 │ 64 │
    └────┴────┴────┘
    = 576 byte
    (32 + 8 = 40, padding 24)

    MemoryPool<30, 3, 0> Expand(4)
    ┌────┬────┐ ┌────┬────┐ ┌────┬────┐ ┌────┬────┐
    │ 40 │ 38 │ │ 40 │ 38 │ │ 40 │ 38 │ │ 40 │ 38 │
    ├────┼────┤ ├────┼────┤ ├────┼────┤ ├────┼────┤
    │ 38 │ 38 │ │ 38 │ 38 │ │ 38 │ 38 │ │ 38 │ 38 │
    └────┴────┘ └────┴────┘ └────┴────┘ └────┴────┘
    = 114 byte * 4 
    (30 + 8 = 38)

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

template<typename, class, size_t, size_t> class Allocator;

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
    template<typename T> struct Chunk {
        static constexpr size_t SIZE = Aligner<sizeof(T)>::ALIGN_TO_POINTER;
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
    template<typename T, class Mtx = SpinLock,
        size_t COUNT = Config::MEMORY_ALLOCATE_DEFAULT,
        size_t ALIGN = Config::MEMORY_ALIGNMENT_DEFAULT>
    struct Setting {
        using LockType = Lock<Mtx>::Type;
        static constexpr size_t CHUNK_COUNT = COUNT;
        static constexpr size_t CHUNK_SIZE  = Chunk<T>::SIZE;
        static constexpr size_t ALIGNMENT   = Aligner<ALIGN>::POWER_OF_TWO;
    };

protected:
    Memory();

protected:
    Segment*             top = nullptr;
    std::stack<Segment*> freeable;
    std::set<Segment*>   all;
    Usage                usage = { 0 };

public:
    const Usage& INFO;
};

template<typename T, class Mtx = void,
    size_t COUNT = Config::MEMORY_ALLOCATE_DEFAULT,
    size_t ALIGN = Config::MEMORY_ALIGNMENT_DEFAULT>
class Allocator: public Memory {
    using Setter = Setting<T, Mtx, COUNT, ALIGN>;

public:
    using Chunk          = Chunk<T>;
    using AlignedSegment = Aligner<Setter::ALIGNMENT, Memory::Segment>::Type;
    using AlignedChunk   = Aligner<Setter::ALIGNMENT, Memory::Chunk<T>>::Type;
    using LockType       = Setter::LockType;

public:
    static constexpr size_t CHUNK_SIZE       = Setter::CHUNK_SIZE;
    static constexpr size_t CHUNK_COUNT      = Setter::CHUNK_COUNT;
    static constexpr size_t ALIGNMENT        = Setter::ALIGNMENT;
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
    template<typename U = T> U*                   Allocate();
    template<typename U = T, typename... Args> U* Construct(Args&&...);
    template<typename U = T> void                 Deconstruct(U* ptr);
    template<typename U = void> void              Deallocate(U*);

public:
    size_t Expand(size_t = 1);
    size_t Reduce();

private:
    LockType mtx;
};

#include "allocator.ipp"
#endif