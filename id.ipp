#include "id.hpp"
#ifdef LWE_ID_HPP

template <class T> ID::Manager UID<T>::gid;

std::vector<std::optional<uint64_t>> ID::hashed = { std::hash<ID::Value>()(0) };

uint64_t ID::Hash::operator()(ID id) const {
    return *hashed[Value(id)];
}

ID ID::Invalid() { return ID(ECode::INVALID_ID); }

ID::ID(Value arg) : value(arg) {
    if (arg >= hashed.size()) {
        hashed.resize(arg + 1);
    }
    if (!hashed[arg]) {
        hashed[arg] = std::hash<Value>()(arg);
    }
}

ID::ID(const ID& arg) : value(arg.value) {}

ID& ID::operator=(const ID& ref) {
    value = ref.value;
    return *this;
}

ID::operator Value() const { return value; }

template <class T> void UID<T>::Generate() {
    if (id == ECode::INVALID_ID) {
        id = gid.Generate();
    }
}

template <class T> void UID<T>::Release() {
    if (id != ECode::INVALID_ID) {
        gid.Release(id);
        id = ID::Invalid();
    }
}

template <class T> UID<T> UID<T>::Next() { return UID(gid.Generate()); }

template <class T> UID<T> UID<T>::Unassigned() { return UID(static_cast<Value>(ECode::INVALID_ID)); }

template <class T> UID<T>::UID(Value arg) : id(arg) {}

template <class T> UID<T>::UID(bool init) {
    if (init) {
        id = gid.Generate();
    }
}

template <class T> UID<T>::UID(UID&& arg) noexcept {
    id     = arg.id;
    arg.id = ID::Invalid();
}

template <class T> UID<T>::~UID() { gid.Release(id); }

template <class T> UID<T>& UID<T>::operator=(UID&& arg) noexcept {
    if (this != &arg) {
        gid.Release(id);
        id     = arg.id;
        arg.id = ID::Invalid();
    }
    return *this;
}

template <class T> UID<T>::operator ID() const { return id; }

template <class T> UID<T>::operator Value() const { return id; }

template <class T> ID UID<T>::Preview() { return gid.Preview(); }

ID::Manager::~Manager() { terminated = true; }

ID ID::Manager::Generate() {
    if (terminated) {
        return ID::Invalid();
    }

    if (cache.empty()) {
        if (next == ECode::INVALID_ID) {
            return ID::Invalid();
        }
        else {
            return ID{next++};
        }
    }

    Value id = cache.top();
    cache.pop();
    return ID(id);
}

void ID::Manager::Release(ID id) {
    if (terminated == true) {
        return;
    }

    if (id >= next) {
        throw std::runtime_error("Disallowed or duplicate ID.");
    }

    if (id == ECode::INVALID_ID) {
        return;
    }

    // check chained value
    if (id == next - 1) {
        --next;

        // pop chained value
        while (!cache.empty()) {
            if (cache.top() == next - 1) {
                cache.pop();
                --next;
            } else
                break;
        }
    }

    else {
        cache.push(id); // wait for reuse
    }
}

ID ID::Manager::Preview() const {
    if (!cache.empty()) {
        return ID(cache.top());
    }
    return ID(next);
}

template <typename T> Identifier<T>::Identifier() : id(false) {}

template <typename T> Identifier<T>::Identifier(const T& arg) : id(true), instance(arg) {}

template <typename T> Identifier<T>::Identifier(const Identifier& arg) : Identifier(arg.instance) {}

template <typename T>
Identifier<T>::Identifier(Identifier&& arg) noexcept
    : id(std::move(arg.id)), instance(std::move(arg.instance)) {}

template <typename T> Identifier<T>& Identifier<T>::operator=(const Identifier& arg) {
    instance = arg.instance;
    return *this;
}

template <typename T> Identifier<T>& Identifier<T>::operator=(Identifier&& arg) noexcept {
    if (&arg != this) {
        id       = std::move(arg.id);
        instance = std::move(arg.instance);
    }
    return *this;
}

#endif