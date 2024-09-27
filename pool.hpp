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
        T* instance = nullptr;

    public:
        template <typename Arg> void OnCreate(Arg&&);
        void                         OnRelease();

    private:
        UniqueID id  = UniqueID::Unassigned();
        size_t   ref = 0;
    };

    struct Converter {
        using Table = std::vector<ID>;
        using Indexer = std::unordered_map<ID, size_t, ID::Hash>;

        size_t operator()(ID);           // push
        ID     operator()(size_t);       // pop
        size_t operator[](ID) const;     // get
        ID     operator[](size_t) const; // get

        Table   table;
        Indexer indexer;
    } converter;

public:
    class Global {
        friend class Pool;

    public:
        struct Iterator {
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
        static Iterator Begin();
        static Iterator End();

    public:
        template <typename Arg> static ID Insert(Arg&&); // allocated
        static bool                       Erase(ID);     // deallocate
        static void                       Clear();       // clear no onwer instance

    private:
        static Item*                      Search(ID);    // get item, null: not found
    };

public:
    struct Iterator {
        friend class Pool;
        Iterator(Pool*, size_t);
        Iterator(const Iterator&);
        Iterator& operator=(const Iterator&);
        Iterator& operator++();
        Iterator  operator++(int);
        bool      operator==(const Iterator&) const;
        bool      operator!=(const Iterator&) const;
        T*        operator->();
        T&        operator*();
        operator ID() const;

    private:
        Pool*  ref;
        size_t index;
    };

public:
    ~Pool();

public:
    template <typename Arg> ID Insert(Arg&&); // insert
    size_t                     Take(ID);      // set ownership

public:
    bool Erase(ID); // erase
    bool Lost(ID);  // reset ownership
    void Leak();    // lost all
    void Clear();   // clear

public:
    T*     Find(ID);
    bool   Exist(ID) const;
    size_t Size() const;
    size_t GetIndex(ID) const;
    ID     GetID(size_t) const;

public:
    Iterator Begin();
    Iterator End();

public:
    T* operator[](ID);
    T* operator[](size_t);

private:
    static Allocator         allocator;
    static std::vector<Item> container;

public:
    static Global global;
};

#include "pool.ipp"
#endif