#pragma once
#include <immintrin.h>
#include <chrono>
#include <cstdint>
#include <thread>
#include "SpinLock.hpp"
namespace libzrvan {
namespace utils {
class Time {
 private:
  //-------------------------------------------------------------------------------------
  static inline uint64_t timeSec_ = 0;
  static inline uint64_t timeMSec_ = 0;
  static inline SpinLock<> startControlLock_;
  //-------------------------------------------------------------------------------------

  static void updateTimers() {
    auto cTime = std::chrono::high_resolution_clock::now().time_since_epoch();
    timeMSec_ = std::chrono::duration_cast<std::chrono::milliseconds>(cTime).count();
    timeSec_ = std::chrono::duration_cast<std::chrono::seconds>(cTime).count();
  }
  //-------------------------------------------------------------------------------------
  /**
   * @brief Timers update background routine
   *
   */
  static void timeUpdateFunc() {
    while (true) {
      timespec t;
      timespec r;
      t.tv_nsec = 1000000;
      t.tv_sec = 0;
      nanosleep(&t, &r);
      updateTimers();
    }
  }
  //-------------------------------------------------------------------------------------
  /**
   * @brief 
   *
   */
  static void startUpdate() {
    startControlLock_.lock();
    if (timeMSec_ == 0 && timeMSec_ == 0) {
      updateTimers();
      std::thread th(timeUpdateFunc);
      th.detach();
    }
    startControlLock_.unlock();
  }

 public:
  //-------------------------------------------------------------------------------------
  /**
   * @brief Get the Time object
   *
   * @return uint64_t
   */
  static uint64_t getTime() {
    if (timeSec_ == 0) {
      startUpdate();
    }
    return timeSec_;
  }

  //-------------------------------------------------------------------------------------
  /**
   * @brief Get the Time M S object
   *
   * @return uint64_t
   */
  static uint64_t getTimeMS() {
    if (timeMSec_ == 0) {
      startUpdate();
    }
    return timeMSec_;
  }
};
}  // namespace utils
}  // namespace libzrvan
