#ifdef LWE_UTILITES_BUFFER_HPP

template <size_t SIZE, size_t X, size_t A, class Mtx>
Buffer<SIZE, X, A, Mtx>::Buffer() : ptr(reinterpret_cast<int8_t*>(allocator.allocate())) {
    ::memset(ptr, 0, SIZE);
}

template <size_t SIZE, size_t X, size_t A, class Mtx>
Buffer<SIZE, X, A, Mtx>::Buffer(const Buffer& ref) : Buffer() {
    memcpy(ptr, ref.ptr, SIZE);
}

template <size_t SIZE, size_t X, size_t A, class Mtx>
Buffer<SIZE, X, A, Mtx>::Buffer(Buffer&& ref) noexcept : ptr(ref.ptr) {
    ref.ptr = nullptr;
}

template <size_t SIZE, size_t X, size_t A, class Mtx>
Buffer<SIZE, X, A, Mtx>::Buffer(const std::string& str) : Buffer() {
    memcpy(ptr, str.c_str(), SIZE < str.size() ? SIZE : str.size());
}

template <size_t SIZE, size_t X, size_t A, class Mtx>
Buffer<SIZE, X, A, Mtx>::Buffer(const char* str) : Buffer(std::string(str)) {}

template <size_t SIZE, size_t X, size_t A, class Mtx> Buffer<SIZE, X, A, Mtx>::~Buffer() {
    if (ptr) allocator.deallocate(reinterpret_cast<Block<SIZE>*>(ptr));
}

template <size_t SIZE, size_t X, size_t A, class Mtx>
Buffer<SIZE, X, A, Mtx>& Buffer<SIZE, X, A, Mtx>::operator=(const Buffer& ref) {
    memcpy(ptr, ref.ptr, SIZE);
    return *this;
}

template <size_t SIZE, size_t X, size_t A, class Mtx>
Buffer<SIZE, X, A, Mtx>& Buffer<SIZE, X, A, Mtx>::operator=(Buffer&& ref) noexcept {
    if (this != &ref) {
        ptr     = ref.ptr;
        ref.ptr = nullptr;
    }
    return *this;
}

template <size_t SIZE, size_t X, size_t A, class Mtx>
Buffer<SIZE, X, A, Mtx>& Buffer<SIZE, X, A, Mtx>::operator=(const std::string& str) {
    memcpy(ptr, str.c_str(), SIZE < str.size() ? SIZE : str.size());
    return *this;
}

template <size_t SIZE, size_t X, size_t A, class Mtx>
Buffer<SIZE, X, A, Mtx>& Buffer<SIZE, X, A, Mtx>::operator=(const char* str) {
    std::string wrapper = str;

    memcpy(ptr, wrapper.c_str(), SIZE < wrapper.size() ? SIZE : wrapper.size());
    return *this;
}

template <size_t SIZE, size_t X, size_t A, class Mtx>
template <size_t OTHER_SIZE, size_t OTHER_X, size_t OTHER_A, class OtherMutex>
Buffer<SIZE, X, A, Mtx>::Buffer(const Buffer<OTHER_SIZE, OTHER_X, OTHER_A, OtherMutex>& other)
    : Buffer() {
    memcpy(ptr, other.ptr, SIZE < OTHER_SIZE ? SIZE : OTHER_SIZE);
}

template <size_t SIZE, size_t X, size_t A, class Mtx>
template <size_t OTHER_SIZE, size_t OTHER_X, size_t OTHER_A, class OtherMutex>
Buffer<SIZE, X, A, Mtx>&
Buffer<SIZE, X, A, Mtx>::operator=(const Buffer<OTHER_SIZE, OTHER_X, OTHER_A, OtherMutex>& other) {
    memcpy(ptr, other.ptr, SIZE < OTHER_SIZE ? SIZE : OTHER_SIZE);
    return *this;
}

template <size_t SIZE, size_t X, size_t A, class Mtx>
int8_t& Buffer<SIZE, X, A, Mtx>::operator[](size_t i) {
    return ptr[i];
}

template <size_t SIZE, size_t X, size_t A, class Mtx>
int8_t Buffer<SIZE, X, A, Mtx>::operator[](size_t i) const {
    return ptr[i];
}

template <size_t SIZE, size_t X, size_t A, class Mtx>
int8_t* Buffer<SIZE, X, A, Mtx>::operator+(intptr_t i) const {
    return ptr + i;
}

template <size_t SIZE, size_t X, size_t A, class Mtx>
int8_t* Buffer<SIZE, X, A, Mtx>::operator-(intptr_t i) const {
    return ptr - i;
}

template <size_t SIZE, size_t X, size_t A, class Mtx> int8_t& Buffer<SIZE, X, A, Mtx>::operator*() {
    return *ptr;
}

template <size_t SIZE, size_t X, size_t A, class Mtx> int8_t* Buffer<SIZE, X, A, Mtx>::operator&() {
    return ptr;
}

template <size_t SIZE, size_t X, size_t A, class Mtx>
Buffer<SIZE, X, A, Mtx>::operator const char* () const {
    return reinterpret_cast<const char*>(ptr);;
}

template <size_t SIZE, size_t X, size_t A, class Mtx>
Buffer<SIZE, X, A, Mtx>::operator char* () {
    return reinterpret_cast<char*>(ptr);
}

template <size_t SIZE, size_t X, size_t A, class Mtx>
Buffer<SIZE, X, A, Mtx>::operator int8_t*() {
    return ptr;
}

template <size_t SIZE, size_t X, size_t A, class Mtx>
Buffer<SIZE, X, A, Mtx>::operator uint8_t*() {
    return reinterpret_cast<uint8_t*>(ptr);
}

template <size_t SIZE, size_t X, size_t A, class Mtx>
Buffer<SIZE, X, A, Mtx>::operator std::string() const {
    return std::string(reinterpret_cast<char*>(ptr), SIZE);
}

template <size_t SIZE, size_t X, size_t A, class Mtx>
constexpr size_t Buffer<SIZE, X, A, Mtx>::Size() {
    return SIZE;
}
#endif