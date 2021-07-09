#pragma once
#include <gtest/gtest.h>
#include <stdio.h>
#include <chrono>
#include "../../../include/utils/Time.hpp"

//---------------------------------------------------------------------------------------
static constexpr uint64_t testCount = 50000000;

TEST(utils, time_test) {
  uint64_t sum = 0;
  libzrvan::utils::Time::getTime();
  for (uint64_t i = 0; i < testCount; i++) {
    sum += libzrvan::utils::Time::getTimeMS();
  }
  std::cout << "dummy sum " << sum << std::endl;
}

//---------------------------------------------------------------------------------------
TEST(utils, time_test_std) {
  uint64_t sum = 0;
  for (uint64_t i = 0; i < testCount; i++) {
    sum += std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
  }
  std::cout << "dummy sum " << sum << std::endl;
}
