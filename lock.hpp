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
class SpinLock {
public:
    NO_COPYABLE(SpinLock);
    NO_MOVABLE(SpinLock);

public:
    SpinLock(int = EConfig::LOCK_SPIN_COUNT_DEFAULT, int = EConfig::LOCK_BACKOFF_LIMIT_DEFAULT);

public:
    void Lock();
    void Unlock();

public:
    void SetSpinCount(int, int backoff = 0);
    void SetBackoffIncrement(int, int spin = 0);

public:
    double GetBackoffWaitSec();

private:
    std::atomic<std::thread::id> owner   = std::thread::id();
    std::atomic_int              spin    = 0;
    std::atomic_int              backoff = 0;
    std::atomic_int              locked  = 0;
    std::atomic_bool             flag    = false;

public:
    void lock();
    void unlock();
};

/*
    void wrapper
*/
class DisableLock {
public:
    void Lock();
    void Unlock();
    void lock();
    void unlock();
};

template<typename T> struct Lock { using Type = T; };
template<> struct Lock<void> { using Type = DisableLock; };

template<class Mtx> class LockGuard {
public:
    using LockType = Lock<Mtx>::Type;

public:
    NO_COPYABLE(LockGuard);
    LockGuard(Mtx&);
    ~LockGuard();

public:
    LockType& Locker();

private:
    LockType& mtx;
};

template<class T, class Mtx = SpinLock> class TypeLock: public LockGuard<Mtx> {
public:
    using LockType = Lock<Mtx>::Type;

public:
    TypeLock();
    LockType& Locker();

private:
    static LockType mtx;
};

template<size_t N, class Mtx = SpinLock> class IndexLock: public LockGuard<Mtx> {
public:
    using LockType = Lock<Mtx>::Type;

public:
    IndexLock();
    LockType& Locker();

private:
    static LockType mtx;
};

#include "lock.ipp"
#endif