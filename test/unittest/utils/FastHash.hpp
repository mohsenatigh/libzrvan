#pragma once
#include "../../../include/utils/FastHash.hpp"
#include "../../../include/utils/Utils.hpp"
#include <chrono>
#include <gtest/gtest.h>
#include <string>
#include <unordered_set>
#include <vector>
#include <xxhash.h>
//---------------------------------------------------------------------------------------

template <class HASH> void testWithHash(const std::string &input) {
  uint64_t sum = 0;
  static const uint32_t loopCount = 1000000;

  HASH h;

  auto start = std::chrono::high_resolution_clock::now();
  for (uint32_t i = 0; i < loopCount; i++) {
    sum += h(input);
  }
  auto end = std::chrono::high_resolution_clock::now();

  std::cout << input.size() << ","
            << std::chrono::duration_cast<std::chrono::microseconds>(end -
                                                                     start)
                   .count()
            << "," << sum << std::endl;

  return;
}
//---------------------------------------------------------------------------------------
template <class HASH> void testAllStrings() {
  auto inputs = {
      std::string(4, 'a'),  std::string(6, 'a'),   std::string(8, 'a'),
      std::string(10, 'a'), std::string(12, 'a'),  std::string(16, 'a'),
      std::string(20, 'a'), std::string(32, 'a'),  std::string(40, 'a'),
      std::string(64, 'a'), std::string(128, 'a'), std::string(256, 'a'),
  };

  std::cout << "string len,time(Micro second),sum" << std::endl;

  for (auto &a : inputs) {
    testWithHash<HASH>(a);
  }
}
//---------------------------------------------------------------------------------------
//xxx hash wraper
class XxxHashWraper {
  public:
    size_t operator()(const std::string& in){
      XXH64_hash_t hash = XXH64(in.data(), in.size(), 0xcbf29ce484222325);
      return static_cast<size_t>(hash); 
    }
};
//---------------------------------------------------------------------------------------
TEST(utils, hash_xxx_test_performance64) {
  testAllStrings<XxxHashWraper>();
}
//---------------------------------------------------------------------------------------
TEST(utils, hash_fasthash_test_performance64) {
  testAllStrings<libzrvan::utils::FastHash<std::string>>();
}
//---------------------------------------------------------------------------------------
TEST(utils, hash_stdhash_test_performance) {
  testAllStrings<std::hash<std::string>>();
}
//---------------------------------------------------------------------------------------
