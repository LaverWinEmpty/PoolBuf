#ifndef LWE_UTILITIES_TYPE_H
#define LWE_UTILITIES_TYPE_H

#include <cstdint>

template <size_t SIZE> struct Block {
    uint8_t data[SIZE];
};

/*
    tparam N: target size
    tparam T: Type

    use
    Aligner<128, int>::Type variable;
*/
template <size_t N, typename T = void> struct Aligner {
public:
    static constexpr size_t PADDING = (N - (sizeof(T) % N)) % N;

private:
    template <size_t N, class T, bool = std::is_class_v<T>> struct Aligned : public T {
        int8_t padding[N];
    };
    template <class T> struct Aligned<0, T, true> : public T {};
    template <size_t N, class T> struct Aligned<N, T, false> {
        T      data;
        int8_t padding[N];
    };
    template <class T> struct Aligned<0, T, false> {
        T data;
    };

public:
    using Type = Aligned<PADDING, T>;
};

/*
    tparam N value
 */
template <size_t N> class Aligner<N, void> {
private:
    static constexpr size_t X = N - 1;

public:
    static constexpr size_t POWER_OF_TWO =
        N == 0 ? 1 : (X | (X >> 1) | (X >> 2) | (X >> 4) | (X >> 8) | (X >> 16) | (X >> 32)) + 1;
};

#endif