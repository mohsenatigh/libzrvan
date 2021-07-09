#pragma once
#include <gtest/gtest.h>
#include <chrono>
#include "../../../include/utils/StaticLoop.hpp"
//---------------------------------------------------------------------------------------
TEST(utils, static_loop_test) {
  uint64_t maskRes = 0;
  uint64_t count = 0;
  uint64_t countIndex = 0;

  auto testFunc = [&](uint32_t index, uint64_t mask) {
    maskRes |= mask;
    count++;
    countIndex += index;
  };

  STATIC_LOOP(64, {testFunc(_static_index,_static_mask);});
  EXPECT_EQ(maskRes, 0xffffffffffffffff);
  EXPECT_EQ(countIndex, 2016);
  EXPECT_EQ(count, 64);
}
