#pragma once
#include "../../../include/ds/ExpMap.hpp"
#include "../../../include/ds/ExpSlotList.hpp"
#include <chrono>
#include <deque>
#include <functional>
#include <gtest/gtest.h>
#include <iostream>
#include <list>
#include <shared_mutex>
#include <vector>
//---------------------------------------------------------------------------------------
struct testObject {
  uint64_t p1;
  uint64_t p2;
  uint64_t p3;
  std::string p4;
};
//---------------------------------------------------------------------------------------
// functionality test
TEST(ds, exp_slot_list_test) {

  // check functionality
  static constexpr uint32_t testCount = 100;

  libzrvan::ds::ExpSlotList<testObject> slotList;
  testObject t = {.p1 = 1, .p2 = 2, .p3 = 3, .p4 = "hello"};

  // simple functionality test
  auto add = [&]() {
    for (uint64_t i = 0; i < testCount; i++) {
      t.p1 = i;
      EXPECT_EQ(slotList.add(i, t, 10), true);
    }
  };

  add();

  for (uint64_t i = 0; i < testCount; i++) {
    EXPECT_EQ(
        slotList.findR(i, [&](testObject &obj) -> bool { return obj.p1 == i; }),
        true);
    EXPECT_EQ(slotList.findW(i), true);
  }

  // test
  EXPECT_EQ(slotList.size(), testCount);
  EXPECT_EQ(slotList.forEach(nullptr), testCount);

  // remove items
  for (uint64_t i = 0; i < testCount; i++) {
    EXPECT_EQ(slotList.remove(i), true);
  }
  EXPECT_EQ(slotList.size(), 0);

  for (uint64_t i = 0; i < testCount; i++) {
    EXPECT_EQ(slotList.remove(i), false);
  }

  // selective remove
  add();
  EXPECT_EQ(slotList.remove(10), true);
  EXPECT_EQ(slotList.remove(40), true);
  EXPECT_EQ(slotList.remove(50), true);
  EXPECT_EQ(slotList.findR(50), false);
  EXPECT_EQ(slotList.size(), testCount - 3);
  EXPECT_EQ(slotList.forEach(nullptr), testCount - 3);
  slotList.flush();
  EXPECT_EQ(slotList.size(), 0);
  EXPECT_EQ(slotList.forEach(nullptr), 0);

  // remove one segment
  add();
  for (int i = 16; i < 32; i++) {
    EXPECT_EQ(slotList.remove(i), true);
  }
  EXPECT_EQ(slotList.forEach(nullptr), testCount - 16);
  slotList.flush();

  // expire check

  add();
  EXPECT_EQ(slotList.expireCheck(libzrvan::utils::Time::getTime() + 11),
            testCount);
  EXPECT_EQ(slotList.size(), 0);
  EXPECT_EQ(slotList.forEach(nullptr), 0);

  // check move
  add();
  libzrvan::ds::ExpSlotList<testObject> temp = std::move(slotList);
  EXPECT_EQ(temp.forEach(nullptr), testCount);
  EXPECT_EQ(temp.size(), testCount);

  EXPECT_EQ(slotList.forEach(nullptr), 0);
  EXPECT_EQ(slotList.size(), 0);
}

//---------------------------------------------------------------------------------------
static void runTest(const std::string &info, std::function<void()> func) {
  auto start = std::chrono::high_resolution_clock::now();
  func();
  auto end = std::chrono::high_resolution_clock::now();
  std::cout << info << " duration is "
            << std::chrono::duration_cast<std::chrono::microseconds>(end -
                                                                     start)
                   .count()
            << std::endl;
}
//---------------------------------------------------------------------------------------
// performance test boundle
static constexpr uint32_t testCount_ = 1000000;
static constexpr uint32_t searchCount_ = 100;
static constexpr uint32_t removeCount_ = 100;

template <class T> static void runExpTest() {
  
  T slotList;
  // add func
  auto addFun = [&]() {
    for (uint64_t i = 0; i < testCount_; i++) {
      testObject obj;
      obj.p1 = i;
      slotList.add(i, obj, 10);
    }
  };

  // always fail search test ( for testing the performance )
  auto findFun = [&]() {
    for (uint64_t i = 0; i < searchCount_; i++) {
      if (slotList.findR(testCount_ + 1)) {
        std::cout << "found" << std::endl;
      }
    }
  };

  // random remove ( for testing the performance )
  auto removeFun = [&]() {
    uint32_t rCount = 0;
    for (uint64_t i = 0; i < removeCount_; i++) {
      if (slotList.remove(i + 100)) {
        rCount++;
      }
    }
    std::cout << rCount << std::endl;
  };
  runTest("add test", addFun);
  runTest("find test", findFun);
  runTest("remove test", removeFun);
}

TEST(ds, exp_list_fill_sample_list) {
  runExpTest<libzrvan::ds::ExpSlotList<testObject>>();
}
//---------------------------------------------------------------------------------------
template <class T> void runTest(const std::string &testType) {
  std::shared_mutex lock;
  T slotList;

  // add func
  auto addFun = [&]() {
    for (uint64_t i = 0; i < testCount_; i++) {
      testObject obj;
      obj.p1 = i;
      lock.lock();
      slotList.push_back(obj);
      lock.unlock();
    }
  };

  // find func
  auto findFun = [&]() {
    for (uint64_t i = 0; i < searchCount_; i++) {
      lock.lock_shared();
      for (auto &i : slotList) {
        if (i.p1 == testCount_ + 1) {
          printf("found");
        }
      }
      lock.unlock_shared();
    }
  };

  // remove function
  auto removeFun = [&]() {
    uint32_t count = 0;
    for (uint64_t i = 0; i < removeCount_; i++) {
      lock.lock();
      for (auto it = slotList.begin(); it != slotList.end(); ++it) {
        if (it->p1 == i + 100) {
          slotList.erase(it);
          count++;
          break;
        }
      }
      lock.unlock();
    }
    std::cout << count << std::endl;
  };

  std::cout << "test " << testType << std::endl;
  runTest("add test std", addFun);
  runTest("find test std", findFun);
  runTest("remove test std", removeFun);
}
//---------------------------------------------------------------------------------------
TEST(ds, exp_list_fill_sample_std) {
  runTest<std::vector<testObject>>("vector");
  runTest<std::list<testObject>>("list");
  runTest<std::deque<testObject>>("deque");
}