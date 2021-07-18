#pragma once

#include <atomic>
#include <cstdint>
#include <immintrin.h>
namespace libzrvan {
namespace utils {

/**
 * @brief Simple spinlock implementation using std::atomic. in low contention
 * situations this lock act as a simple spinlock but in highly contention
 * systems it acts as a sleeping mutex. When it is acting as a sleeping mutex
 * the performance is highly dependent on the OS scheduling algorithm and timer
 *
 * @tparam MaxLoopBeforeSleep
 */
template <uint32_t MaxLoopBeforeSleep = 10> class SpinLock {
private:
  std::atomic<bool> lock_ = {false};
  //-------------------------------------------------------------------------------------
  inline void pause(uint32_t &counter) {
    _mm_pause();
    if (MaxLoopBeforeSleep && ++counter > MaxLoopBeforeSleep) {
      timespec t;
      timespec r;
      t.tv_nsec = 1;
      t.tv_sec = 0;
      nanosleep(&t, &r);
      counter = 0;
    }
  }
  //-------------------------------------------------------------------------------------
public:
  /**
   * @brief lock the spinlock
   *
   */
  void lock() {
    uint32_t counter = 0;
    while (try_lock() == false) {
      pause(counter);
    }
  }
  //-------------------------------------------------------------------------------------
  /**
   * @brief Unlock the spinlock
   *
   */
  void unlock() { lock_.store(false, std::memory_order_release); }
  //-------------------------------------------------------------------------------------
  /**
   * @brief Tries to lock the spinlock
   *
   * @return true
   * @return false
   */
  bool try_lock() {
    if (locked()) {
      return false;
    }

    if (!lock_.exchange(true, std::memory_order_acquire)) {
      return true;
    }

    return false;
  }
  //-------------------------------------------------------------------------------------
  /**
   * @brief Get the lock status
   *
   * @return true
   * @return false
   */
  bool locked() { return lock_.load(std::memory_order_relaxed); }

}; // namespace utils
} // namespace utils
} // namespace libzrvan
