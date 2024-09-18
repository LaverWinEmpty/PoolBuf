#ifdef LWE_MAP_HPP

template<typename T, class Mtx, size_t COUNT, size_t ALIGN>
Map<T, Mtx, COUNT, ALIGN>::AllocatorType Map<T, Mtx, COUNT, ALIGN>::pool;

template<typename T, class Mtx, size_t COUNT, size_t ALIGN>
std::vector<typename Map<T, Mtx, COUNT, ALIGN>::Item> Map<T, Mtx, COUNT, ALIGN>::items;

template<typename T, class Mtx, size_t COUNT, size_t ALIGN> std::vector<void*> Map<T, Mtx, COUNT, ALIGN>::owner;

template<typename T, class Mtx, size_t COUNT, size_t ALIGN>
Map<T, Mtx, COUNT, ALIGN>::Iterator::Iterator(size_t index): index(index) {}

template<typename T, class Mtx, size_t COUNT, size_t ALIGN>
Map<T, Mtx, COUNT, ALIGN>::Iterator::Iterator(const Iterator& ref): index(ref.index) {}

template<typename T, class Mtx, size_t COUNT, size_t ALIGN>
auto Map<T, Mtx, COUNT, ALIGN>::Iterator::operator=(const Iterator& ref) -> Iterator& {
    index = ref.index;
    return *this;
}

template<typename T, class Mtx, size_t COUNT, size_t ALIGN>
bool Map<T, Mtx, COUNT, ALIGN>::Iterator::operator==(const Iterator& ref) {
    return index == ref.index;
}

template<typename T, class Mtx, size_t COUNT, size_t ALIGN>
bool Map<T, Mtx, COUNT, ALIGN>::Iterator::operator!=(const Iterator& ref) {
    return index != ref.index;
}

template<typename T, class Mtx, size_t COUNT, size_t ALIGN>
auto Map<T, Mtx, COUNT, ALIGN>::Iterator::operator++() -> Iterator& {
    if(index >= items.size()) return *this;
    do {
        ++index;
    } while(index < items.size() && items[index].id == ID::INVALID);
    return *this;
}

template<typename T, class Mtx, size_t COUNT, size_t ALIGN>
auto Map<T, Mtx, COUNT, ALIGN>::Iterator::operator--() -> Iterator& {
    if(index == 0) return *this;
    do {
        --index;
    } while(index > 0 && items[index].id == ID::INVALID);
    return *this;
}

template<typename T, class Mtx, size_t COUNT, size_t ALIGN>
auto Map<T, Mtx, COUNT, ALIGN>::Iterator::operator++(int) -> Iterator {
    Iterator temp = *this;
    ++(*this);
    return temp;
}

template<typename T, class Mtx, size_t COUNT, size_t ALIGN>
auto Map<T, Mtx, COUNT, ALIGN>::Iterator::operator--(int) -> Iterator {
    Iterator temp = *this;
    --(*this);
    return temp;
}

template<typename T, class Mtx, size_t COUNT, size_t ALIGN> T& Map<T, Mtx, COUNT, ALIGN>::Iterator::operator*() {
    return *items[index].ptr;
}

template<typename T, class Mtx, size_t COUNT, size_t ALIGN> T* Map<T, Mtx, COUNT, ALIGN>::Iterator::operator->() {
    return items[index].ptr;
}

template<typename T, class Mtx, size_t COUNT, size_t ALIGN> Map<T, Mtx, COUNT, ALIGN>::~Map() {
    for(typename std::unordered_map<size_t, UID*>::iterator itr = mine.begin(); itr != mine.end(); ++itr) {
        itr->second->Release();
    }
}

template<typename T, class Mtx, size_t COUNT, size_t ALIGN> auto Map<T, Mtx, COUNT, ALIGN>::Begin() -> Iterator {
    size_t index = 0;
    while(items[index].id == ID::INVALID) {
        ++index;
    }
    return Iterator(index);
}

template<typename T, class Mtx, size_t COUNT, size_t ALIGN> auto Map<T, Mtx, COUNT, ALIGN>::End() -> Iterator {
    return Iterator(items.size());
}

template<typename T, class Mtx, size_t COUNT, size_t ALIGN> inline size_t Map<T, Mtx, COUNT, ALIGN>::Size() {
    return size;
}

template<typename T, class Mtx, size_t COUNT, size_t ALIGN>
bool Map<T, Mtx, COUNT, ALIGN>::IsMine(const Iterator& itr) {
    return mine.find(itr.index + 1) != mine.end();
}

template<typename T, class Mtx, size_t COUNT, size_t ALIGN> ID Map<T, Mtx, COUNT, ALIGN>::Insert(T&& arg) {
    [[maybe_unused]] Locker _;

    const Memory::Usage& usage = pool.GetUsage();

    if(usage.chunk.usable == 0) {
        if(pool.Expand() == false) {
            return ID::Invalid();
        }
        items.resize(usage.chunk.total);
        owner.resize(usage.chunk.total);
    }

    T* ptr = reinterpret_cast<T*>(pool.Construct(arg));

    ID next = UID::Preview();
    if(next == ID::INVALID) {
        return ID::Invalid();
    }
    size_t index = ID::Indexing(next);

    items[index].ptr = ptr;
    items[index].id.Generate();
    owner[index] = this;

    mine[next] = &items[index].id;

    ++size;
    return next;
}

template<typename T, class Mtx, size_t COUNT, size_t ALIGN> bool Map<T, Mtx, COUNT, ALIGN>::Erase(ID id) {
    [[maybe_unused]] Locker _;

    size_t index = ID::Indexing(id);
    if(items[index].id == ID::INVALID) {
        return false;
    }

    pool.Deconstruct(items[index].ptr);

    owner[index] = nullptr;
    mine[id]->Release();
    mine.erase(id);

    const Memory::Usage& usage = pool.GetUsage();
    if(usage.chunk.usable >= (COUNT * 3)) {
        pool.Reduce();
    }

    --size;
    return true;
}

template<typename T, class Mtx, size_t COUNT, size_t ALIGN> T& Map<T, Mtx, COUNT, ALIGN>::operator[](const ID& id) {
    size_t index = ID::Indexing(id);
    if(owner[index] == this) {
        T* ptr = items[index].ptr;
        if(ptr) return *ptr;
    }
    throw std::runtime_error("Not found.");
}

#endif