#ifndef LWE_MAP_HPP
#define LWE_MAP_HPP

#include "allocator.hpp"
#include "id.hpp"

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
    struct Iterator {
        friend class Map;

    public:
        Iterator(size_t index);
        Iterator(const Iterator& ref);
        Iterator& operator=(const Iterator& ref);

    public:
        bool operator==(const Iterator& ref);
        bool operator!=(const Iterator& ref);
        Iterator& operator++();
        Iterator& operator--();
        Iterator operator++(int);
        Iterator operator--(int);

    public:
        T& operator*();
        T* operator->();

    private:
        size_t index = 0;
    };

public:
    ~Map();

public:
    Iterator Begin();
    Iterator End();
    bool     IsMine(const Iterator& itr);
    size_t   Size();

public:
    ID   Insert(T&& arg);
    bool Erase(ID id);
    T&   operator[](const ID& id);

private:
    static AllocatorType             pool;
    static std::vector<Item>         items;
    static std::vector<void*>        owner;
    std::unordered_map<size_t, UID*> mine;
    size_t                           size = 0;
};

#include "map.ipp"
#endif