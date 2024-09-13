#include "id.hpp"
#include "allocator.hpp"

template<typename T, class Lock = DisableLock,
	size_t POOL_CHUNK_COUNT = EConfig::MEMORY_POOL_CHUNK_COUNT_DEFAULT,
	size_t POOL_ALIGNMENT = EConfig::MEMORY_POOL_ALIGNMENT_DEFAULT>
class Map {
	struct Node {
		SID<T> id;
		T*     data = nullptr;
	};

public:
	using AllocatorType = Allocator<sizeof(T), Lock, POOL_CHUNK_COUNT, POOL_ALIGNMENT>;
	using ContainerType = std::vector<Node>;
	using LockType  = Lock;

public:
	template<bool VAR>
	struct Indexer {
		using ReturnType = typename std::conditional_t<VAR, T&, const T&>;

		const ID   id;
		ReturnType data;
	};

public:
	Indexer<true>  operator[](size_t);
	Indexer<false> operator[](size_t) const;
	Indexer<true>  operator[](const ID&);
	Indexer<false> operator[](const ID&) const;

public:
	SID<T, Lock> Insert(const T&);
	bool         Erase(const ID&);
	T*           Find(const ID&) const; 
	ID           Check(size_t) const;

public:
	size_t Size() const;
	size_t Capacity() const;

private:
	AllocatorType allocator;
	ContainerType container;
	LockType      lock;
};

template<typename T, class Lock, size_t N, size_t A>
Map<T, Lock, N, A>::Indexer<true> Map<T, Lock, N, A>::operator[](size_t index) {
	if (index >= container.size())          throw std::out_of_range("OUT OF RANGE: INDEX");
	if (container[index].id == ID::INVALID) throw std::runtime_error("NULL POINTER EXCEPTION");
	return { container[index].id, *container[index].data };
}

template<typename T, class Lock, size_t N, size_t A>
Map<T, Lock, N, A>::Indexer<false> Map<T, Lock, N, A>::operator[](size_t index) const {
	if (index >= container.size())          throw std::out_of_range("OUT OF RANGE: INDEX");
	if (container[index].id == ID::INVALID) throw std::runtime_error("NULL POINTER EXCEPTION");
	return { container[index].id, *container[index].data };
}


template<typename T, class Lock, size_t N, size_t A>
Map<T, Lock, N, A>::Indexer<true> Map<T, Lock, N, A>::operator[](const ID& id) {
	size_t index = static_cast<size_t>(id) - 1;
	if (index >= container.size())          throw std::out_of_range("OUT OF RANGE: ID");
	if (container[index].id == ID::INVALID) throw std::runtime_error("NULL POINTER EXCEPTION");
	return { container[index].id, *container[index].data };
}

template<typename T, class Lock, size_t N, size_t A>
Map<T, Lock, N, A>::Indexer<false> Map<T, Lock, N, A>::operator[](const ID& id) const {
	size_t index = static_cast<size_t>(id) - 1;
	if (index >= container.size())          throw std::out_of_range("OUT OF RANGE: ID");
	if (container[index].id == ID::INVALID) throw std::runtime_error("NULL POINTER EXCEPTION");
	return { container[index].id, *container[index].data };
}

template<typename T, class Lock, size_t N, size_t A>
SID<T, Lock> Map<T, Lock, N, A>::Insert(const T& data) {
	LockGuard guard(lock);

	ID     id = ID(UID<T>::Preview());
	size_t index = id - 1;

	size_t size = container.size();
	if (index >= size) {
		container.resize(size + N);
	}

	container[index].data = allocator.New<T>(data);
	if (!container[index].data) {
		return container[index].id;
	}

	*container[index].data = data;

	return container[index].id;
}

template<typename T, class Lock, size_t N, size_t A>
bool Map<T, Lock, N, A>::Erase(const ID& id) {
	LockGuard guard(lock);

	size_t index = static_cast<size_t>(id) - 1;
	if (index >= container.size()) {
		return false;
	}

	allocator.Delete(container[index].data);
	container[index].id.Release();
	return true;
}

template<typename T, class Lock, size_t N, size_t A>
T* Map<T, Lock, N, A>::Find(const ID& id) const {
	LockGuard guard(lock);

	size_t index = static_cast<size_t>(id) - 1;

	if (index >= container.size()) {
		return nullptr;
	}
	return container[index].data;
}

template<typename T, class Lock, size_t N, size_t A>
ID Map<T, Lock, N, A>::Check(size_t index) const {
	if (index >= container.size()) {
		throw std::out_of_range("OUT OF RANGE: INDEX");
	}
	return container[index].id;
}

template<typename T, class Lock, size_t N, size_t A>
size_t Map<T, Lock, N, A>::Size() const {
	//return table.size();
	return 0;
}

template<typename T, class Lock, size_t N, size_t A>
size_t Map<T, Lock, N, A>::Capacity() const {
	return container.size();
}