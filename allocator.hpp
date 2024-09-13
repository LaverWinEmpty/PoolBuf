#ifndef LWE_UTILITIES_ALLOCATOR_HPP
#define LWE_UTILITIES_ALLOCATOR_HPP

#include <set>
#include <stack>

#include "config.h"
#include "lock.hpp"
#include "type.h"

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
    │ metadata │ 0x00 │ data │      │ 0x00 │ data │ ... │ data │      │
    ├──────────┼──────┼──────┼──────┴──────┴──────┴ ─ ─ ┴──────┴──────┘
    │          │      │      └─> padding
    │          │      └────────> data (multiple of pointer byte)
    │          └───────────────> metadata address
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
    faster than boost under certain conditions
    stable regardless of size

    Method

    void* New()
    - return instance
    - array is impossible
    - if pool empty, call Expand (failed: return nullptr)

    Dlete(void*)
    - free
    - pool check => throw error when mismatching

    bool Expand()
    - create new block (use _aligned_malloc())
    - failed: return bool

    Reduce()
    - release unusing pool (use _aligned_free())

    GetUsage()
    - get usage inforamtion structure


    MemoryPool<SIZE, COUNT, ALIGNMENT>
    - non thread safe memory pool object class

    use

    MemoryPool<sizeof(int)> pool;
    auto p = pool.New();
    pool.Delete(p);


    Allocator<Type, COUNT, ALIGNMENT, MutexType>
    - thread safe memory static instance pool

    use

    ptr = Allocator<int>::New();
    Allocator<int>::Delete(ptr);
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
    template <size_t N> struct Chunk {
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

template <size_t SIZE, class Mtx = DisableLock,
          size_t COUNT = EConfig::MEMORY_POOL_CHUNK_COUNT_DEFAULT,
          size_t ALIGN = EConfig::MEMORY_POOL_ALIGNMENT_DEFAULT>
class Allocator : public MemPoolInfo {
public:
    static constexpr size_t ALIGNMENT = Aligner<ALIGN>::POWER_OF_TWO;

public:
    using Chunk          = Chunk<SIZE>;
    using AlignedSegment = Aligner<ALIGNMENT, MemPoolInfo::Segment>::Type;
    using AlignedChunk   = Aligner<ALIGNMENT, MemPoolInfo::Chunk<SIZE>>::Type;

public:
    static constexpr size_t CHUNK_SIZE  = MemPoolInfo::Chunk<SIZE>::SIZE;
    static constexpr size_t CHUNK_COUNT = COUNT;
    static constexpr size_t BLOCK_TOTAL_SIZE =
        sizeof(AlignedSegment) + sizeof(AlignedChunk) * CHUNK_COUNT;

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
    template <typename T, typename... Args> T* New(Args&&...);
    template <typename T> void                 Delete(T* ptr);

private:
    std::stack<Segment*> freeable;
    std::set<Segment*>   all;
    Segment*             top = nullptr;
};

#include "allocator.ipp"
#endif