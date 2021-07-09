#pragma once
#include <gtest/gtest.h>
#include <stdio.h>
#include <mutex>
#include <shared_mutex>
#include <thread>
#include <vector>
#include "../../../include/utils/RWSpinLock.hpp"
#include "../../../include/utils/SpinLock.hpp"
//---------------------------------------------------------------------------------------
TEST(utils, spinlock_test_functionality) {
  libzrvan::utils::SpinLock<> lock;

  lock.lock();
  EXPECT_EQ(lock.try_lock(), false);
  lock.unlock();
  EXPECT_EQ(lock.try_lock(), true);
  lock.unlock();
}
//---------------------------------------------------------------------------------------
TEST(utils, rwlock_test_functionality) {
  libzrvan::utils::RWSpinLock<> lock;

  lock.lock();
  EXPECT_EQ(lock.try_lock(), false);
  EXPECT_EQ(lock.try_lock_shared(), false);
  lock.unlock();

  lock.lock_shared();
  EXPECT_EQ(lock.try_lock(), false);
  lock.unlock_shared();

  lock.lock_shared();
  EXPECT_EQ(lock.try_lock_shared(), true);
  lock.unlock_shared();
  lock.unlock_shared();

}
//---------------------------------------------------------------------------------------
template <typename LOCKTYPE>
void testWriteLock() {
  uint64_t counter = 0;
  static const uint64_t threadCount = 64;
  static const uint64_t loopCount = 128000;
  static const uint64_t target = (threadCount * loopCount);
  std::vector<std::thread> threads;
  LOCKTYPE lock;

  auto threadRt = [&]() {
    for (uint64_t i = 0; i < loopCount; i++) {
      lock.lock();
      counter++;
      lock.unlock();
    }
  };

  for (uint64_t i = 0; i < threadCount; i++) {
    threads.emplace_back(std::thread(threadRt));
  }

  for (auto& t : threads) {
    t.join();
  }

  EXPECT_EQ(counter, target);
}
//---------------------------------------------------------------------------------------
template <typename LOCKTYPE>
void testRWLock() {
  uint64_t counter = 0;
  static const uint64_t readThreadCount = 64;
  static const uint64_t writeThreadCount = 8;
  static const uint64_t loopCount = 128000;
  static const uint64_t target = (writeThreadCount * loopCount);
  std::vector<std::thread> threads;
  LOCKTYPE lock;

  //read thread
  auto threadR = [&]() {
    uint32_t val = 0;
    while (true) {
      lock.lock_shared();
      val = counter;
      lock.unlock_shared();
      if (val == target) {
        break;
      }
    }
  };

  //write thread
  auto threadW = [&]() {
    for (uint64_t i = 0; i < loopCount; i++) {
      lock.lock();
      counter++;
      lock.unlock();
    }
  };

  //
  for (uint64_t i = 0; i < writeThreadCount; i++) {
    threads.emplace_back(std::thread(threadW));
  }

  for (uint64_t i = 0; i < readThreadCount; i++) {
    threads.emplace_back(std::thread(threadR));
  }

  for (auto& t : threads) {
    t.join();
  }

  EXPECT_EQ(counter, target);
}
//---------------------------------------------------------------------------------------
TEST(utils, rwlock_test_read_write_performance_rw_spin_lock) { testRWLock<libzrvan::utils::RWSpinLock<>>(); }
//---------------------------------------------------------------------------------------
TEST(utils, wlock_test_write_performance_std_shared_mutex) { testWriteLock<std::shared_mutex>(); }
//---------------------------------------------------------------------------------------
TEST(utils, wlock_test_write_performance_std_mutex) { testWriteLock<std::mutex>(); }
//---------------------------------------------------------------------------------------
struct pthreadMutexWraper {
  pthread_mutex_t lock_;
  pthreadMutexWraper() { pthread_mutex_init(&lock_, 0); }
  void lock() { pthread_mutex_lock(&lock_); }
  void unlock() { pthread_mutex_unlock(&lock_); }
};

TEST(utils, wlock_test_write_performance_PthreadMutex) { testWriteLock<pthreadMutexWraper>(); }
//---------------------------------------------------------------------------------------
TEST(utils, wlock_test_write_performance_SpinLock) { testWriteLock<libzrvan::utils::SpinLock<>>(); }
//---------------------------------------------------------------------------------------
TEST(utils, wlock_test_write_performance_RWSpinLock) { testWriteLock<libzrvan::utils::RWSpinLock<>>(); }
//---------------------------------------------------------------------------------------
struct pthreadSpinLockWraper {
  pthread_spinlock_t lock_;
  pthreadSpinLockWraper() { pthread_spin_init(&lock_, 0); }
  void lock() { pthread_spin_lock(&lock_); }
  void unlock() { pthread_spin_unlock(&lock_); }
};

TEST(utils, wlock_test_write_performance_PthreadSpinLock) { testWriteLock<pthreadSpinLockWraper>(); }
//---------------------------------------------------------------------------------------
struct pthreadRWLockWraper {
  pthread_rwlock_t lock_;
  pthreadRWLockWraper() { pthread_rwlock_init(&lock_, 0); }
  void lock() { pthread_rwlock_wrlock(&lock_); }
  void unlock() { pthread_rwlock_unlock(&lock_); }
};

TEST(utils, wlock_test_write_performance_PthreadRWLock) { testWriteLock<pthreadRWLockWraper>(); }
//---------------------------------------------------------------------------------------
