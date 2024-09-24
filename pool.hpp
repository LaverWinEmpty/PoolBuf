#ifndef LWE_POOL_HPP
#define LWE_POOL_HPP

#include "allocator.hpp"
#include "id.hpp"

template <typename T, size_t POOL_CHUNK_COUNT = EConfig::MEMORY_POOL_CHUNK_COUNT_DEFAULT,
          size_t POOL_ALIGNMENT = EConfig::MEMORY_POOL_ALIGNMENT_DEFAULT>
class Pool {
public:
    using Allocator = Allocator<T, void, POOL_CHUNK_COUNT, POOL_ALIGNMENT>;
    using UniqueID  = UID<Pool<T>>;

    struct Item {
        friend class Pool;
        friend struct Iterator;
        T* instance;

    private:
        Pool*    parent;
        size_t   index;
        UniqueID id = UniqueID::Unassigned();
    };

private:
    inline static Allocator         allocator;
    inline static std::vector<Item> container;
    std::vector<ID>                 converter;

public:
    struct Iterator {
        friend class Pool;
        Iterator(Item*);
        Iterator(const Iterator&);
        Iterator& operator=(const Iterator&);
        Iterator& operator++();
        Iterator  operator++(int);
        bool      operator==(const Iterator&) const;
        bool      operator!=(const Iterator&) const;
        bool      operator==(const Pool&) const;
        bool      operator!=(const Pool&) const;
        bool      operator==(const Pool* const) const;
        bool      operator!=(const Pool* const) const;
        T*        operator->();
        T&        operator*();

    private:
        Item* item;
    };

public:
    static Iterator Begin();
    static Iterator End();

public:
    ~Pool();

public:
    bool Owner(const Iterator&) const;

public:
    template <typename Arg> ID        Insert(Arg&&); // insert
    template <typename Arg> static ID Enable(Arg&&); // allocated
    size_t                            Take(ID);      // set owner

public:
    bool        Erase(ID);    // erase
    static bool Disable(ID);  // deallocate
    ID          Lost(size_t); // reset owner

public:
    static T* Find(ID);        // find, null: not found
    bool      Exist(ID) const; // check owner
    size_t    Size() const;    // size

private:
    static Item* Search(ID); // get item, null: not found

public:
    static void Clean(); // delete parent is null items

public:
    T* operator[](ID);
    T* operator[](size_t);
};

#include "pool.ipp"
#endif