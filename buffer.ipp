#ifdef LWE_UTILITES_BUFFER_HPP

template<size_t SIZE, class Lock, size_t COUNT, size_t ALIGN>
Buffer<SIZE, Lock, COUNT, ALIGN>::Buffer():
    ptr(Singleton<AllocatorType, void>::GetInstance()->New<int8_t, Lock>()) {
    ::memset(ptr, 0, SIZE);
}

template<size_t SIZE, class Lock, size_t COUNT, size_t ALIGN> Buffer<SIZE, Lock, COUNT, ALIGN>::Buffer(const Buffer& ref): Buffer() {
    TypeLock<Buffer, Lock>;
    memcpy(ptr, ref.ptr, SIZE);
}

template<size_t SIZE, class Lock, size_t COUNT, size_t ALIGN> Buffer<SIZE, Lock, COUNT, ALIGN>::Buffer(Buffer&& ref) noexcept{
    TypeLock<Buffer, Lock>;
    ptr     = ref.ptr; 
    ref.ptr = nullptr;
}

template<size_t SIZE, class Lock, size_t COUNT, size_t ALIGN> Buffer<SIZE, Lock, COUNT, ALIGN>::Buffer(const std::string& str): Buffer() {
    TypeLock<Buffer, Lock>;
    memcpy(ptr, str.c_str(), SIZE < str.size() ? SIZE : str.size());
}

template<size_t SIZE, class Lock, size_t COUNT, size_t ALIGN>
Buffer<SIZE, Lock, COUNT, ALIGN>::Buffer(const char* str): Buffer(std::string(str)) {}

template<size_t SIZE, class Lock, size_t COUNT, size_t ALIGN> Buffer<SIZE, Lock, COUNT, ALIGN>::~Buffer() {
    if(ptr) Singleton<AllocatorType, void>::GetInstance()->Free<Lock>(reinterpret_cast<Block<SIZE>*>(ptr));
}

template<size_t SIZE, class Lock, size_t COUNT, size_t ALIGN>
auto Buffer<SIZE, Lock, COUNT, ALIGN>::operator=(const Buffer& ref) -> Buffer&{
    TypeLock<Buffer, Lock>;
    memcpy(ptr, ref.ptr, SIZE);
    return *this;
}

template<size_t SIZE, class Lock, size_t COUNT, size_t ALIGN>
auto Buffer<SIZE, Lock, COUNT, ALIGN>::operator=(Buffer&& ref) noexcept -> Buffer& {
    if(this != &ref) {
        TypeLock<Buffer, Lock>;
        ptr     = ref.ptr;
        ref.ptr = nullptr;
    }
    return *this;
}

template<size_t SIZE, class Lock, size_t COUNT, size_t ALIGN>
auto Buffer<SIZE, Lock, COUNT, ALIGN>::operator=(const std::string& str) -> Buffer& {
    TypeLock<Buffer, Lock>;
    memcpy(ptr, str.c_str(), SIZE < str.size() ? SIZE : str.size());
    return *this;
}

template<size_t SIZE, class Lock, size_t COUNT, size_t ALIGN>
auto Buffer<SIZE, Lock, COUNT, ALIGN>::operator=(const char* str) -> Buffer& {
    TypeLock<Buffer, Lock>;
    std::string wrapper = str;
    memcpy(ptr, wrapper.c_str(), SIZE < wrapper.size() ? SIZE : wrapper.size());
    return *this;
}

template<size_t SIZE, class Lock, size_t COUNT, size_t ALIGN>
template<size_t N, class L, size_t C, size_t A>
Buffer<SIZE, Lock, COUNT, ALIGN>::Buffer(const Buffer<N, L, C, A>& other): Buffer() {
    TypeLock<Buffer, Lock>;
    memcpy(ptr, other.ptr, SIZE < N ? SIZE : N);
}

template<size_t SIZE, class Lock, size_t COUNT, size_t ALIGN>
template<size_t N, class L, size_t C, size_t A>
auto Buffer<SIZE, Lock, COUNT, ALIGN>::operator=(const Buffer<N, L, C, A>& other) -> Buffer& {
    TypeLock<Buffer, Lock>;
    memcpy(ptr, other.ptr, SIZE < N ? SIZE : N);
    return *this;
}

template<size_t SIZE, class Lock, size_t COUNT, size_t ALIGN> int8_t& Buffer<SIZE, Lock, COUNT, ALIGN>::operator[](size_t i) {
    return ptr[i];
}

template<size_t SIZE, class Lock, size_t COUNT, size_t ALIGN> int8_t Buffer<SIZE, Lock, COUNT, ALIGN>::operator[](size_t i) const {
    return ptr[i];
}

template<size_t SIZE, class Lock, size_t COUNT, size_t ALIGN> int8_t* Buffer<SIZE, Lock, COUNT, ALIGN>::operator+(intptr_t i) const {
    return ptr + i;
}

template<size_t SIZE, class Lock, size_t COUNT, size_t ALIGN> int8_t* Buffer<SIZE, Lock, COUNT, ALIGN>::operator-(intptr_t i) const {
    return ptr - i;
}

template<size_t SIZE, class Lock, size_t COUNT, size_t ALIGN> int8_t& Buffer<SIZE, Lock, COUNT, ALIGN>::operator*() {
    return *ptr;
}

template<size_t SIZE, class Lock, size_t COUNT, size_t ALIGN> int8_t* Buffer<SIZE, Lock, COUNT, ALIGN>::operator&() {
    return ptr;
}

template<size_t SIZE, class Lock, size_t COUNT, size_t ALIGN> Buffer<SIZE, Lock, COUNT, ALIGN>::operator const char*() const {
    return reinterpret_cast<const char*>(ptr);
}

template<size_t SIZE, class Lock, size_t COUNT, size_t ALIGN> Buffer<SIZE, Lock, COUNT, ALIGN>::operator char*() const {
    return reinterpret_cast<char*>(ptr);
}

template<size_t SIZE, class Lock, size_t COUNT, size_t ALIGN> Buffer<SIZE, Lock, COUNT, ALIGN>::operator int8_t*() const {
    return ptr;
}

template<size_t SIZE, class Lock, size_t COUNT, size_t ALIGN> Buffer<SIZE, Lock, COUNT, ALIGN>::operator uint8_t*() const {
    return reinterpret_cast<uint8_t*>(ptr);
}

template<size_t SIZE, class Lock, size_t COUNT, size_t ALIGN> Buffer<SIZE, Lock, COUNT, ALIGN>::operator std::string() const {
    return std::string(reinterpret_cast<char*>(ptr), SIZE);
}

template<size_t SIZE, class Lock, size_t COUNT, size_t ALIGN> constexpr size_t Buffer<SIZE, Lock, COUNT, ALIGN>::Size() {
    return SIZE;
}
#endif