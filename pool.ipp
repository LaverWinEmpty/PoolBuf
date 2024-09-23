#ifdef LWE_POOL_HPP

template <typename T, size_t N, size_t A> ID Pool<T, N, A>::Lost(size_t index) {
    size_t last = converter.size() - 1;
    if (index > last) {
        return ID::Invalid();
    }

    ID temp          = converter[index]; // store
    converter[index] = converter[last];  // change
    converter.resize(last);              // delete

    container[ID::ToIndex(temp)].parent = nullptr; // lost
    return temp;
}

template <typename T, size_t N, size_t A> auto Pool<T, N, A>::Begin() -> Iterator {
    Item* item = &container[0];
    for (size_t i = 0; i < container.size() && item->id == ID::INVALID; ++i) {
        ++item;
    }
    return Iterator(&container[0]);
}

template <typename T, size_t N, size_t A> auto Pool<T, N, A>::End() -> Iterator {
    return Iterator(&container[0] + container.size());
}

template <typename T, size_t N, size_t A> Pool<T, N, A>::~Pool() {
    size_t loop = converter.size();
    for (size_t i = 0; i < loop; ++i) {
        Item* item = &container[ID::ToIndex(converter[i])];

        item->id.Release();
        allocator.Deconstruct(item->instance);
    }
}

template <typename T, size_t N, size_t A> bool Pool<T, N, A>::Owner(const Iterator& itr) const {
    return itr.item->parent == this;
}

template <typename T, size_t N, size_t A> size_t Pool<T, N, A>::Take(ID id) {
    size_t last = converter.size();

    Item* item = Search(id);
    if (item) {
        if (item->parent != nullptr) {
            return item->index; // current index
        }

        converter.push_back(id); // set last
        item->index  = last;     // set last
        item->parent = this;     // set this
    }
    return last; // itme is null => return out of range index
}

template <typename T, size_t N, size_t A>
template <typename Arg>
ID Pool<T, N, A>::Enable(Arg&& arg) {
    ID next = UniqueID::Preview();
    if (next == ID::INVALID) {
        return ID::Invalid();
    }

    size_t index = ID::ToIndex(next);
    if (index >= container.size()) {
        if (allocator.Expand(1) == 0) {
            return ID::Invalid(); // failed call malloc()
        }
        container.resize(allocator.INFO.chunk.total);
    }

    Item* item = Search(next);

    item->instance = allocator.Construct(arg);
    item->parent   = nullptr;
    item->index    = 0;
    item->id.Generate();

    return item->id;
}

template <typename T, size_t N, size_t A> bool Pool<T, N, A>::Disable(ID id) {
    Item* item = Search(id);
    if (!item) {
        return false;
    }

    if (item->parent) {
        item->parent->Lost(item->index);
    }
    item->id.Release();
    allocator.Deconstruct(item->instance);
    return true;
}

template <typename T, size_t N, size_t A>
template <typename Arg>
ID Pool<T, N, A>::Insert(Arg&& arg) {
    ID id = Enable(arg);
    Take(id);
    return id;
}

template <typename T, size_t N, size_t A> bool Pool<T, N, A>::Erase(ID id) {
    if (id > container.size()) {
        return false;
    }

    Item* item = Search(id);
    if (item->parent != this) {
        return false;
    }

    return Disable(id);
}

template <typename T, size_t N, size_t A> T* Pool<T, N, A>::Find(ID id) {
    Item* item = Search(id);
    if (item) {
        return item->instance;
    }
    return nullptr;
}

template <typename T, size_t N, size_t A> bool Pool<T, N, A>::Exist(ID id) const {
    Item* item = Search(id);
    if (item) {
        return item->parent == this;
    }
    return false;
}

template <typename T, size_t N, size_t A> size_t Pool<T, N, A>::Size() const {
    return converter.size();
}

template <typename T, size_t N, size_t A> auto Pool<T, N, A>::Search(ID id) -> Item* {
    if (id > container.size()) {
        return nullptr;
    }

    Item* item = &container[ID::ToIndex(id)];
    if (item->id == ID::INVALID) {
        item = nullptr;
    }
    return item;
}

template <typename T, size_t N, size_t A> T* Pool<T, N, A>::operator[](ID id) {
    Item* item = Search(id);
    if (!item) {
        return item->instance;
    }
    return nullptr;
}

template <typename T, size_t N, size_t A> T* Pool<T, N, A>::operator[](size_t index) {
    if (index >= converter.size()) return nullptr;
    return container[ID::ToIndex(converter[index])];
}

template <typename T, size_t N, size_t A> void Pool<T, N, A>::Clean() {
    size_t loop = container.size();

    for (size_t i = 0; i < loop; ++i) {
        Item* item = &container[i];
        if (item->parent == nullptr && item->id != ID::INVALID) {
            item->id.Release();
            allocator.Deconstruct(item->instance);
        }
    }
}

template <typename T, size_t N, size_t A>
Pool<T, N, A>::Iterator::Iterator(Item* ptr) : item(ptr) {}

template <typename T, size_t N, size_t A>
Pool<T, N, A>::Iterator::Iterator(const Iterator& ref) : item(ref.item) {}

template <typename T, size_t N, size_t A>
auto Pool<T, N, A>::Iterator::operator=(const Iterator& ref) -> Iterator& {
    item = ref.item;
    return *this;
}

template <typename T, size_t N, size_t A> auto Pool<T, N, A>::Iterator::operator++() -> Iterator& {
    Item* end = &container[0] + container.size();
    if (item == end) return *this;
    do {
        ++item;
    } while (item != end && item->id == ID::INVALID);
    return *this;
}

template <typename T, size_t N, size_t A>
auto Pool<T, N, A>::Iterator::operator++(int) -> Iterator {
    Iterator temp = *this;
    this->operator++();
    return temp;
}

template <typename T, size_t N, size_t A>
bool Pool<T, N, A>::Iterator::operator==(const Iterator& ref) const {
    return item == ref.item;
}

template <typename T, size_t N, size_t A>
bool Pool<T, N, A>::Iterator::operator!=(const Iterator& ref) const {
    return item != ref.item;
}

template <typename T, size_t N, size_t A> T* Pool<T, N, A>::Iterator::operator->() {
    return item->instance;
}

template <typename T, size_t N, size_t A> T& Pool<T, N, A>::Iterator::operator*() {
    return *item->instance;
}

#endif