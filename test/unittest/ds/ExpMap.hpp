#pragma once
#include <gtest/gtest.h>
#include <chrono>
#include "../../../include/ds/ExpMap.hpp"
//---------------------------------------------------------------------------------------
struct testObjectMap {
  uint64_t p1;
  uint64_t p2;
  uint64_t p3;
  std::string p4;
};
//---------------------------------------------------------------------------------------
// functionality test
TEST(ds, exp_map_test) {
  // check functionality
  static constexpr uint32_t testCount = 100;
  libzrvan::ds::ExpMap<uint64_t, testObjectMap> map;
  testObjectMap t = {.p1 = 1, .p2 = 2, .p3 = 3, .p4 = "hello"};

  // simple functionality test
  auto add = [&]() {
    for (uint64_t i = 0; i < testCount; i++) {
      t.p1 = i;
      EXPECT_EQ(map.add(i, t, 10), true);
    }
  };

  add();

  for (uint64_t i = 0; i < testCount; i++) {
    EXPECT_EQ(map.findR(i, [&](testObjectMap& obj) -> bool { return obj.p1 == i; }), true);
    EXPECT_EQ(map.findW(i), true);
  }

  // test
  EXPECT_EQ(map.size(), testCount);
  EXPECT_EQ(map.forEach(nullptr), testCount);

  // remove items
  for (uint64_t i = 0; i < testCount; i++) {
    EXPECT_EQ(map.remove(i), true);
  }
  // EXPECT_EQ(slotList.size(), 0);

  for (uint64_t i = 0; i < testCount; i++) {
    EXPECT_EQ(map.remove(i), false);
  }

  // selective remove
  add();
  EXPECT_EQ(map.remove(10), true);
  EXPECT_EQ(map.remove(40), true);
  EXPECT_EQ(map.remove(50), true);
  EXPECT_EQ(map.findR(50), false);
  EXPECT_EQ(map.size(), testCount - 3);
  EXPECT_EQ(map.forEach(nullptr), testCount - 3);
  map.flush();
  EXPECT_EQ(map.size(), 0);
  EXPECT_EQ(map.forEach(nullptr), 0);

  // remove one segment
  add();
  for (int i = 16; i < 32; i++) {
    EXPECT_EQ(map.remove(i), true);
  }
  EXPECT_EQ(map.forEach(nullptr), testCount - 16);
  map.flush();

  // expire check

  add();
  uint32_t totalExpired = 0;
  for (uint32_t i = 0; i < map.getSegmentsCount(); i++) {
    totalExpired += map.expireCheck(libzrvan::utils::Time::getTime() + 11);
  }
  EXPECT_EQ(totalExpired, testCount);

  EXPECT_EQ(map.size(), 0);
  EXPECT_EQ(map.forEach(nullptr), 0);
}

//---------------------------------------------------------------------------------------
static void runMapTest(const std::string& info, std::function<void(uint32_t tid)> func, uint32_t tCount) {
  std::vector<std::thread> threads;
  auto start = std::chrono::high_resolution_clock::now();
  for (uint32_t i = 0; i < tCount; i++) {
    threads.emplace_back(func, i);
  }
  for (auto& t : threads) {
    t.join();
  }
  auto end = std::chrono::high_resolution_clock::now();

  std::cout << info << " thread count " << tCount << " duration is " << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count()
            << std::endl;
}
//---------------------------------------------------------------------------------------
static constexpr uint32_t testObjectsCount_ = 100000;
//---------------------------------------------------------------------------------------
template <class K>
void fillMapTestObjects(std::vector<K>& vec) {
  for (uint64_t i = 0; i < testObjectsCount_; i++) {
    vec.emplace_back(random() * i);
  }
}
//---------------------------------------------------------------------------------------
template <>
void fillMapTestObjects<std::string>(std::vector<std::string>& vec) {
  for (uint64_t i = 0; i < testObjectsCount_; i++) {
    vec.emplace_back("item-" + std::to_string(random() * i));
  }
}
//---------------------------------------------------------------------------------------
template <class K, uint32_t TCOUNT>
void testExpList(const std::string& comment) {
  std::vector<K> testObjects[TCOUNT];
  libzrvan::ds::ExpMap<K, testObjectMap> map;

  // fill test objects
  for (uint32_t i = 0; i < TCOUNT; i++) {
    fillMapTestObjects<K>(testObjects[i]);
  }

  // add test function
  auto fillFunc = [&](uint32_t index) {
    for (auto i : testObjects[index]) {
      testObjectMap obj;
      map.add(i, obj, 10);
    }
  };

  // read only search function
  auto searchFuncRead = [&](uint32_t index) {
    uint64_t total = 0;
    for (auto i : testObjects[index]) {
      if (map.findR(i)) {
        total++;
      }
    }

    if (total != testObjectsCount_) {
      std::cout << "invalid match count " << total << std::endl;
    }
  };

  // read and write ssearch function
  auto searchFuncWrite = [&](uint32_t index) {
    uint64_t total = 0;
    for (auto i : testObjects[index]) {
      if (map.findW(i)) {
        total++;
      }
    }

    if (total != testObjectsCount_) {
      std::cout << "invalid match count " << total << std::endl;
    }
  };

  runMapTest("test exp map insert performance (" + comment + ")", fillFunc, TCOUNT);
  runMapTest("test exp map read performance (" + comment + ")", searchFuncRead, TCOUNT);
  runMapTest("test exp map write performance (" + comment + ")", searchFuncWrite, TCOUNT);
}
//---------------------------------------------------------------------------------------
TEST(ds, exp_map_test_performance) {
#define __TEST_EXP_MAP_WITH_T(TCOUNT)      \
  testExpList<uint64_t, TCOUNT>("uint64"); \
  testExpList<std::string, TCOUNT>("string");\
  std::cout << std::endl;

  __TEST_EXP_MAP_WITH_T(1)
  __TEST_EXP_MAP_WITH_T(2)
  __TEST_EXP_MAP_WITH_T(4)
  __TEST_EXP_MAP_WITH_T(8)
  __TEST_EXP_MAP_WITH_T(12)
  
}
//---------------------------------------------------------------------------------------
template <class K, class KEYTYPE, uint32_t TCOUNT>
void testUnorderedMap(const std::string& comment) {
  std::vector<K> testObjects[TCOUNT];
  std::unordered_map<K, testObjectMap> map;
  KEYTYPE lock;

  // fill test objects
  for (uint32_t i = 0; i < TCOUNT; i++) {
    fillMapTestObjects<K>(testObjects[i]);
  }

  // add test function
  auto fillFunc = [&](uint32_t tindex) {
    for (auto i : testObjects[tindex]) {
      testObjectMap obj;
      lock.lock();
      map.emplace(i, obj);
      lock.unlock();
    }
  };

  // read only search function
  auto searchFuncRead = [&](uint32_t tindex) {
    uint64_t total = 0;
    for (auto i : testObjects[tindex]) {
      lock.lock_shared();
      if (map.find(i) != map.end()) {
        total++;
      }
      lock.unlock_shared();
    }
    if (total != testObjectsCount_) {
      std::cout << "invalid match count " << total << std::endl;
    }
  };

  // read and write search function
  auto searchFuncWrite = [&](uint32_t tindex) {
    uint64_t total = 0;
    for (auto i : testObjects[tindex]) {
      lock.lock();
      if (map.find(i) != map.end()) {
        total++;
      }
      lock.unlock();
    }
    if (total != testObjectsCount_) {
      std::cout << "invalid match count " << total << std::endl;
    }
  };

  runMapTest("test unordered map insert performance (" + comment + ")", fillFunc, TCOUNT);
  runMapTest("test unordered map read performance (" + comment + ")", searchFuncRead, TCOUNT);
  runMapTest("test unordered map write performance (" + comment + ")", searchFuncWrite, TCOUNT);
}
//---------------------------------------------------------------------------------------
TEST(ds, exp_map_test_unordered_map_performance) {
#define __TEST_MAP_WITH_T(TCOUNT)                                                                                    \
  testUnorderedMap<uint64_t, std::shared_mutex, TCOUNT>("uint64_t with std::shared_mutex");                          \
  testUnorderedMap<uint64_t, std::shared_mutex, TCOUNT>("string with std::shared_mutex");                            \
  std::cout << std::endl;                                                                                            \
  testUnorderedMap<std::string, libzrvan::utils::RWSpinLock<>, TCOUNT>("uint64_t with libzrvan::utils::RWSpinLock"); \
  testUnorderedMap<std::string, libzrvan::utils::RWSpinLock<>, TCOUNT>("string with libzrvan::utils::RWSpinLock"); \
  std::cout << std::endl;

  __TEST_MAP_WITH_T(1)
  __TEST_MAP_WITH_T(2)
  __TEST_MAP_WITH_T(4)
  __TEST_MAP_WITH_T(12)
  
#undef __TEST_MAP_WITH_T
}