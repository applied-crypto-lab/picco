#ifndef _RSS_TYPES_HPP_
#define _RSS_TYPES_HPP_

#include <cstdint>
typedef uint64_t *priv_int;         // this will be defined/written based on the ring_size
#define MPZ_CAST(X) (priv_int *)(X) // check how to up-cast from 1D to 2D

#endif // _RSS_TYPES_HPP_