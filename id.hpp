#ifndef LWE_ID_HPP
#define LWE_ID_HPP

#include <algorithm>
#include <stdexcept>
#include <queue>

// default id
struct ID {
public:
    using Value = size_t;

public:
    static constexpr Value INVALID = 0;
    static ID              Invalid();

public:
    explicit ID(Value = INVALID);
    ID(const ID&);
    ID& operator=(const ID&);
    operator Value() const;

public:
    ID& operator++();
    ID& operator--();
    ID  operator++(int);
    ID  operator--(int);

private:
    Value value;

public:
    class Manager {
    public:
        using Value                    = ID::Value;
        using Cache                    = std::priority_queue<Value>;
        static constexpr Value INVALID = ID::INVALID;

    public:
        ~Manager();

    public:
        ID   Generate();
        void Release(ID);
        ID   Preview() const;

    private:
        Value next = 1;
        Cache cache;
        bool  terminated = false;
    };

public:
    class Set {
    private:
        void   Sort();
        size_t Search(ID);
        size_t Search(ID, size_t, size_t);

    public:
        bool   Insert(ID);
        bool   Erase(ID);
        size_t Size();

    public:
        ID operator[](size_t);

    private:
        std::vector<ID> container;
        size_t          begin = 0; // sort begin position
        size_t          end   = 0; // sort end postion;
        Value           max   = INVALID;
    };
};

// unique id
template <class T = void> class UID {
public:
    using Value                    = ID::Value;
    static constexpr Value INVALID = ID::INVALID;

private:
    UID(Value);

public:
    static UID Next();
    static UID Unassigned();

public:
    UID(bool = true);
    UID(UID&&) noexcept;
    ~UID();

public:
    UID& operator=(UID&&) noexcept;
    UID& operator=(UID&) = delete;

public:
    operator ID() const;
    operator Value() const;

public:
    static ID Preview();

private:
    ID id;

public:
    void Release();
    void Generate();

private:
    static ID::Manager gid; // grouped id
};

template <typename T> struct Identifier {
    Identifier();
    Identifier(const T&);
    Identifier(const Identifier&);
    Identifier(Identifier&&) noexcept;
    Identifier& operator=(const Identifier&);
    Identifier& operator=(Identifier&&) noexcept;

    UID<T> id;
    T      instance;
};

#include "id.ipp"
#endif