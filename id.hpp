#ifndef LWE_ID_HPP
#define LWE_ID_HPP

#include <functional>
#include <optional>
#include <queue>
#include <stdexcept>

#include "config.h"

// default id
struct ID {
public:
    using Value = size_t;

public:
    static ID Invalid();

public:
    explicit ID(Value = ECode::INVALID_ID);
    ID(const ID&);
    ID& operator=(const ID&);
    operator Value() const;

private:
    Value value;

public:
    struct Hash {
        uint64_t operator()(ID id) const;
    };

public:
    class Manager {
    public:
        using Value = ID::Value;
        using Cache = std::priority_queue<Value>;

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

private:
    static std::vector<std::optional<uint64_t>> hashed;
};

// unique id
template <class T = void> class UID {
public:
    using Value = ID::Value;

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