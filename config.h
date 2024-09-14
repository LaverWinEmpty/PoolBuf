#ifndef LWE_UTILITIES_CONFIG_H
#define LWE_UTILITIES_CONFIG_H

namespace EConfig {

enum {
    MEMORY_POOL_ALIGNMENT_DEFAULT   = 1,
    MEMORY_POOL_CHUNK_COUNT_DEFAULT = 64,
    BUFFER_SIZE_DEFAULT             = 4096,
    LOCK_SPIN_COUNT_DEFAULT         = 4000,
    LOCK_BACKOFF_LIMIT_DEFAULT      = 0,
};

} // namespace EConfig
#endif