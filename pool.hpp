#ifndef LWE_POOL_HPP
#define LWE_POOL_HPP

#include <algorithm>

#include "allocator.hpp"
#include "id.hpp"

template <typename T, size_t POOL_CHUNK_COUNT = EConfig::MEMORY_ALLOCATE_DEFAULT,
          size_t POOL_ALIGNMENT = EConfig::MEMORY_ALIGNMENT_DEFAULT>
class Pool {
public:
    struct Allocate {
        T _; // for new allocator instancea
    }; 

public:
    using Allocator = Allocator<Allocate, void, POOL_CHUNK_COUNT, POOL_ALIGNMENT>;
    using UniqueID  = UID<T>;

    struct Item {
        friend class Pool;
        friend struct Iterator;
        T* instance;

    public:
        template <typename Arg> void OnCreate(Arg&&);
        void                         OnRelease();

    private:
        UniqueID id  = UniqueID::Unassigned();
        size_t   ref = 0;
    };kf

public:
    class Flag {
        Flag(ID);

    public:
        size_t arr, bit;
    };

private:
    static Allocator         allocator;
    static std::vector<Item> container;
    std::vector<ID>          converter;
    std::vector<size_t>      checker;

private:
    const Allocator& GetAllocator() const;

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
        operator ID() const;

    private:
        Item* item;
    };

public:
    ~Pool();

private:
    static Item* Search(ID); // get item, null: not found

public:
    static Iterator Begin();
    static Iterator End();

public:
    template <typename Arg> static ID Create(Arg&&); // allocated
    template <typename Arg> ID        Insert(Arg&&); // insert
    size_t                            Take(ID);      // set owner

public:
    static bool Release(ID); // deallocate
    bool        Erase(ID);   // erase
    bool        Lost(ID);    // reset owner
    void        Leak();      // lost all

public:
    T*     Find(ID);     // find, null: not found
    size_t Size() const; // size
    void   Sort();       // member id sort

public:
    bool        Exist(ID) const; // check owner
    void        Clear();         // clear
    static void Clean();         // clear no onwer instance

public:
    T* operator[](ID);
    T* operator[](size_t);

public:
    static Identifier<T> Extern(ID);
    static bool          Intern(Identifier<T>&&);
    bool                 Include(Identifier<T>&&);
};

#include "pool.ipp"
#endif