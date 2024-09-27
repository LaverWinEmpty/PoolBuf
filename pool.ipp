#ifdef LWE_POOL_HPP

template <typename T, size_t N, size_t A> Pool<T, N, A>::Allocator Pool<T, N, A>::allocator;

template <typename T, size_t N, size_t A>
std::vector<typename Pool<T, N, A>::Item> Pool<T, N, A>::container;

template <typename T, size_t N, size_t A>
template <typename Arg>
void Pool<T, N, A>::Item::OnCreate(Arg&& arg) {
    instance = allocator.Allocate<T>();
    new (instance) T(arg);
    id.Generate();

    ref = 0;
}

template <typename T, size_t N, size_t A> void Pool<T, N, A>::Item::OnRelease() {
    id.Release();
    instance->~T();
    Pool::allocator.Deallocate(instance);

    ref = 0;
}

template <typename T, size_t N, size_t A> Pool<T, N, A>::~Pool() { Clear(); }

template <typename T, size_t N, size_t A>
template <typename Arg>
ID Pool<T, N, A>::Insert(Arg&& arg) {
    ID id = Global::Insert(arg);
    Take(id);
    return id;
}

template <typename T, size_t N, size_t A> size_t Pool<T, N, A>::Take(ID id) {
    Item* item = Global::Search(id);
    if (!item || item->id == ECode::INVALID_ID) {
        return ECode::INVALID_INDEX;
    }

    size_t index = converter(id); // insert
    if (index != ECode::INVALID_INDEX) {
        ++item->ref; // counting
    }
    return index;
}

template <typename T, size_t N, size_t A> bool Pool<T, N, A>::Erase(ID id) {
    if (Lost(id)) {
        if (container[id].ref == 0) {
            return Global::Erase(id);
        }
    }
    return false;
}

template <typename T, size_t N, size_t A> bool Pool<T, N, A>::Lost(ID id) {
    Item* item = Global::Search(id);
    if (!item || item->id == ECode::INVALID_ID) {
        return false;
    }

    size_t index = converter[id];
    if (item && converter(index) != ECode::INVALID_ID) {
        --item->ref;
        return true;
    }
    return false;
}

template <typename T, size_t N, size_t A> void Pool<T, N, A>::Leak() {
    size_t loop = Size();
    for (size_t i = 0; i < loop; ++i) {
        Lost(GetID(0));
    }
}

template <typename T, size_t N, size_t A> void Pool<T, N, A>::Clear() {
    size_t loop = Size();

    for (size_t i = 0; i < loop; ++i) {
        Erase(converter[0]);
    }
    allocator.Reduce();
}

template <typename T, size_t N, size_t A> T* Pool<T, N, A>::Find(ID id) {
    Item* item = Global::Search(id);
    if (item && item->id) {
        return item->instance;
    }
    return nullptr;
}

template <typename T, size_t N, size_t A> bool Pool<T, N, A>::Exist(ID id) const {
    return Global::Search(id) && converter[id] != ECode::INVALID_INDEX;
}

template <typename T, size_t N, size_t A> size_t Pool<T, N, A>::Size() const {
    return converter.table.size();
}

template <typename T, size_t N, size_t A> size_t Pool<T, N, A>::GetIndex(ID id) const {
    return converter[id];
}

template <typename T, size_t N, size_t A> ID Pool<T, N, A>::GetID(size_t idx) const {
    return converter[idx];
}

template <typename T, size_t POOL_CHUNK_COUNT, size_t POOL_ALIGNMENT>
auto Pool<T, POOL_CHUNK_COUNT, POOL_ALIGNMENT>::Begin() -> Iterator {
    return Iterator(this, 0);
}

template <typename T, size_t POOL_CHUNK_COUNT, size_t POOL_ALIGNMENT>
auto Pool<T, POOL_CHUNK_COUNT, POOL_ALIGNMENT>::End() -> Iterator {
    return Iterator(this, Size());
}

template <typename T, size_t N, size_t A> T* Pool<T, N, A>::operator[](ID id) {
    Item* item = Global::Search(id);
    if (!item || converter[id] == ECode::INVALID_INDEX) {
        return nullptr;
    }
    return item->instance;
}

template <typename T, size_t N, size_t A> T* Pool<T, N, A>::operator[](size_t index) {
    ID id = converter[index];
    if (id == ECode::INVALID_ID || Search(id) == nullptr) {
        return nullptr;
    }
    return container[id].instance;
}

/*
    Global
*/

template <typename T, size_t N, size_t A> auto Pool<T, N, A>::Global::Begin() -> Iterator {
    Item* item = &container[1];
    for (size_t i = 1; i < container.size() && item->id == ECode::INVALID_ID; ++i) {
        ++item;
    }
    return item;
}

template <typename T, size_t N, size_t A> auto Pool<T, N, A>::Global::End() -> Iterator {
    return Iterator(&container[0] + container.size());
}

template <typename T, size_t N, size_t A>
template <typename Arg>
ID Pool<T, N, A>::Global::Insert(Arg&& arg) {
    ID next = UniqueID::Preview();
    if (next == ECode::INVALID_ID) {
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

template <typename T, size_t N, size_t A> bool Pool<T, N, A>::Global::Erase(ID id) {
    Item* item = Global::Search(id);
    if (!item) {
        return false;
    }
    item->OnRelease();

    return true;
}

template <typename T, size_t N, size_t A> void Pool<T, N, A>::Global::Clear() {
    size_t loop = container.size();

    for (size_t i = 0; i < loop; ++i) {
        Item* item = &container[i];
        if (item->id != ECode::INVALID_ID && item->ref == 0) {
            item->id.Release();
            allocator.Deconstruct(item->instance);
        }
    }

    allocator.Reduce();
}

template <typename T, size_t N, size_t A> auto Pool<T, N, A>::Global::Search(ID id) -> Item* {
    if (id == ECode::INVALID_ID && id >= container.size()) {
        return nullptr;
    }
    return &container[id];
}


/*
    Converter
*/

template <typename T, size_t N, size_t A> size_t Pool<T, N, A>::Converter::operator()(ID id) {
    size_t last = table.size();

    Converter::Indexer::const_iterator itr = indexer.find(id);

    // not exist or deleted id
    if (itr == indexer.end() || itr->second == ECode::INVALID_INDEX) {
        table.push_back(id); // new
        indexer[id] = last;
        return last;
    }
    return ECode::INVALID_INDEX; // duplicated
}

template <typename T, size_t N, size_t A> ID Pool<T, N, A>::Converter::operator()(size_t idx) {
    size_t last = table.size() - 1;

    ID deleteID = operator[](idx);
    ID lastID = operator[](last);

    // id exist
    if (deleteID != ECode::INVALID_ID) {
        table[idx] = table[last];          // move
        indexer[lastID] = idx;                  // move
        indexer[deleteID] = ECode::INVALID_INDEX; // ref delete
        table.pop_back();                         // delete
    }
    return deleteID;
}

template <typename T, size_t N, size_t A>
ID Pool<T, N, A>::Converter::operator[](size_t index) const {
    if (index >= table.size()) {
        return ID::Invalid();
    }
    return table[index];
}

template <typename T, size_t N, size_t A> size_t Pool<T, N, A>::Converter::operator[](ID id) const {
    Converter::Indexer::const_iterator itr = indexer.find(id);
    if (itr == indexer.end()) {
        return ECode::INVALID_INDEX;
    }
    return itr->second;
}


/*
    Iterator
*/

template <typename T, size_t N, size_t A>
Pool<T, N, A>::Iterator::Iterator(Pool* ref, size_t idx) : ref(ref), index(idx) {}

template <typename T, size_t N, size_t A>
Pool<T, N, A>::Iterator::Iterator(const Iterator& arg) : ref(arg.ref), index(arg.index) {}

template <typename T, size_t N, size_t A>
auto Pool<T, N, A>::Iterator::operator=(const Iterator& arg) -> Iterator& {
    ref = arg.ref;
    index = arg.index;
    return *this;
}

template <typename T, size_t N, size_t A> auto Pool<T, N, A>::Iterator::operator++() -> Iterator& {
    ++index;
    return *this;
}

template <typename T, size_t N, size_t A>
auto Pool<T, N, A>::Iterator::operator++(int) -> Iterator {
    Iterator temp = *this;
    ++index;
    return temp;
}

template <typename T, size_t N, size_t A>
bool Pool<T, N, A>::Iterator::operator==(const Iterator& ref) const {
    return index == ref.index;
}

template <typename T, size_t N, size_t A>
bool Pool<T, N, A>::Iterator::operator!=(const Iterator& ref) const {
    return index != ref.index;
}

template <typename T, size_t N, size_t A> T* Pool<T, N, A>::Iterator::operator->() {
    return container[ref->converter[index]].instance;
}

template <typename T, size_t N, size_t A> T& Pool<T, N, A>::Iterator::operator*() {
    return *container[ref->converter[index]].instance;
}

template <typename T, size_t N, size_t A> Pool<T, N, A>::Iterator::operator ID() const {
    return ref->converter[index];
}

/*
    Global::Iterator
*/

template <typename T, size_t N, size_t A>
Pool<T, N, A>::Global::Iterator::Iterator(Item* ptr) : item(ptr) {}

template <typename T, size_t N, size_t A>
Pool<T, N, A>::Global::Iterator::Iterator(const Iterator& ref) : item(ref.item) {}

template <typename T, size_t N, size_t A>
auto Pool<T, N, A>::Global::Iterator::operator=(const Iterator& ref) -> Iterator& {
    item = ref.item;
    return *this;
}

template <typename T, size_t N, size_t A>
auto Pool<T, N, A>::Global::Iterator::operator++() -> Iterator& {
    Item* end = &container[0] + container.size();
    if (item == end) return *this;
    do {
        ++item;
    } while (item != end && item->id == ECode::INVALID_ID);
    return *this;
}

template <typename T, size_t N, size_t A>
auto Pool<T, N, A>::Global::Iterator::operator++(int) -> Iterator {
    Iterator temp = *this;
    this->operator++();
    return temp;
}

template <typename T, size_t N, size_t A>
bool Pool<T, N, A>::Global::Iterator::operator==(const Iterator& ref) const {
    return item == ref.item;
}

template <typename T, size_t N, size_t A>
bool Pool<T, N, A>::Global::Iterator::operator!=(const Iterator& ref) const {
    return item != ref.item;
}

template <typename T, size_t N, size_t A>
bool Pool<T, N, A>::Global::Iterator::operator==(const Pool* parent) const {
    return parent->Exist(item->id);
}

template <typename T, size_t N, size_t A>
bool Pool<T, N, A>::Global::Iterator::operator!=(const Pool* parent) const {
    return !operator==(parent);
}

template <typename T, size_t N, size_t A>
bool Pool<T, N, A>::Global::Iterator::operator==(const Pool& parent) const {
    return parent.Exist(item->id);
}

template <typename T, size_t N, size_t A>
bool Pool<T, N, A>::Global::Iterator::operator!=(const Pool& parent) const {
    return !operator==(parent);
}

template <typename T, size_t N, size_t A> T* Pool<T, N, A>::Global::Iterator::operator->() {
    return item->instance;
}

template <typename T, size_t N, size_t A> T& Pool<T, N, A>::Global::Iterator::operator*() {
    return *item->instance;
}

template <typename T, size_t N, size_t A> Pool<T, N, A>::Global::Iterator::operator ID() const {
    return item->id;
}



#endif