#pragma once
#include "../../../include/utils/Counter.hpp"
#include "../../../include/utils/StaticLoop.hpp"
#include <gtest/gtest.h>
#include <string>
#include <thread>

//---------------------------------------------------------------------------------------
static const uint64_t countersLoopCount_ = 512000;
static const auto countersThreadCount_ = {1,  4,  6,  8,  12, 16,
                                          20, 24, 32, 40, 64};

void runCounterTest(uint32_t threadCount, std::function<void()> addFn,
                    std::function<void()> rFn) {
  std::vector<std::thread> threads;
  libzrvan::utils::Counter<int64_t> cnt;

  auto start = std::chrono::high_resolution_clock::now();

  for (uint64_t i = 0; i < threadCount; i++) {
    threads.emplace_back(std::thread(addFn));
    threads.emplace_back(std::thread(rFn));
  }

  for (auto &t : threads) {
    t.join();
  }
  auto end = std::chrono::high_resolution_clock::now();

  std::cout << threadCount << ","
            << std::chrono::duration_cast<std::chrono::microseconds>(end -
                                                                     start)
                   .count()
            << std::endl;
}
//---------------------------------------------------------------------------------------

TEST(utils, counter_test) {
  libzrvan::utils::Counter<int64_t> cnt;

  // add thread
  auto addThread = [&]() {
    for (uint64_t i = 0; i < countersLoopCount_; i++) {
      cnt.add(1);
    }
  };

  // remove thread
  auto removeThread = [&]() {
    for (uint64_t i = 0; i < countersLoopCount_; i++) {
      cnt.sub(1);
    }
  };

  for (auto &i : countersThreadCount_) {
    runCounterTest(i, addThread, removeThread);
  }

  EXPECT_EQ(0, cnt.get());
}
//---------------------------------------------------------------------------------------
TEST(utils, attomic_counter_test) {
  std::atomic<int64_t> cnt = {0};

  // add thread
  auto addThread = [&]() {
    for (uint64_t i = 0; i < countersLoopCount_; i++) {
      cnt.fetch_add(1, std::memory_order_relaxed);
    }
  };

  // remove thread
  auto removeThread = [&]() {
    for (uint64_t i = 0; i < countersLoopCount_; i++) {
      cnt.fetch_sub(1, std::memory_order_relaxed);
    }
  };

  for (auto &i : countersThreadCount_) {
    runCounterTest(i, addThread, removeThread);
  }

  EXPECT_EQ(0, cnt);
}
