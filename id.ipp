#include "id.hpp"
#ifdef LWE_ID_HPP

template <class T> ID::Manager UID<T>::gid;

ID ID::Invalid() { return ID(INVALID); }

ID::ID(Value arg) : value(arg) {}

ID::ID(const ID& arg) : value(arg.value) {}

ID& ID::operator=(const ID& ref) {
    value = ref.value;
    return *this;
}

ID::operator Value() const { return value; }

ID& ID::operator++() {
    ++value;
    return *this;
}

ID& ID::operator--() {
    --value;
    return *this;
}

ID ID::operator++(int) {
    ID id = ID{value};
    ++value;
    return id;
}

ID ID::operator--(int) {
    ID id = ID{value};
    --value;
    return id;
}

template <class T> void UID<T>::Generate() {
    if (id == INVALID) {
        id = gid.Generate();
    }
}

template <class T> void UID<T>::Release() {
    if (id != INVALID) {
        gid.Release(id);
        id = ID(INVALID);
    }
}

template <class T> UID<T> UID<T>::Next() { return UID(gid.Generate()); }

template <class T> UID<T> UID<T>::Unassigned() { return UID(INVALID); }

template <class T> UID<T>::UID(Value arg) : id(arg) {}

template <class T> UID<T>::UID(bool init) {
    if (init) {
        id = gid.Generate();
    }
}

template <class T> UID<T>::UID(UID&& arg) noexcept {
    id     = arg.id;
    arg.id = ID(INVALID);
}

template <class T> UID<T>::~UID() { gid.Release(id); }

template <class T> UID<T>& UID<T>::operator=(UID&& arg) noexcept {
    if (this != &arg) {
        gid.Release(id);
        id     = arg.id;
        arg.id = ID(INVALID);
    }
    return *this;
}

template <class T> UID<T>::operator ID() const { return id; }

template <class T> UID<T>::operator Value() const { return id; }

template <class T> ID UID<T>::Preview() { return gid.Preview(); }

ID::Manager::~Manager() { terminated = true; }

ID ID::Manager::Generate() {
    if (terminated) {
        return ID(INVALID);
    }

    if (cache.empty()) {
        if (next == INVALID) {
            return ID(INVALID);
        } else
            return ID{next++};
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

    if (id == INVALID) {
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

size_t ID::Set::Search(ID id) {
    return Search(id, 0, container.size());
}

size_t ID::Set::Search(ID id, size_t begin, size_t end) {
    std::vector<ID>::iterator itr = std::lower_bound(container.begin() + begin, container.begin() + end, id);
    if (itr != container.end() && *itr == id) {
        return std::distance(container.begin(), itr);
    }
    return container.size(); // induce to out of range
}

void ID::Set::Sort() {
    if (begin != end) {
        std::sort(&container[0] + begin, &container[0] + end);
    }
    begin = end = (container.size() - 1);
}

bool ID::Set::Insert(ID arg) {
    // insert
    container.push_back(arg);

    // sorted
    if (begin == end) {
        if (arg < container[end]) {
            end = container.size() - 1;
        }
    }

    // unsorted
    else {
        // smaller push back, find min
        if (arg < max) {
            end = container.size() -1;
            while (begin != 0 && arg < container[begin]) {
                --begin;
            }
        }
    }

    if (arg > max) {
        max = arg;
    }
    return true;
}

bool ID::Set::Erase(ID id) {
    if (!id) {
        return false;
    }
    size_t index = container.size(); // invlaid
    size_t last = index - 1;        // last

    // unsorted
    if (begin != end) {
        // out of sort range
        if (begin != 0 && id < container[begin]) {
            index = Search(id, 0, begin + 1);
        }

        // out of sort range
        else if (end != last && container[end] > id) {
            index = Search(id, end, index);
        }

        else Sort();
    }

    // sorted
    if (begin == end) {
        // last == max
        if (container[last] == id) {
            index = last;
            if (last) max = container[last - 1];
            else max = ID::Invalid();
        }
        else index = Search(id);
    }

    // delete
    if (index == container.size()) {
        return false;
    }

    container[index] = container[last]; // move
    container.resize(last);             // delete

    // move position
    if (index < begin) {
        begin = index;
    }
    return true;
}

size_t ID::Set::Size() {
    return container.size();
}

ID ID::Set::operator[](size_t index) {
    if (index >= container.size()) {
        return ID::Invalid();
    }
    Sort();

    return container[index];
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