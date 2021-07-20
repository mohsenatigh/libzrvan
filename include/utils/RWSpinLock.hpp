#pragma once

#include <immintrin.h>
#include <atomic>
#include <cstdint>
#include "SpinLock.hpp"
namespace libzrvan {
namespace utils {

/**
 * @brief Simple RWspinlock implementation. in low contention situations this lock act as a simple spinlock but in highly contention
 * systems it acts as a mutex . When it is acting as a mutex the performance is highly dependent on the OS scheduling algorithm and timer. It also
 * supports Strong-writer mechanisms. It means we can prioritize writer threads over readers
 *
 * @tparam MaxLoopBeforeSleep
 * @tparam StrongWriter
 */
template <uint32_t MaxLoopBeforeSleep = 10, bool StrongWriter = true>
class RWSpinLock {
 private:
  std::atomic<int> users_ = {0};
  libzrvan::utils::SpinLock<MaxLoopBeforeSleep> wlock_;

  //-------------------------------------------------------------------------------------
  inline void pause(uint32_t& counter) {
    _mm_pause();
    if (MaxLoopBeforeSleep && ++counter == MaxLoopBeforeSleep) {
      timespec t;
      timespec r;
      t.tv_nsec = 1;
      t.tv_sec = 0;
      nanosleep(&t, &r);
      counter = 0;
    }
  }
  //-------------------------------------------------------------------------------------
  inline void write_lock_strong() {
    uint32_t loop = 0;
    wlock_.lock();
    while (users_.load(std::memory_order_relaxed) != 0) {
      pause(loop);
    }
  }

 public:
 //-------------------------------------------------------------------------------------
  /**
   * @brief Construct a new RWSpinLock object
   *
   */
  RWSpinLock() {}
  //-------------------------------------------------------------------------------------
  /**
   * @brief read lock
   *
   */
  void lock_shared() {
    uint32_t loop = 0;
    while (!try_lock_shared()) {
      pause(loop);
    }
  }
  //-------------------------------------------------------------------------------------
  /**
   * @brief Try to get the reader lock
   *
   */
  bool try_lock_shared() {
    // check write lock
    if (wlock_.locked()) {
      return false;
    }

    // add number of users
    users_.fetch_add(1, std::memory_order_relaxed);
    if (wlock_.locked()) {
      users_.fetch_sub(1, std::memory_order_relaxed);
      return false;
    }
    return true;
  }
  //-------------------------------------------------------------------------------------
  /**
   * @brief read unlock
   *
   */
  void unlock_shared() { users_.fetch_sub(1, std::memory_order_relaxed); }
  //-------------------------------------------------------------------------------------
  /**
   * @brief Write lock. If the strong writer is enabled. the caller has a higher priority over readers
   *
   */
  void lock() {
    uint32_t loop = 0;

    // try get the write lock
    if (StrongWriter == false) {
      while (try_lock() == false) {
        pause(loop);
      }
    } else {
      write_lock_strong();
    }
  }
  //-------------------------------------------------------------------------------------
  /**
   * @brief Trying to get the write lock. this function is always weak
   *
   */
  bool try_lock() {
    if (users_.load(std::memory_order_relaxed) != 0) {
      return false;
    }

    if (!wlock_.try_lock()) {
      return false;
    }

    if (users_.load(std::memory_order_relaxed) != 0) {
      wlock_.unlock();
      return false;
    }
    return true;
  }
  //-------------------------------------------------------------------------------------
  /**
   * @brief write unlock
   *
   */
  void unlock() { wlock_.unlock(); }

};  // namespace utils
}  // namespace utils
}  // namespace libzrvan
