#pragma once
#include <gtest/gtest.h>
#include <string>
#include <thread>
#include "../../../include/utils/Counter.hpp"
#include "../../../include/utils/StaticLoop.hpp"
//---------------------------------------------------------------------------------------
TEST(utils, counter_test) {
  uint32_t threadCount = 64;
  uint64_t loopCount = 512000;
  std::vector<std::thread> threads;
  libzrvan::utils::Counter<int64_t> cnt;

  // add thread
  auto addThread = [&]() {
    for (uint64_t i = 0; i < loopCount; i++) {
      cnt.add(1);
    }
  };

  // remove thread
  auto removeThread = [&]() {
    for (uint64_t i = 0; i < loopCount; i++) {
      cnt.sub(1);
    }
  };

  for (uint64_t i = 0; i < threadCount; i++) {
    threads.emplace_back(std::thread(addThread));
    threads.emplace_back(std::thread(removeThread));
  }

  for (auto& t : threads) {
    t.join();
  }

  EXPECT_EQ(0, cnt.get());
}
//---------------------------------------------------------------------------------------
TEST(utils, attomic_counter_test) {
  uint32_t threadCount = 64;
  uint64_t loopCount = 512000;
  std::vector<std::thread> threads;
  std::atomic<int64_t> cnt = {0};

  // add thread
  auto addThread = [&]() {
    for (uint64_t i = 0; i < loopCount; i++) {
      cnt.fetch_add(1, std::memory_order_relaxed);
    }
  };

  // remove thread
  auto removeThread = [&]() {
    for (uint64_t i = 0; i < loopCount; i++) {
      cnt.fetch_sub(1, std::memory_order_relaxed);
    }
  };

  for (uint64_t i = 0; i < threadCount; i++) {
    threads.emplace_back(std::thread(addThread));
    threads.emplace_back(std::thread(removeThread));
  }

  for (auto& t : threads) {
    t.join();
  }

  EXPECT_EQ(0, cnt);
}
