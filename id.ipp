#include "id.hpp"
#ifdef LWE_ID_HPP

template<class T> ID::Manager UID<T>::gid;

ID ID::Invalid() {
    return ID(INVALID);
}

size_t ID::Indexing(ID arg) {
    return arg - 1;
}

ID::ID(Value arg): value(arg) {}

ID::ID(const ID& arg): value(arg.value) {}

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

template<class T> void UID<T>::Generate() {
    if(id == INVALID) {
        id = gid.Generate();
    }
}

template<class T> void UID<T>::Release() {
    if(id != INVALID) {
        gid.Release(id);
        id = ID(INVALID);
    }
}

template<class T> UID<T> UID<T>::Next() {
    return UID(gid.Generate());
}

template<class T> UID<T> UID<T>::Unassigned() {
    return UID(INVALID);
}

template<class T> UID<T>::UID(Value arg): id(arg) {}

template<class T> UID<T>::UID(bool init) {
    if(init) {
        id = gid.Generate();
    }
}

template<class T> UID<T>::UID(UID&& arg) noexcept {
    id     = arg.id;
    arg.id = ID(INVALID);
}

template<class T> UID<T>::~UID() {
    gid.Release(id);
}

template<class T> UID<T>& UID<T>::operator=(UID&& arg) noexcept {
    if(this != &arg) {
        gid.Release(id);
        id     = arg.id;
        arg.id = ID(INVALID);
    }
    return *this;
}

template<class T> UID<T>::operator ID() const {
    return id;
}

template<class T> UID<T>::operator Value() const {
    return id;
}

template<class T> ID UID<T>::Preview() {
    return gid.Preview();
}

ID::Manager::~Manager() {
    end = true;
}

ID ID::Manager::Generate() {
    if (end) {
        return ID(INVALID);
    }

    if(cache.empty()) {
        if(next == INVALID) {
            return ID(INVALID);
        } else return ID{ next++ };
    }

    Value id = cache.top();
    cache.pop();
    return ID(id);
}

void ID::Manager::Release(ID id) {
    if (end == true) {
        return;
    }

    if(id >= next) {
        throw std::runtime_error("Value tampered.");
    }

    if(id == INVALID) {
        return;
    }

    if(id == next - 1) {
        --next;

        while(!cache.empty()) {
            if(cache.top() == next - 1) {
                cache.pop();
                --next;
            } else break;
        }
    }

    else {
        cache.push(id);
    }
}

ID ID::Manager::Preview() const {
    if(!cache.empty()) {
        return ID{ cache.top() };
    }
    return ID{ next };
}

#endif