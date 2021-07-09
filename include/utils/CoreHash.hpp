#pragma once

#include <cstdint>
namespace libzrvan {
namespace utils {

/**
 * @brief core hash algorithm. used to decrease the collision chance in a numeric space
 */
class CoreHash {
 public:
  static uint64_t hash(uint64_t input) {
    uint64_t x=input;
    x = ((x >> 32) ^ x) * 0x45d9f3b;
    x = ((x >> 32) ^ x) * 0x45d9f3b;
    x = ((x >> 32) ^ x);
    return x;
  }
}; 
}  // namespace utils
}  // namespace libzrvan
