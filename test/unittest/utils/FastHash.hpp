#pragma once
#include <gtest/gtest.h>
#include <chrono>
#include <string>
#include "../../../include/utils/FastHash.hpp"
//---------------------------------------------------------------------------------------
static const std::string input_(16, 'a');
static const uint32_t loopCount_ = 10000000;
//---------------------------------------------------------------------------------------
TEST(utils, fasthash_test_performance64) {
  std::cout << "loop count " << loopCount_ << " string len " << input_.size() << std::endl;

  // disable optimization
  uint64_t sum = 0;

  // check performance
  for (uint32_t i = 0; i < loopCount_; i++) {
    sum += libzrvan::utils::FastHash::hash64(reinterpret_cast<const uint8_t*>(input_.data()), input_.size());
  }
  std::cout << sum << "\n";
}
//---------------------------------------------------------------------------------------
TEST(utils, fasthash_test_std_hash_performance) {
  std::hash<std::string> h;
  uint32_t sum = 0;
  for (uint32_t i = 0; i < loopCount_; i++) {
    sum += h(input_);
  }
  std::cout << sum << "\n";
}
//---------------------------------------------------------------------------------------
