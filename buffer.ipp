#ifdef LWE_UTILITES_BUFFER_HPP

template <size_t SIZE, class M, size_t X, size_t A>
Buffer<SIZE, M, X, A>::Buffer() : ptr(reinterpret_cast<int8_t*>(allocator.GetInstance()->Malloc())) {
    ::memset(ptr, 0, SIZE);
}

template <size_t SIZE, class M, size_t X, size_t A>
Buffer<SIZE, M, X, A>::Buffer(const Buffer& ref) : Buffer() {
    memcpy(ptr, ref.ptr, SIZE);
}

template <size_t SIZE, class M, size_t X, size_t A>
Buffer<SIZE, M, X, A>::Buffer(Buffer&& ref) noexcept : ptr(ref.ptr) {
    ref.ptr = nullptr;
}

template <size_t SIZE, class M, size_t X, size_t A>
Buffer<SIZE, M, X, A>::Buffer(const std::string& str) : Buffer() {
    memcpy(ptr, str.c_str(), SIZE < str.size() ? SIZE : str.size());
}

template <size_t SIZE, class M, size_t X, size_t A>
Buffer<SIZE, M, X, A>::Buffer(const char* str) : Buffer(std::string(str)) {}

template <size_t SIZE, class M, size_t X, size_t A> Buffer<SIZE, M, X, A>::~Buffer() {
    if (ptr) allocator.GetInstance()->Free(reinterpret_cast<Block<SIZE>*>(ptr));
}

template <size_t SIZE, class M, size_t X, size_t A>
Buffer<SIZE, M, X, A>& Buffer<SIZE, M, X, A>::operator=(const Buffer& ref) {
    memcpy(ptr, ref.ptr, SIZE);
    return *this;
}

template <size_t SIZE, class M, size_t X, size_t A>
Buffer<SIZE, M, X, A>& Buffer<SIZE, M, X, A>::operator=(Buffer&& ref) noexcept {
    if (this != &ref) {
        ptr     = ref.ptr;
        ref.ptr = nullptr;
    }
    return *this;
}

template <size_t SIZE, class M, size_t X, size_t A>
Buffer<SIZE, M, X, A>& Buffer<SIZE, M, X, A>::operator=(const std::string& str) {
    memcpy(ptr, str.c_str(), SIZE < str.size() ? SIZE : str.size());
    return *this;
}

template <size_t SIZE, class M, size_t X, size_t A>
Buffer<SIZE, M, X, A>& Buffer<SIZE, M, X, A>::operator=(const char* str) {
    std::string wrapper = str;

    memcpy(ptr, wrapper.c_str(), SIZE < wrapper.size() ? SIZE : wrapper.size());
    return *this;
}

template <size_t SIZE, class M, size_t X, size_t A>
template <size_t OTHER_S, class OtherM, size_t OTHER_X, size_t OTHER_A>
Buffer<SIZE, M, X, A>::Buffer(const Buffer<OTHER_S, OtherM, OTHER_X, OTHER_A>& other)
    : Buffer() {
    memcpy(ptr, other.ptr, SIZE < OTHER_S ? SIZE : OTHER_S);
}

template <size_t SIZE, class M, size_t X, size_t A>
template <size_t OTHER_S, class OtherM, size_t OTHER_X, size_t OTHER_A>
Buffer<SIZE, M, X, A>&
Buffer<SIZE, M, X, A>::operator=(const Buffer<OTHER_S, OtherM, OTHER_X, OTHER_A>& other) {
    memcpy(ptr, other.ptr, SIZE < OTHER_S ? SIZE : OTHER_S);
    return *this;
}

template <size_t SIZE, class M, size_t X, size_t A>
int8_t& Buffer<SIZE, M, X, A>::operator[](size_t i) {
    return ptr[i];
}

template <size_t SIZE, class M, size_t X, size_t A>
int8_t Buffer<SIZE, M, X, A>::operator[](size_t i) const {
    return ptr[i];
}

template <size_t SIZE, class M, size_t X, size_t A>
int8_t* Buffer<SIZE, M, X, A>::operator+(intptr_t i) const {
    return ptr + i;
}

template <size_t SIZE, class M, size_t X, size_t A>
int8_t* Buffer<SIZE, M, X, A>::operator-(intptr_t i) const {
    return ptr - i;
}

template <size_t SIZE, class M, size_t X, size_t A> int8_t& Buffer<SIZE, M, X, A>::operator*() {
    return *ptr;
}

template <size_t SIZE, class M, size_t X, size_t A> int8_t* Buffer<SIZE, M, X, A>::operator&() {
    return ptr;
}

template <size_t SIZE, class M, size_t X, size_t A>
Buffer<SIZE, M, X, A>::operator const char* () const {
    return reinterpret_cast<const char*>(ptr);;
}

template <size_t SIZE, class M, size_t X, size_t A>
Buffer<SIZE, M, X, A>::operator char* () {
    return reinterpret_cast<char*>(ptr);
}

template <size_t SIZE, class M, size_t X, size_t A>
Buffer<SIZE, M, X, A>::operator int8_t*() {
    return ptr;
}

template <size_t SIZE, class M, size_t X, size_t A>
Buffer<SIZE, M, X, A>::operator uint8_t*() {
    return reinterpret_cast<uint8_t*>(ptr);
}

template <size_t SIZE, class M, size_t X, size_t A>
Buffer<SIZE, M, X, A>::operator std::string() const {
    return std::string(reinterpret_cast<char*>(ptr), SIZE);
}

template <size_t SIZE, class M, size_t X, size_t A>
constexpr size_t Buffer<SIZE, M, X, A>::Size() {
    return SIZE;
}
#endif