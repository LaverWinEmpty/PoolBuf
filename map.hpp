#ifndef LWE_MAP_HPP
#define LWE_MAP_HPP

#include "allocator.hpp"
#include "id.hpp"

/*
    Lock: write static method
    - Enable
    - Disable
*/

template<typename T, class Mtx = SpinLock,
    size_t COUNT = EConfig::MEMORY_POOL_CHUNK_COUNT_DEFAULT,
    size_t ALIGN = EConfig::MEMORY_POOL_CHUNK_COUNT_DEFAULT>
class Map {
public:
    struct Item {
        T* instance = nullptr;

    private:
        friend class Map;
        UID<T> id     = UID<T>::Unassigned();
        size_t index  = 0;
        Map*   parent = nullptr;
    };

private:
    std::vector<ID> table;

private:
    inline static std::vector<Item>                container;
    inline static Allocator<T, void, COUNT, ALIGN> allocator;
    inline static Lock<Mtx>::Type                  mtx;

public:
    // global data iterator
    struct Iterator {
        friend class Map;
        Iterator(Item*);
        Iterator(const Iterator&);
        bool      operator==(const Iterator&) const;
        bool      operator!=(const Iterator&) const;
        Iterator& operator++();
        Iterator& operator--();
        Iterator  operator++(int);
        Iterator  operator--(int);
        T*        operator->();
        T&        operator*();
    private:
        Item* item;
    };

public:
    ~Map();

public:
    bool Exist(ID);
    bool Exist(Iterator);

public:
    ID     Insert(T&&);
    bool   Erase(ID);
    size_t Size() const;

public:
    static ID    Enable(T&&);
    static bool  Disable(ID);

public:
    static Item*    Find(ID);
    static Iterator Begin();
    static Iterator End();

public:
    T& operator[](ID);
    T& operator[](size_t);
    const T& operator[](ID) const;
    const T& operator[](size_t) const;
};

#include "map.ipp"
#endif