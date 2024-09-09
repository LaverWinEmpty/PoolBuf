#define NO_INSTANTIABLE(type)                                                                      \
    type()  = delete;                                                                              \
    ~type() = delete

#define NO_COPYABLE(type)                                                                          \
    type(type&)                  = delete;                                                         \
    type(const type&)            = delete;                                                         \
    type& operator=(type&)       = delete;                                                         \
    type& operator=(const type&) = delete

#define NO_MOVABLE(type)                                                                           \
    type(type&&)            = delete;                                                              \
    type& operator=(type&&) = delete
