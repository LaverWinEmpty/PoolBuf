#ifdef LWE_POOL_HPP

template <typename T, size_t N, size_t A> Pool<T, N, A>::Allocator Pool<T, N, A>::allocator;

template <typename T, size_t N, size_t A>
std::vector<typename Pool<T, N, A>::Item> Pool<T, N, A>::container;

template <typename T, size_t N, size_t A>
template <typename Arg>
void Pool<T, N, A>::Item::OnCreate(Arg&& arg) {
    instance = allocator.Allocate<T>();
    new(instance) T(arg);
    id.Generate();
}

template <typename T, size_t N, size_t A> void Pool<T, N, A>::Item::OnRelease() {
    id.Release();
    instance->~T();
    Pool::allocator.Deallocate(instance);
}

template <typename T, size_t N, size_t A>
auto Pool<T, N, A>::GetAllocator() const -> const Allocator& {
    return allocator;
}

template <typename T, size_t N, size_t A> Pool<T, N, A>::~Pool() { Clear(); }

template <typename T, size_t N, size_t A> auto Pool<T, N, A>::Search(ID id) -> Item* {
    if (id == ID::INVALID && id >= container.size()) {
        return nullptr;
    }
    return &container[id];
}

template <typename T, size_t N, size_t A> auto Pool<T, N, A>::Begin() -> Iterator {
    Item* item = &container[0];
    for (size_t i = 0; i < container.size() && item->id == ID::INVALID; ++i) {
        ++item;
    }
    return item;
}

template <typename T, size_t N, size_t A> auto Pool<T, N, A>::End() -> Iterator {
    return Iterator(&container[1] + container.size());
}

template<typename T, size_t N, size_t A>
Pool<T, N, A>::Flag::Flag(ID id): arr(id >> 8), bit(1 << (id & 0x3F)) { }

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

    while (next >= container.size()) {
        if (allocator.Expand(1) == 0) {
            return ID::Invalid(); // failed call malloc()
        }
        container.resize(1 + allocator.INFO.chunk.total);
    }

    Item* item = Search(next);
    item->OnCreate(arg);
    return item->id;
}

template <typename T, size_t N, size_t A> size_t Pool<T, N, A>::Take(ID id) {
    size_t last = converter.size();

    Item* item = Search(id);
    if (item) {
        // set check
        Flag flag = id;
        if (flag.arr >= checker.size()) {
            checker.resize(flag.arr);
        }
        else if(checker[flag.arr] & flag.bit) {
            size_t loop = converter.size();
            for (size_t i = 0; i < loop; ++i) {
                if (converter[i] == id) {
                    return i;
                }
            }
            throw std::logic_error("Set flag but not exist.");
        }
        checker[flag.arr] |= flag.bit;

        converter.push_back(id); // set last
        item->index = last;      // set last
        item->parent = this;     // set this
    }
    return last; // itme is null => return out of range index
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

template <typename T, size_t N, size_t A> bool Pool<T, N, A>::Erase(ID id) {
    if (id >= container.size()) {
        return false;
    }

    Item* item = Search(id);
    if (item->parent != this) {
        return false;
    }

    return Release(id);
}


template <typename T, size_t N, size_t A> bool Pool<T, N, A>::Lost(ID id) {
    Item* item = Search(id);
    if (item == nullptr) {
        return false;
    }

    Flag flag = id;
    if (flag.arr >= checker.size()) {
        return false;
    }

    size_t last = converter.size() - 1;


    ID temp          = converter[flag]; // store
    converter[index] = converter[last];  // change
    converter.resize(last);              // delete

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
        container[converter[i]].index = i; // remapping
    }
}

template <typename T, size_t N, size_t A> bool Pool<T, N, A>::Exist(ID id) const {
    Item* item = Search(id);
    if (item) {
        size_t index = id >> 8;
        if (index >= flag.size()) {
            return false; // not set
        }
        return flag[index] & (id & 0x3f);
    }
}

template <typename T, size_t N, size_t A> void Pool<T, N, A>::Clear() {
    size_t loop = converter.size();
    for (size_t i = 0; i < loop; ++i) {
        container[converter[i]].ref -= 1;
        container[converter[i]].OnRelease();
    }
    converter.resize(0);
    allocator.Reduce();
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
    return container[converter[index]].instance;
}

template <typename T, size_t N, size_t A> Identifier<T> Pool<T, N, A>::Extern(ID id) {

    Item* item = Search(id);
    if (item == nullptr) {
        return Identifier<T>();
    }
    Owner(id)->Lost(id);

    Identifier<T> object;
    object.id = std::move(item->id);
    object.instance = *item->instance;    // copy
    allocator.Deallocate(item->instance); // do decontsruct

    return object; // rvo
}

template <typename T, size_t N, size_t A> bool Pool<T, N, A>::Intern(Identifier<T>&& ref) {
    Item* item = Search(ref.id);
    if (item == nullptr) {
        return false;
    }

    item->id = std::move(ref.id);
    item->instance = allocator.Allocate<T>(); // no construct
    *item->instance = ref.instance;
    new (&ref) Identifier<T>();

    return true;
}

template <typename T, size_t N, size_t A> bool Pool<T, N, A>::Include(Identifier<T>&& ref) {
    
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