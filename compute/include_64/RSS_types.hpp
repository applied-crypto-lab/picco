#ifndef _RSS_TYPES_HPP_
#define _RSS_TYPES_HPP_
#include <cstdint>

// this will be defined/written based on the ring_size
typedef uint64_t *priv_int;
/* this is the "base" type, which is used so we don't need to use std::remove_pointer_t<> repeatedly  (e.g. nodenetwork)
 */
typedef uint64_t priv_int_t;
#endif // _RSS_TYPES_HPP_