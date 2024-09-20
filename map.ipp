#ifdef LWE_MAP_HPP

template<typename T> Map<T>::~Map() {
    size_t loop = table.size();
    for(size_t i = 0; i < loop; ++i) {
        Disable(table[i]);
    }
}

template<typename T> bool Map<T>::Exist(ID id) {
    return container[ID::ToIndex(id)].parent == this;
}

template<typename T> bool Map<T>::Exist(Iterator itr) {
    return itr.item.parent == this;
}

template<typename T> ID Map<T>::Insert(T&& arg) {
    ID id = Enable(std::forward<T>(arg));
    if(id == ID::INVALID) {
        return ID::Invalid();
    }

    size_t index = ID::ToIndex(id);

    container[index].parent = this;         // set
    container[index].index  = table.size(); // back

    table.push_back(id); // push

    return id;
}

template<typename T> bool Map<T>::Erase(ID id) {
    if(id > container.size() || container[ID::ToIndex(id)].prent != this) {
        return false;
    }
    return Disable(id);
}

template<typename T> size_t Map<T>::Size() const {
    return table.size();
}

template<typename T> T& Map<T>::operator[](ID id) {
    Item* temp = Find(id);
    if(!temp || temp->parent != this) {
        throw std::out_of_range("Not found.");
    }
    return *temp->instance;
}

template<typename T> T& Map<T>::operator[](size_t index) {
    Item* temp = Find(table[index]);
    if(!temp || temp->parent != this) {
        throw std::out_of_range("Not found.");
    }
    return *temp->instance;
}

template<typename T> const T& Map<T>::operator[](ID id) const {
    return const_cast<const T&>(const_cast<Map>(*this)[id]);
}

template<typename T> const T& Map<T>::operator[](size_t index) const {
    return const_cast<const T&>(const_cast<Map>(*this)[index]);
}

template<typename T> auto Map<T>::Find(ID id) -> Item* {
    Item* ret = nullptr;
    if(id <= container.size()) {
        ret = &container[ID::ToIndex(id)];
        if(ret->id == ID::INVALID) {
            ret = nullptr;
        }
    }
    return ret;
}

template<typename T> ID Map<T>::Enable(T&& arg) {
    // check
    ID next = UID<T>::Preview();
    if(next == ID::INVALID) {
        return next;
    }

    // check and allocate
    if(allocator.GetUsage().chunk.usable == 0) {
        container.resize(container.size() + allocator.CHUNK_COUNT);
        if(allocator.Expand() == false) {
            return ID::Invalid(); // malloc failed
        }
    }

    // next 1 => [0].id = 1
    Item& item = container[ID::ToIndex(next)];

    // allocate
    item.id.Generate(); // enable, this id == next
    item.instance = allocator.Construct<T>(std::forward<T>(arg));

    return next;
}

template<typename T> bool Map<T>::Disable(ID id) {
    if(id > container.size()) {
        return false;
    }

    Item& item = container[ID::ToIndex(id)];
    if(item.id == ID::INVALID) {
        return false;
    }

    Map* map = item.parent;
    if(map) {
        std::vector<ID>& table = map->table;

        // get pos
        size_t erase = item.index;
        size_t back  = table.size() - 1;

        // back pos to erase pos
        container[ID::ToIndex(table[back])].index = erase;

        // swap
        ID temp      = table[erase];
        table[erase] = table[back];
        table[back]  = temp;

        // after pop back
        table.pop_back();
    }

    // not actually deallocated
    allocator.Deconstruct(item.instance);
    item.id.Release(); // disable
    item.parent = nullptr;

    return true;
}

template<typename T> auto Map<T>::Begin() -> Iterator {
    size_t index = 0;
    size_t loop  = container.size();
    while(index < loop && container[index].id == ID::INVALID) {
        ++index;
    }
    return Iterator(&container[index]);
}

template<typename T> auto Map<T>::End() -> Iterator {
    return Iterator(&container[0] + container.size());
}

template<typename T> Map<T>::Iterator::Iterator(Item* item): item(item) {}
template<typename T> Map<T>::Iterator::Iterator(const Iterator& ref): item(ref.item) {}

template<typename T> bool Map<T>::Iterator::operator==(const Iterator& ref) const {
    return item == ref.item;
}

template<typename T> bool Map<T>::Iterator::operator!=(const Iterator& ref) const {
    return item != ref.item;
}

template<typename T> auto Map<T>::Iterator::operator++() -> Iterator& {
    Item* end = &container[0] + container.size();
    if(item != end) {
        do {
            ++item;
        } while(item != end && item->id == ID::INVALID); // pass
    }
    return *this;
}

template<typename T> auto Map<T>::Iterator::operator--() -> Iterator& {
    Item* begin = &container[0];
    if(item != begin) {
        do {
            --item;
        } while(item != begin && item->id == ID::INVALID); // pass
    }
    return *this;
}

template<typename T> auto Map<T>::Iterator::operator++(int) -> Iterator {
    Iterator itr = *this;
    ++*this;
    return itr;
}

template<typename T> auto Map<T>::Iterator::operator--(int) -> Iterator {
    Iterator itr = *this;
    --*this;
    return itr;
}

template<typename T> T* Map<T>::Iterator::operator->() {
    return item->instance;
}

template<typename T> T& Map<T>::Iterator::operator*() {
    return *item->instance;
}


#endif