#ifdef LWE_IOCP_SINGLETON_HPP

template <class T, class Mtx> std::unique_ptr<T> Singleton<T, Mtx>::instance = nullptr;

template <class T, class Mtx>
template <typename... Args>
T* Singleton<T, Mtx>::GetInstance(Args... args) {
    if (instance == nullptr) {
        LockGuardType _;
        if (instance == nullptr) {
            instance.reset(new T{ args... });
        }
    }
    return instance.get();
}

template <class T, class Mtx>
template <typename... Args>
void Singleton<T, Mtx>::CreateInstance(Args... args) {
    if (instance == nullptr) {
        LockGuardType _;
        if (instance == nullptr) {
            instance.reset(new T{args...});
        }
    }
}

template <class T, class Mtx> void Singleton<T, Mtx>::DestroyInstance() {
    if (instance != nullptr) {
        LockGuardType _;
        if (instance != nullptr) {
            instance.reset(nullptr);
        }
    }
}

template <class T, class Mtx> T* Singleton<T, Mtx>::Instance() { return instance.get(); }


template <class T, class Mtx> T* Singleton<T, Mtx>::operator->() { return instance.get(); }

template <class T, class Mtx> const T* Singleton<T, Mtx>::operator->() const {
    return instance.get();
}

template <class T, class Mtx> T& Singleton<T, Mtx>::operator*() {
    if (instance == nullptr) {
        throw std::runtime_error("Null Pointer Exception");
    }
    return *instance.get();
}

template <class T, class Mtx> const T& Singleton<T, Mtx>::operator*() const {
    if (instance == nullptr) {
        throw std::runtime_error("Null Pointer Exception");
    }
    return *instance.get();
}

template <class T, class Mtx> Singleton<T, Mtx>::operator T*() const { return instance.get(); }

#endif