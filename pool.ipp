#ifdef LWE_POOL_HPP

template <typename T, size_t N, size_t A> Pool<T, N, A>::Allocator Pool<T, N, A>::allocator;

template <typename T, size_t N, size_t A>
std::vector<typename Pool<T, N, A>::Item> Pool<T, N, A>::container;

template <typename T, size_t B, size_t A>
template <typename Arg>
void Pool<T, B, A>::Item::OnCreate(Arg&& arg) {
    instance = Pool::allocator.Construct<T>(arg);
    id.Generate();
}

template <typename T, size_t B, size_t A> void Pool<T, B, A>::Item::OnRelease() {
    id.Release();
    Pool::allocator.Deconstruct(static_cast<Type*>(instance));
}

template <typename T, size_t N, size_t A>
auto Pool<T, N, A>::GetAllocator() const -> const Allocator& {
    return allocator;
}

template <typename T, size_t N, size_t A> Pool<T, N, A>::~Pool() { Clear(); }

template <typename T, size_t N, size_t A> auto Pool<T, N, A>::Search(ID id) -> Item* {
    if (id == ID::INVALID && id > container.size()) {
        return nullptr;
    }
    return &container[ID::ToIndex(id)];
}

template <typename T, size_t N, size_t A> auto Pool<T, N, A>::Begin() -> Iterator {
    Item* item = &container[0];
    for (size_t i = 0; i < container.size() && item->id == ID::INVALID; ++i) {
        ++item;
    }
    return item;
}

template <typename T, size_t N, size_t A> auto Pool<T, N, A>::End() -> Iterator {
    return Iterator(&container[0] + container.size());
}

template <typename T, size_t N, size_t A>
template <typename Arg>
ID Pool<T, N, A>::Insert(Arg&& arg) {
    ID id = Create(arg);
    Take(id);
    return id;
}

template <typename T, size_t N, size_t A>
template <typename Arg>
ID Pool<T, N, A>::Create(Arg&& arg) {
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
    item->Enable(arg);
    return item->id;
}

template <typename T, size_t N, size_t A> size_t Pool<T, N, A>::Take(ID id) {
    size_t last = converter.size();

    Item* item = Search(id);
    if (item) {
        if (item->parent != nullptr) {
            return last; // already have parent => return out of tange index
        }

        converter.push_back(id); // set last
        item->index  = last;     // set last
        item->parent = this;     // set this
    }
    return last; // itme is null => return out of range index
}

template <typename T, size_t N, size_t A> bool Pool<T, N, A>::Erase(ID id) {
    if (id > container.size()) {
        return false;
    }

    Item* item = Search(id);
    if (item->parent != this) {
        return false;
    }

    return Release(id);
}

template <typename T, size_t N, size_t A> bool Pool<T, N, A>::Release(ID id) {
    Item* item = Search(id);
    if (!item) {
        return false;
    }

    if (item->parent) {
        item->parent->Give(item->index);
    }
    item->Disable();

    return true;
}

template <typename T, size_t N, size_t A> bool Pool<T, N, A>::Give(ID id, Pool* other) {
    Item* item = Search(id);
    if (!item && item->parent != this) {
        return false;
    }
    return Give(item->index, other);
}

template <typename T, size_t N, size_t A> bool Pool<T, N, A>::Give(size_t index, Pool* other) {
    size_t last = converter.size() - 1;
    if (index > last) {
        return false;
    }

    ID temp          = converter[index]; // store
    converter[index] = converter[last];  // change
    converter.resize(last);              // delete

    // give
    container[ID::ToIndex(temp)].parent = nullptr;
    if (other) {
        other.Take(index);
    }

    return true;
}

template <typename T, size_t N, size_t A> bool Pool<T, N, A>::Lost(ID id) {
    Item* item = Search(id);
    if (item && item->parent) {
        item->parent->Give(item->index);
        return true;
    }
    return false;
}

template <typename T, size_t N, size_t A> T* Pool<T, N, A>::Find(ID id) {
    Item* item = Search(id);
    if (item && item->id) {
        return item->instance;
    }
    return nullptr;
}

template <typename T, size_t N, size_t A> size_t Pool<T, N, A>::Size() const {
    return converter.size();
}

template <typename T, size_t N, size_t A> void Pool<T, N, A>::Sort() {
    std::sort(converter.begin(), converter.end());

    size_t loop = converter.size();
    for (size_t i = 0; i < loop; ++i) {
        container[ID::ToIndex(converter[i])].index = i; // remapping
    }
}

template <typename T, size_t N, size_t A> bool Pool<T, N, A>::Exist(ID id) const {
    Item* item = Search(id);
    return item && item->id && item->parent == this;
}

template <typename T, size_t N, size_t A> void Pool<T, N, A>::Clear() {
    size_t loop = converter.size();
    for (size_t i = 0; i < loop; ++i) {
        container[ID::ToIndex(converter[i])].Disable();
    }
    converter.resize(0);
    allocator.Reduce();
}

template <typename T, size_t N, size_t A> auto Pool<T, N, A>::Owner(ID id) -> Pool* {
    Item* item = Search();
    if (item && item->id) {
        return item->parent;
    }
    return nullptr;
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

    allocator.Reduce();
}

template <typename T, size_t N, size_t A> T* Pool<T, N, A>::operator[](ID id) {
    Item* item = Search(id);
    if (item->parent != this) {
        return nullptr;
    }
    return item->instance;
}

template <typename T, size_t N, size_t A> T* Pool<T, N, A>::operator[](size_t index) {
    if (index >= converter.size()) return nullptr;
    return container[ID::ToIndex(converter[index])].instance;
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

template <typename T, size_t N, size_t A>
bool Pool<T, N, A>::Iterator::operator==(const Pool& pool) const {
    return item->parent == &pool;
}

template <typename T, size_t N, size_t A>
bool Pool<T, N, A>::Iterator::operator!=(const Pool& pool) const {
    return item->parent != &pool;
}

template <typename T, size_t N, size_t A>
bool Pool<T, N, A>::Iterator::operator==(const Pool* const pool) const {
    return item->parent == pool;
}

template <typename T, size_t N, size_t A>
bool Pool<T, N, A>::Iterator::operator!=(const Pool* const pool) const {
    return item->parent != pool;
}

template <typename T, size_t N, size_t A> T* Pool<T, N, A>::Iterator::operator->() {
    return item->instance;
}

template <typename T, size_t N, size_t A> T& Pool<T, N, A>::Iterator::operator*() {
    return *item->instance;
}

template <typename T, size_t N, size_t A> Pool<T, N, A>::Iterator::operator ID() const {
    return item->id;
}

#endif