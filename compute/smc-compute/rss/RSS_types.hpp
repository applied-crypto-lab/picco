#ifndef _RSS_TYPES_HPP_
#define _RSS_TYPES_HPP_
#include <cstdint>
#if __RSS_32__
typedef uint32_t *priv_int;
typedef uint32_t priv_int_t;
#endif
#if __RSS_64__
typedef uint64_t *priv_int;
typedef uint64_t priv_int_t;
#endif
#endif // _RSS_TYPES_HPP_