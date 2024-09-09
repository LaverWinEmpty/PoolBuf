Description

We initially intended to create a buffer for the server using Boost's memory pool, but due to performance and large-scale stability issues, we decided to implement a custom memory pool. This custom implementation aims to provide:

Better performance
Improved block management
Monitoring of block usage
Inclusion of a spin lock for memory pool usage purposes
