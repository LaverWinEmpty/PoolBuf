#include "lock.hpp"
#include "iostream"

#ifdef LWE_UTILITIES_LOCK_HPP

SpinLock::SpinLock(int spin, int backoff): spin(spin), backoff(backoff) {}

void SpinLock::Lock() {
    std::thread::id id = std::this_thread::get_id();
    if(owner == id) {
        ++locked;
        return;
    }

    int attempts = 0;

    if(backoff) {
        while(flag.exchange(true, std::memory_order_acquire)) {
            if(++attempts > spin) {
                throw std::runtime_error("Timeout");
            }
            std::this_thread::sleep_for(std::chrono::microseconds(attempts * backoff));
        }
    }

    else {
        while(flag.exchange(true, std::memory_order_acquire)) {
            if(++attempts > spin) {
                throw std::runtime_error("Deadlock");
            }
            std::this_thread::yield();
        }
    }

    owner = id;
    ++locked;
}

void SpinLock::Unlock() {
    if(--locked == 0) {
        owner = std::thread::id();
        flag.store(false, std::memory_order_release); // release the lock
    }
}

void SpinLock::SetSpinCount(int arg, int backoffLimit) {
    spin = arg;
    if(backoffLimit) {
        backoff = backoffLimit;
    }
}

void SpinLock::SetBackoffIncrement(int arg, int spinCount) {
    backoff = arg;
    if(spinCount) {
        spin = spinCount;
    }
}

double SpinLock::GetBackoffWaitSec() {
    int n = 0;
    for(int i = 0; i < spin; ++i) {
        n += i * backoff;
    }
    return n * 1e-6;
}

void SpinLock::lock() {
    Lock();
}

void SpinLock::unlock() {
    Unlock();
}

template<class Mtx> LockGuard<Mtx>::LockGuard(Mtx& arg): mtx(arg) {
    mtx.lock();
}

template<class Mtx> LockGuard<Mtx>::~LockGuard() {
    mtx.unlock();
}

template<class Mtx> auto LockGuard<Mtx>::Locker() -> LockType& {
    return mtx;
}

template<class T, class Mtx> TypeLock<T, Mtx>::TypeLock(): LockGuard<Mtx>(mtx) {}

template<class T, class Mtx> auto TypeLock<T, Mtx>::Locker() -> LockType& {
    return mtx;
}

template<size_t N, class Mtx> IndexLock<N, Mtx>::IndexLock(): LockGuard<Mtx>(mtx) {}

template<size_t N, class Mtx> auto IndexLock<N, Mtx>::Locker() -> LockType& {
    return mtx;
}

template<class T, class Mtx> Lock<Mtx>::Type TypeLock<T, Mtx>::mtx;
template<size_t N, class Mtx> Lock<Mtx>::Type IndexLock<N, Mtx>::mtx;

void DisableLock::Lock() {}

void DisableLock::Unlock() {}

void DisableLock::lock() {}

void DisableLock::unlock() {}

#endif