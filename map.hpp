#include "id.hpp"
#include "allocator.hpp"

#include <unordered_map>

// pooled storage
template<typename T, class Mtx = SpinLock, size_t COUNT = EConfig::MEMORY_POOL_CHUNK_COUNT_DEFAULT,
         size_t ALIGN = EConfig::MEMORY_POOL_ALIGNMENT_DEFAULT>
class Map {
    using UID = UID<T>;
    struct Item {
        UID id  = UID::Unassigned();
        T*  ptr = nullptr;
    };

public:
    using AllocatorType = Allocator<T, void, COUNT, ALIGN>;
    using Locker        = TypeLock<Map<T, Mtx, COUNT, ALIGN>, Mtx>;

public:
    ~Map() {
        for(typename std::unordered_map<size_t, UID*>::iterator itr = mine.begin(); itr != mine.end(); ++itr) {
            itr->second->Release();
        }
    }

public:
    static T* Find(ID id) {
        [[maybe_unused]] TypeLock<> _;

        if(id > items.size()) {
            return nullptr;
        }
        size_t index = ID::Indexing(id);
        if(items[index].id == ID::INVALID) {
            return nullptr;
        }
        return items[index].ptr;
    }

public:
    ID Insert(T&& arg) {
        [[maybe_unused]] Locker _;

        const Memory::Usage& usage = pool.GetUsage();

        if(usage.chunk.usable == 0) {
            if(pool.Expand() == false) {
                return ID::Invalid();
            }
            items.resize(usage.chunk.total);
            owner.resize(usage.chunk.total);
        }

        T* ptr = reinterpret_cast<T*>(pool.Construct(arg));

        ID next = UID::Preview();
        if(next == ID::INVALID) {
            return ID::Invalid();
        }
        size_t index = ID::Indexing(next);

        items[index].ptr = ptr;
        items[index].id.Generate();
        owner[index] = this;

        mine[next] = &items[index].id;

        ++size;
        return next;
    }

public:
    bool Erase(ID id) {
        [[maybe_unused]] Locker _;

        size_t index = ID::Indexing(id);
        if(items[index].id == ID::INVALID) {
            return false;
        }

        pool.Deconstruct(items[index].ptr);

        mine[id]->Release();
        owner[index] = nullptr;

        const Memory::Usage& usage = pool.GetUsage();
        if(usage.chunk.usable >= (COUNT * 3)) {
            pool.Reduce();
        }

        --size;
        return true;
    }

public:
    size_t Size() { return size; }

public:
    T& operator[](const ID& id) {
        size_t index = ID::Indexing(id);
        if(owner[index] == this) {
            T* ptr = items[index].ptr;
            if(ptr) return *ptr;
        }
        throw std::runtime_error("Not found.");
    }

private:
    static AllocatorType             pool;
    static std::vector<Item>         items;
    static std::vector<void*>        owner;
    std::unordered_map<size_t, UID*> mine;
    size_t                           size = 0;
};

template<typename T, class Lock, size_t COUNT, size_t ALIGN>
Map<T, Lock, COUNT, ALIGN>::AllocatorType Map<T, Lock, COUNT, ALIGN>::pool;

template<typename T, class Lock, size_t COUNT, size_t ALIGN>
std::vector<typename Map<T, Lock, COUNT, ALIGN>::Item> Map<T, Lock, COUNT, ALIGN>::items;

template<typename T, class Lock, size_t COUNT, size_t ALIGN> std::vector<void*> Map<T, Lock, COUNT, ALIGN>::owner;
