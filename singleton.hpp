#ifndef LWE_IOCP_SINGLETON_HPP
#define LWE_IOCP_SINGLETON_HPP

#include <memory>

#include "lock.hpp"
#include "macro.h"

template <class T, class Lock = SpinLock> class Singleton {
public:
    using InstanceType  = T;
    using LockGuardType = TypeLock<Singleton<T, Lock>>;

public:
    template <typename... Args> static T*   GetInstance(Args... args);
    template <typename... Args> static void CreateInstance(Args... args);
    static void                             DestroyInstance();
    static T*                               Instance();

public:
    T*       operator->();
    const T* operator->() const;
    T&       operator*();
    const T& operator*() const;
    operator T*() const;

protected:
    static std::unique_ptr<T> instance;
};

#include "singleton.ipp"
#endif