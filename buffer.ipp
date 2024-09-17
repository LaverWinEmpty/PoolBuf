#ifdef LWE_UTILITES_BUFFER_HPP

template<size_t SIZE, class Mtx, size_t COUNT, size_t ALIGN> Buffer<SIZE, Mtx, COUNT, ALIGN>::Buffer():
    ptr(pool.Allocate<int8_t>()) {
    ::memset(ptr, 0, SIZE);
}

template<size_t SIZE, class Mtx, size_t COUNT, size_t ALIGN> Buffer<SIZE, Mtx, COUNT, ALIGN>::Buffer(const Buffer& ref): Buffer() {
    memcpy(ptr, ref.ptr, SIZE);
}

template<size_t SIZE, class Mtx, size_t COUNT, size_t ALIGN> Buffer<SIZE, Mtx, COUNT, ALIGN>::Buffer(Buffer&& ref) noexcept{
    ptr     = ref.ptr; 
    ref.ptr = nullptr;
}

template<size_t SIZE, class Mtx, size_t COUNT, size_t ALIGN> Buffer<SIZE, Mtx, COUNT, ALIGN>::Buffer(const std::string& str): Buffer() {
    memcpy(ptr, str.c_str(), SIZE < str.size() ? SIZE : str.size());
}

template<size_t SIZE, class Mtx, size_t COUNT, size_t ALIGN>
Buffer<SIZE, Mtx, COUNT, ALIGN>::Buffer(const char* str): Buffer(std::string(str)) {}

template<size_t SIZE, class Mtx, size_t COUNT, size_t ALIGN> Buffer<SIZE, Mtx, COUNT, ALIGN>::~Buffer() {
    pool.Deallocate(ptr);
}

template<size_t SIZE, class Mtx, size_t COUNT, size_t ALIGN>
auto Buffer<SIZE, Mtx, COUNT, ALIGN>::operator=(const Buffer& ref) -> Buffer&{
    memcpy(ptr, ref.ptr, SIZE);
    return *this;
}

template<size_t SIZE, class Mtx, size_t COUNT, size_t ALIGN>
auto Buffer<SIZE, Mtx, COUNT, ALIGN>::operator=(Buffer&& ref) noexcept -> Buffer& {
    if(this != &ref) {
        ptr     = ref.ptr;
        ref.ptr = nullptr;
    }
    return *this;
}

template<size_t SIZE, class Mtx, size_t COUNT, size_t ALIGN>
auto Buffer<SIZE, Mtx, COUNT, ALIGN>::operator=(const std::string& str) -> Buffer& {
    memcpy(ptr, str.c_str(), SIZE < str.size() ? SIZE : str.size());
    return *this;
}

template<size_t SIZE, class Mtx, size_t COUNT, size_t ALIGN>
auto Buffer<SIZE, Mtx, COUNT, ALIGN>::operator=(const char* str) -> Buffer& {
    std::string wrapper = str;
    memcpy(ptr, wrapper.c_str(), SIZE < wrapper.size() ? SIZE : wrapper.size());
    return *this;
}

template<size_t SIZE, class Mtx, size_t COUNT, size_t ALIGN>
template<size_t N, class M, size_t CNT, size_t A>
Buffer<SIZE, Mtx, COUNT, ALIGN>::Buffer(const Buffer<N, M, CNT, A>& other): Buffer() {
    memcpy(ptr, other.ptr, SIZE < N ? SIZE : N);
}

template<size_t SIZE, class Mtx, size_t COUNT, size_t ALIGN>
template<size_t N, class M, size_t CNT, size_t A>
auto Buffer<SIZE, Mtx, COUNT, ALIGN>::operator=(const Buffer<N, M, CNT, A>& other) -> Buffer& {
    memcpy(ptr, other.ptr, SIZE < N ? SIZE : N);
    return *this;
}

template<size_t SIZE, class Mtx, size_t COUNT, size_t ALIGN> int8_t& Buffer<SIZE, Mtx, COUNT, ALIGN>::operator[](size_t i) {
    return ptr[i];
}

template<size_t SIZE, class Mtx, size_t COUNT, size_t ALIGN> int8_t Buffer<SIZE, Mtx, COUNT, ALIGN>::operator[](size_t i) const {
    return ptr[i];
}

template<size_t SIZE, class Mtx, size_t COUNT, size_t ALIGN> int8_t* Buffer<SIZE, Mtx, COUNT, ALIGN>::operator+(intptr_t i) const {
    return ptr + i;
}

template<size_t SIZE, class Mtx, size_t COUNT, size_t ALIGN> int8_t* Buffer<SIZE, Mtx, COUNT, ALIGN>::operator-(intptr_t i) const {
    return ptr - i;
}

template<size_t SIZE, class Mtx, size_t COUNT, size_t ALIGN> int8_t& Buffer<SIZE, Mtx, COUNT, ALIGN>::operator*() {
    return *ptr;
}

template<size_t SIZE, class Mtx, size_t COUNT, size_t ALIGN> int8_t* Buffer<SIZE, Mtx, COUNT, ALIGN>::operator&() {
    return ptr;
}

template<size_t SIZE, class Mtx, size_t COUNT, size_t ALIGN> Buffer<SIZE, Mtx, COUNT, ALIGN>::operator const char*() const {
    return reinterpret_cast<const char*>(ptr);
}

template<size_t SIZE, class Mtx, size_t COUNT, size_t ALIGN> Buffer<SIZE, Mtx, COUNT, ALIGN>::operator char*() const {
    return reinterpret_cast<char*>(ptr);
}

template<size_t SIZE, class Mtx, size_t COUNT, size_t ALIGN> Buffer<SIZE, Mtx, COUNT, ALIGN>::operator int8_t*() const {
    return ptr;
}

template<size_t SIZE, class Mtx, size_t COUNT, size_t ALIGN> Buffer<SIZE, Mtx, COUNT, ALIGN>::operator uint8_t*() const {
    return reinterpret_cast<uint8_t*>(ptr);
}

template<size_t SIZE, class Mtx, size_t COUNT, size_t ALIGN> Buffer<SIZE, Mtx, COUNT, ALIGN>::operator std::string() const {
    return std::string(reinterpret_cast<char*>(ptr), SIZE);
}

template<size_t SIZE, class Mtx, size_t COUNT, size_t ALIGN> constexpr size_t Buffer<SIZE, Mtx, COUNT, ALIGN>::Size() {
    return SIZE;
}
#endif