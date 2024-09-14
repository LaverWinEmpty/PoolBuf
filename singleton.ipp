#ifdef LWE_IOCP_SINGLETON_HPP

template<class T, class Lock> std::unique_ptr<T> Singleton<T, Lock>::instance = nullptr;

template<class T, class Lock> template<typename... Args> T* Singleton<T, Lock>::GetInstance(Args... args) {
    if(instance == nullptr) {
        LockGuardType _;
        if(instance == nullptr) {
            instance.reset(new T{ args... });
        }
    }
    return instance.get();
}

template<class T, class Lock> template<typename... Args> void Singleton<T, Lock>::CreateInstance(Args... args) {
    if(instance == nullptr) {
        LockGuardType _;
        if(instance == nullptr) {
            instance.reset(new T{ args... });
        }
    }
}

template<class T, class Lock> void Singleton<T, Lock>::DestroyInstance() {
    if(instance != nullptr) {
        LockGuardType _;
        if(instance != nullptr) {
            instance.reset(nullptr);
        }
    }
}

template<class T, class Lock> T* Singleton<T, Lock>::Instance() {
    return instance.get();
}

template<class T, class Lock> T* Singleton<T, Lock>::operator->() {
    return instance.get();
}

template<class T, class Lock> const T* Singleton<T, Lock>::operator->() const {
    return instance.get();
}

template<class T, class Lock> T& Singleton<T, Lock>::operator*() {
    if(instance == nullptr) {
        throw std::runtime_error("Null Pointer Exception");
    }
    return *instance.get();
}

template<class T, class Lock> const T& Singleton<T, Lock>::operator*() const {
    if(instance == nullptr) {
        throw std::runtime_error("Null Pointer Exception");
    }
    return *instance.get();
}

template<class T, class Lock> Singleton<T, Lock>::operator T*() const {
    return instance.get();
}

#endif