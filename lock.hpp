#ifndef LWE_UTILITIES_LOCK_HPP
#define LWE_UTILITIES_LOCK_HPP

#include <atomic>
#include <chrono>
#include <thread>

#include "config.h"
#include "macro.h"

/*
    LockGuard<MutexType> : object
    
    TypeLock<typename, MutexType>: static

    IndexLock<size_t, MutexTpye> : static

    SpinLock
    - atomic bool with locking and backoff mechanism
    - reentrant allowed
    - effective when small or large task


    use

    SpinLock mtx;

    mtx.Lock();
    mtx.lock(); // same method for std::lock_guard, reentrant allowed
    mtx.Unlock();
    mtx.unlock(); // same method for std::lock_guard, reentrant allowed

    LockGuard lock(mtx); // or (&mtx)
    // work
    // auto unlock

    TypeLock<void, SpinLock> lock_void; // key == type
    // work
    // auto unlock

    IndexLock<0, SpinLock> lock_0; // key == value
    // work
    // auto unlock

    std::lock_guard<SpinLock> l;
*/

template <typename Derived> class LockGuardBase {
public:
    LockGuardBase(bool = true);
    ~LockGuardBase();

public:
    NO_COPYABLE(LockGuardBase);
    NO_MOVABLE(LockGuardBase);
};

class SpinLock {
public:
    NO_COPYABLE(SpinLock);
    NO_MOVABLE(SpinLock);

public:
    SpinLock(size_t limit     = EConfig::SPIN_LOCK_BACKOFF_LIMIT_DEFAULT,
             size_t increment = EConfig::SPIN_LOCK_BACKOFF_INCREMENT_DEFAULT);

public:
    void Lock();
    void Unlock();

public:
    void SetBackoff(int limit, int increment);
    void SetBackoffLimit(int);
    void SetBackoffIncrement(int);

public:
    size_t GetTotalWaitTime();

private:
    std::atomic_bool             mtx    = false;
    std::atomic_int              locked = 0;
    std::atomic<std::thread::id> owner  = std::thread::id();

public:
    void lock();
    void unlock();

private:
    struct {
        size_t limit, increment;
    } backoff;
};

template <class T, class Mutex = SpinLock> class TypeLock : LockGuardBase<TypeLock<T, Mutex>> {
    friend LockGuardBase;
    static Mutex mtx;
};

template <size_t N, class Mutex = SpinLock> class IndexLock : LockGuardBase<IndexLock<N, Mutex>> {
    friend LockGuardBase;
    static Mutex mtx;
};

template <class Mutex = SpinLock> class LockGuard : LockGuardBase<LockGuard<Mutex>> {
    friend LockGuardBase;
    Mutex& mtx;

public:
    LockGuard(Mutex& arg);
};

#include "lock.ipp"
#endif