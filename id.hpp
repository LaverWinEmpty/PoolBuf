#ifndef LWE_ID_HPP
#define LWE_ID_HPP

#include <queue>

#include "lock.hpp"

// default id
struct ID {
public:
    using Value = size_t;

public:
    static constexpr Value INVALID = 0;
    static ID              Invalid();
    static ID              FromIndex(size_t);
    static size_t          ToIndex(ID);


public:
    explicit ID(Value = INVALID);
    ID(const ID&);
    ID& operator=(const ID&);
    operator Value() const;

public:
    bool operator==(const ID& id) const { return value == id.value; }
    bool operator!=(const ID& id) const { return value != id.value; }
    bool operator>(const ID& id) const { return value > id.value; }
    bool operator<(const ID& id) const { return value < id.value; }

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
        bool  end = false;
    };
};

// unique id
template<class T = void> class UID {
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

#include "id.ipp"
#endif