#pragma once

#include <cstdint>
namespace libzrvan {
namespace utils {

/**
 * @brief implimentation of the Fast hash algorithm.
 *
 */
class FastHash {
 private:
  //-------------------------------------------------------------------------------------
  template <uint64_t seed>
  static uint64_t fasthash(const uint8_t *buf, std::size_t len) {
    // mix function
    auto mix = [](uint64_t v) -> uint64_t {
      v ^= v >> 23;
      v *= 0x2127599bf4325c37ULL;
      v ^= v >> 47;
      return v;
    };

    //
    const uint64_t m = 0x880355f21e6d1965ULL;
    const uint64_t *pos = reinterpret_cast<const uint64_t *>(buf);
    const uint64_t *end = pos + (len / 8);
    const unsigned char *pos2;
    uint64_t h = seed ^ (len * m);
    uint64_t v;

    while (pos != end) {
      v = *pos++;
      h ^= mix(v);
      h *= m;
    }

    pos2 = reinterpret_cast<const uint8_t *>(pos);
    v = 0;
    switch (len & 7) {
      case 7:
        v ^= (uint64_t)pos2[6] << 48;
        break;
      case 6:
        v ^= (uint64_t)pos2[5] << 40;
        break;
      case 5:
        v ^= (uint64_t)pos2[4] << 32;
        break;
      case 4:
        v ^= (uint64_t)pos2[3] << 24;
        break;
      case 3:
        v ^= (uint64_t)pos2[2] << 16;
        break;
      case 2:
        v ^= (uint64_t)pos2[1] << 8;
        break;
      case 1:
        v ^= (uint64_t)pos2[0];
        break;
      default:
        return h;
    }

    h ^= mix(v);
    h *= m;
    return mix(h);
  }

 public:
  //-------------------------------------------------------------------------------------
  /**
   * @brief
   *
   * @param buffer Input buffer
   * @param len  Input length
   * @return uint64_t
   */
  static uint64_t hash64(const uint8_t *buffer, std::size_t len) { return fasthash<0xcbf29ce484222325>(buffer, len); }
  //-------------------------------------------------------------------------------------
  /**
   * @brief
   *
   * @param buffer Input buffer
   * @param len Input length
   * @return uint32_t
   */
  static uint32_t hash32(const uint8_t *buffer, std::size_t len) {
    uint64_t h = fasthash<0x811c9dc5>(buffer, len);
    return (h - (h >> 32));
  }
};

}  // namespace utils
}  // namespace libzrvan
