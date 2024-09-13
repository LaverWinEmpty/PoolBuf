#include "id.hpp"
#ifdef LWE_ID_HPP

template<class T, class Mtx> GID<T> UID<T, Mtx>::gid;

template<class T, class Mtx> std::map<typename SID<T, Mtx>::Value, typename SID<T, Mtx>::Block> SID<T, Mtx>::pool;

ID::ID(Value arg) : value(arg) { }

ID::ID(const ID& arg) : value(arg.value) { }

ID& ID::operator=(const ID& ref) {
    value = ref.value;
    return *this;
}

ID::operator Value() const {
    return value;
}

ID& ID::operator++() {
    ++value;
    return *this;
}

ID& ID::operator--() {
    --value;
    return *this;
}

ID ID::operator++(int) {
    ID id = ID{ value };
    ++value;
    return id;
}

ID ID::operator--(int) {
    ID id = ID{ value };
    --value;
    return id;
}

template<class T, class Mtx> void UID<T, Mtx>::Generate() {
    if (id == INVALID) {
        id = gid.Generate();
    }
}

template<class T, class Mtx> void UID<T, Mtx>::Release() {
    LockGuardType _();
    if (id != INVALID) {
        gid.Release(id);
        id = ID(INVALID);
    }
}

template<class T, class Mtx> UID<T, Mtx>::UID(Value arg) : id(arg) {}

template<class T, class Mtx> UID<T, Mtx>::UID(bool empty) : id(INVALID) {
    if (!empty) {
        LockGuardType _();
        id = gid.Generate();
    }
}

template<class T, class Mtx> UID<T, Mtx>::UID(UID&& arg) noexcept {
    LockGuardType _();
    id = arg.id;
    arg.id = ID(INVALID);
}

template<class T, class Mtx> UID<T, Mtx>::~UID() {
    LockGuardType _();
    if (!gid.end) {
        gid.Release(id);
    }
}

template<class T, class Mtx> UID<T, Mtx>& UID<T, Mtx>::operator=(UID&& arg) noexcept {
    LockGuardType _();
    if (this != &arg) {
        gid.Release(id);
        id = arg.id;
        arg.id = ID(INVALID);
    }
    return *this;
}

template<class T, class Mtx> UID<T, Mtx>::operator ID() const {
    return id;
}

template<class T, class Mtx> UID<T, Mtx>::operator Value() const {
    return id;
}

template<class T, class Mtx> ID UID<T, Mtx>::Preview() {
    return gid.Preview();
}

template<class T> GID<T>::~GID() {
    end = true;
}

template<class T> ID GID<T>::Generate() {
    if (cache.empty()) {
        if (next == INVALID) {
            return ID(INVALID);
        }
        else return ID{ next++ };
    }

    Value id = cache.top();
    cache.pop();
    return ID(id);
}

template<class T> void GID<T>::Release(ID id) {
    Value value = id;

    if (value >= next) {
        throw std::runtime_error("VALUE TAMPERED");
    }

    if (value == INVALID) {
        return;
    }

    if (value == next - 1) {
        --next;

        while (!cache.empty()) {
            if (cache.top() == next - 1) {
                cache.pop();
                --next;
            }
            else break;
        }
    }

    else {
        cache.push(value);
    }
}

template<class T> ID GID<T>::Preview() const {
    if (!cache.empty()) {
        return ID{ cache.top() };
    }
    return ID{ next };
}

template<class T, class Mtx> SID<T, Mtx>::SID() {
    UID<T, Mtx> uid;
    if (uid == INVALID) {
        throw std::runtime_error("ID EXCEEDED");
    }

    id = uid;
    pool[id].uid = std::move(uid);
    pool[id].count = 1;
}

template<class T, class Mtx> SID<T, Mtx>::SID(UID<T, Mtx>&& uid) {
    if (uid == INVALID) {
        throw std::runtime_error("INVALID ID");
    }

    id = uid;
    pool[id].uid = std::move(uid);
    pool[id].count = 1;
}

template<class T, class Mtx> SID<T, Mtx>::SID(const SID& sid) {
    if (sid == INVALID) {
        throw std::runtime_error("INVALID ID");
    }
    id = sid.id;

    ++pool[id].count;
}

template<class T, class Mtx> SID<T, Mtx>::~SID() {
    if (id == INVALID) {
        return;
    }

    --pool[id].count;

    if (pool[id].count == 0) {
        pool[id].uid.Release();
    }

    id = ID(INVALID);
}

template<class T, class Mtx> SID<T, Mtx>& SID<T, Mtx>::operator=(const SID& sid) {
    if (sid != INVALID) {
        this->~SID();
    }

    id = sid.id;
    ++pool[id].count;
    pool[id].uid.Generate();

    return *this;
}

template<class T, class Mtx> SID<T, Mtx>& SID<T, Mtx>::operator=(UID<T, Mtx>&& uid) noexcept {
    if (uid == INVALID) {
        return *this;
    }

    id = uid;
    pool[id].uid = uid;
    ++pool[id].count;

    return *this;
}

template<class T, class Mtx>
size_t SID<T, Mtx>::Counting() const {
    return pool[id].count;
}

template<class T, class Mtx>
SID<T, Mtx>::operator ID() const {
    return id;
}

template<class T, class Mtx>
SID<T, Mtx>::operator Value() const {
    return id;
}

#endif