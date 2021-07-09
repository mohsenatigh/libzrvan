#pragma once
#include <gtest/gtest.h>
#include "../../../include/utils/CoreHash.hpp"
//---------------------------------------------------------------------------------------
TEST(utils, corehash_test_functionality) {
  // check functionality
  for (int i = 0; i < 2; i++) {
    auto out64 = libzrvan::utils::CoreHash::hash(0x112233445566);
    EXPECT_EQ(out64, 15480340755623179135ul);
  }
}
//---------------------------------------------------------------------------------------
TEST(utils, corehash_test_performance) {
  static const uint32_t loopCount = 2560000;
  std::cout << "loop count " << loopCount << std::endl;

  // disable optimization
  uint64_t sum = 0;

  // check performance
  for (uint32_t i = 0; i < loopCount; i++) {
    sum += libzrvan::utils::CoreHash::hash(0x112233445566ul);
  }
  EXPECT_EQ(sum, 15532011044981501952ul);
}
//---------------------------------------------------------------------------------------
