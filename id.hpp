#ifndef LWE_ID_HPP
#define LWE_ID_HPP

#include <queue>
#include <map>

#include "lock.hpp"

// default id
struct ID {
public:
    using Value                    = size_t;
    static constexpr Value INVALID = 0;

public:
    static ID     Invalid();
    static size_t Indexing(ID);

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
};

// grouped ID
template<class T> class GID {
    using Value                    = ID::Value;
    using Cache                    = std::priority_queue<Value>;
    static constexpr Value INVALID = ID::INVALID;

    template<class, class> friend class UID;

public:
    ~GID();

public:
    ID   Generate();
    void Release(ID);
    ID   Preview() const;

private:
    Value next = 1;
    Cache cache;
    bool  end = false;
};

// unique id
template<class T = void, class Mtx = DisableLock> class UID {
public:
    using Value                    = ID::Value;
    static constexpr Value INVALID = ID::INVALID;
    using LockGuardType            = TypeLock<UID<T>, Mtx>;

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
    static GID<T> gid;
};

// shared id
template<class T = void, class Mtx = DisableLock> class SID {
public:
    using Value                    = ID::Value;
    static constexpr Value INVALID = ID::INVALID;

public:
    SID();
    SID(UID<T, Mtx>&&);
    SID(const SID&);
    ~SID();

public:
    SID& operator=(const SID&);
    SID& operator=(SID&&) = delete;
    SID& operator=(UID<T, Mtx>&&) noexcept;

public:
    size_t Counting() const;

public:
    operator ID() const;
    operator Value() const;

private:
    ID id = ID(INVALID);

private:
    struct Block {
        UID<T, Mtx> uid;
        size_t      count = 0;
    };

private:
    static std::map<Value, Block> pool;
};

#include "id.ipp"
#endif