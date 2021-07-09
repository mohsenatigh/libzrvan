#pragma once

#include <atomic>
#include <cstdint>
#include "SpinLock.hpp"

namespace libzrvan {
namespace utils {

/**
 * @brief This counter class is a lockless mechanism for maintaining statistics in a high-performance environment (instead of atomic variables). It is
 * a suitable mechanism for counters that update frequently but read rarely. In exchange for highly optimized write performance, It uses a lot more
 * memory and slow read operation.
 *
 * Please note that type T must accept negative values
 */
template <class T, uint32_t maxThreads = 512>
class Counter {
 private:
  //-------------------------------------------------------------------------------------
  /**
   * @brief shared variables
   *
   */
  static inline std::atomic<int> activeThreads_;
  static inline thread_local int threadIndex_ = -1;

  //-------------------------------------------------------------------------------------
  /**
   * @brief class local varialbles
   *
   */
  T counters_[maxThreads] = {0};

  //-------------------------------------------------------------------------------------
  /**
   * @brief Allocating or returning a unique index for each active thread
   *
   * @return int
   */
  int getIndex() {
    if (threadIndex_ == -1) {
      threadIndex_ = activeThreads_.fetch_add(1, std::memory_order_relaxed);
    }
    return threadIndex_;
  }

 public:
  Counter() {}
  ~Counter() = default;

  //-------------------------------------------------------------------------------------
  /**
   * @brief
   *
   * @param val
   */
  void add(const T& val) { counters_[getIndex()] += val; }

  //-------------------------------------------------------------------------------------
  /**
   * @brief
   *
   * @param val
   */
  void sub(const T& val) { counters_[getIndex()] -= val; }

  //-------------------------------------------------------------------------------------
  /**
   * @brief
   *
   * @param val
   */
  void operator+=(const T& val) { add(val); }

  //-------------------------------------------------------------------------------------
  /**
   * @brief
   *
   * @param val
   */
  void operator-=(const T& val) { sub(val); }

  //-------------------------------------------------------------------------------------
  /**
   * @brief get the result
   *
   * @return T
   */
  T get() {
    T sum(0);
    for (auto& i : counters_) {
      sum += i;
    }
    return sum;
  }

};  // namespace utils
}  // namespace utils
}  // namespace libzrvan
