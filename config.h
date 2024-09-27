#ifndef LWE_UTILITIES_CONFIG_H
#define LWE_UTILITIES_CONFIG_H

namespace EConfig {

enum {
    MEMORY_ALIGNMENT_DEFAULT   = 8,
    MEMORY_ALLOCATE_DEFAULT    = 64,
    BUFFER_SIZE_DEFAULT        = 4096,
    LOCK_SPIN_COUNT_DEFAULT    = 4000,
    LOCK_BACKOFF_LIMIT_DEFAULT = 0,
};

} // namespace EConfig

namespace ECode {

enum {
    UNKNOWN_ERROR = -1,
    INVALID_INDEX = -1,
    INVALID_ID    =  0,
};

};

#endif