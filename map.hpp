#include "id.hpp"
#include "allocator.hpp"
#include "singleton.hpp"

// pooled PooledStorage
template<typename T, class Setter = Memory::Setting<sizeof(T)>> class PooledStorage {
    using UID = UID<T, void>;
    struct Item {
        UID uid = UID::Unassigned();
        T*  ptr = nullptr;
    };

    using Allocator = Singleton<Allocator<sizeof(T), Setter>>;
    
public:
    PooledStorage() {
        if (Allocator::Instance() == nullptr) {
            TypeLock<PooledStorage<T>, Setter::LockType> _;
            if (Allocator::Instance() == nullptr) {
                Allocator::CreateInstance();
            }
        }
    }

public:
    ID Insert(T&& arg) {
        TypeLock<PooledStorage<T>, Setter::LockType> _;
        
        const Memory::Usage& usage = Allocator::Instance()->GetUsage();

        if(usage.chunk.usable == 0) {
            if (Allocator::Instance()->Expand() == false) {
                return ID::Invalid();
            }
            items.resize(usage.chunk.total);
            owner.resize(usage.chunk.total);
        }
        T* ptr = Allocator::Instance()->New<T>(arg);
        
        ID next = UID::Preview();
        if(next == ID::INVALID) {
            return ID::Invalid();
        }
        size_t index = ID::Indexing(next);

        items[index].uid.Generate();
        items[index].ptr = ptr;

        owner[index] = this;

        ++size;
        return next;
    }

public:
    bool Erase(ID id) {
        TypeLock<PooledStorage<T>, Setter::LockType> _;

        size_t index = ID::Indexing(id);
        if (items[index].uid == ID::INVALID) {
            return false;
        }

        Allocator::Instance()->Delete<T>(items[index].ptr);
        items[index].ptr = nullptr;
        items[index].uid.Release();

        owner[index] = nullptr;

        --size;

        const Memory::Usage& usage = Allocator::Instance()->GetUsage();
        if(usage.chunk.usable >= (Setter::CHUNK_COUNT * 3)) {
            Allocator::Instance()->Reduce();
        }

        return true;
    }

public:
    size_t Size() { return size; }

private:
    T* Test(const ID& id) const {
        size_t index = ID::Indexing(id);
        if (owner[index] != this) {
            return nullptr;
        }
        return items[index].ptr;
    }

public:
    T& operator[](const ID& id) { 
        T* ptr = Test(id);
        if(ptr) return *ptr;
        throw std::out_of_range("Not found.");
    }

private:
    size_t size = 0;

private:
    static std::vector<Item>  items;
    static std::vector<void*> owner;
};


template<typename T, class Set> std::vector<typename PooledStorage<T, Set>::Item> PooledStorage<T, Set>::items;
template<typename T, class Set> std::vector<void*> PooledStorage<T, Set>::owner;
