#include "lock.hpp"

#ifdef LWE_UTILITIES_LOCK_HPP

SpinLock::SpinLock(size_t limit, size_t increment) : backoff{limit, increment} {}

void SpinLock::Lock() {
    std::thread::id id = std::this_thread::get_id();
    if (owner == id) {
        ++locked;
        return;
    }

    size_t attempts = 0;
    while (mtx.exchange(true, std::memory_order_acquire)) {
        if (++attempts > backoff.limit) throw std::runtime_error("DEADLOCK");
        std::this_thread::sleep_for(
            std::chrono::microseconds(attempts * backoff.increment)); // backoff
    }
    owner = id;
    ++locked;
}

void SpinLock::Unlock() {
    if (--locked == 0) {
        owner = std::thread::id();
        mtx.store(false, std::memory_order_release); // release the lock
    }
}

void SpinLock::SetBackoff(int limit, int increment) {
    backoff.limit     = limit;
    backoff.increment = increment;
}

void SpinLock::SetBackoffLimit(int n) { backoff.limit = n; }

void SpinLock::SetBackoffIncrement(int n) { backoff.increment = n; }

size_t SpinLock::GetTotalWaitTime() {
    size_t n = 0;
    for (size_t i = 1; i <= backoff.limit; ++i) {
        n += i * backoff.increment;
    }
    return n;
}

void SpinLock::lock() { Lock(); }

void SpinLock::unlock() { Unlock(); }

template <typename T> LockGuardBase<T>::LockGuardBase(bool set) {
    if (set) static_cast<T*>(this)->mtx.lock();
}

template <typename T> LockGuardBase<T>::~LockGuardBase() { static_cast<T*>(this)->mtx.unlock(); }

template <typename Mtx>
LockGuard<Mtx>::LockGuard(Mtx& arg) : mtx(arg), LockGuardBase<LockGuard<Mtx>>(false) {
    mtx.lock();
}

template <class T, class Mtx> Mtx  TypeLock<T, Mtx>::mtx;
template <size_t N, class Mtx> Mtx IndexLock<N, Mtx>::mtx;

void DisableLock::Lock() {
    if (id != std::this_thread::get_id())
        throw std::runtime_error("NOT PROTECTED");
}

void DisableLock::Unlock() {}

void DisableLock::lock() {
    Lock();
}

void DisableLock::unlock() {}


#endif