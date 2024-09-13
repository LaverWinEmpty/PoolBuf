#ifndef LWE_UTILITES_BUFFER_HPP
#define LWE_UTILITES_BUFFER_HPP

#include <string>

#include "allocator.hpp"
#include "config.h"
#include "lock.hpp"
#include "singleton.hpp"

/*
    buffer like char[N]
    use memory pool


    use

    Buffer<> arr = "string";
    arr[0] = 'S';
    *(arr + 6) = 's';
    *arr = 's';
    std::cout << arr;

    Buffer<15, 8, 3, std::mutex> copy = arr; // size 16 byte, count 8, alignment 4
    std::string str = copy;
    std::cout << str;
 */

template <size_t SIZE = EConfig::BUFFER_SIZE_DEFAULT, class Mtx = SpinLock,
          size_t ALLOCATOR_CHUNK_COUNT = EConfig::MEMORY_POOL_CHUNK_COUNT_DEFAULT,
          size_t ALLOCATOR_ALIGNMENT   = EConfig::MEMORY_POOL_ALIGNMENT_DEFAULT>
class Buffer {
public:
    using AllocatorType =
        Allocator<sizeof(Block<SIZE>), Mtx, ALLOCATOR_CHUNK_COUNT, ALLOCATOR_ALIGNMENT>;

public:
    static constexpr size_t Size();

public:
    Buffer();
    Buffer(const Buffer&);
    Buffer(Buffer&&) noexcept;
    Buffer(const std::string&);
    Buffer(const char*);

public:
    ~Buffer();

public:
    Buffer& operator=(const Buffer&);
    Buffer& operator=(Buffer&&) noexcept;
    Buffer& operator=(const std::string&);
    Buffer& operator=(const char*);

public:
    template <size_t, class, size_t, size_t> friend class Buffer;
    template <size_t N, class M, size_t X, size_t A> Buffer(const Buffer<N, M, X, A>&);
    template <size_t N, class M, size_t X, size_t A> Buffer(const Buffer&&) = delete;
    template <size_t N, class M, size_t X, size_t A> Buffer& operator=(const Buffer<N, M, X, A>&);
    template <size_t N, class M, size_t X, size_t A>
    Buffer& operator=(const Buffer<N, M, X, A>&&) = delete;

public:
    int8_t& operator[](size_t);
    int8_t operator[](size_t) const;

public:
    int8_t* operator+(intptr_t) const;
    int8_t* operator-(intptr_t) const;
    int8_t& operator*();
    int8_t* operator&();

public:
    operator const char*() const;
    explicit operator char*();
    explicit operator int8_t*();
    explicit operator uint8_t*();
    operator std::string() const;

private:
    int8_t* ptr;

private:
    Singleton<AllocatorType, void> allocator;
};

#include "buffer.ipp"
#endif