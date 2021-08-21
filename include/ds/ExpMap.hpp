#pragma once

#include "../utils/CoreHash.hpp"
#include "../utils/FastHash.hpp"
#include "../utils/Time.hpp"
#include "ExpSlotList.hpp"
#include <cstdint>

namespace libzrvan {
namespace ds {

//---------------------------------------------------------------------------------------
/**
 * @brief Thread-safe hash linked list data structure with the expiration
 * capability. Internally it uses an array of ExpSlotList. so it doesn't use  a
 * big lock for access control
 *
 * @tparam K  key type class
 * @tparam T
 * @tparam SEGCOUNT hash segment count
 * @tparam EXTEND_LIFE_ON_ACCESS considering the expiration time after the last
 * access instead of an absolute value
 * @tparam PRELOAD Preloading the hash segments. It will increase the insertion
 * speed in the cost of higher memory usage
 */
template <class K, class T, class HASH = utils::FastHash<K>,
          uint32_t SEGCOUNT = 256000, bool EXTEND_LIFE_ON_ACCESS = true,
          bool PRELOAD = true,
          class LOCK=libzrvan::utils::RWSpinLock<>>
class ExpMap {
public:
  using MatchFunc = std::function<bool(T &)>;

private:
  // hash segments
  ExpSlotList<T, EXTEND_LIFE_ON_ACCESS,LOCK> *segmensts_;
  uint32_t checkIndex_ = 0;
  std::atomic<size_t> count_ = 0;
  HASH hash_;

  //-------------------------------------------------------------------------------------
  inline uint32_t getSegment(uint64_t key) const { return (key % SEGCOUNT); }

public:
  //-------------------------------------------------------------------------------------
  /**
   * @brief Construct a new Exp Map objects
   *
   */
  ExpMap() {
    // warm the timer !
    libzrvan::utils::Time().getTime();

    // create segments lits
    segmensts_ = new ExpSlotList<T, EXTEND_LIFE_ON_ACCESS>[SEGCOUNT];
    if (PRELOAD) {
      for (uint32_t i = 0; i < SEGCOUNT; i++) {
        segmensts_[i].preLoad();
      }
    }
  }

  //-------------------------------------------------------------------------------------
  /**
   * @brief Disable copy and move constructor
   *
   */
  ExpMap(const ExpMap &) = delete;
  ExpMap(ExpMap &&obj) = delete;

  //-------------------------------------------------------------------------------------
  /**
   * @brief Destroy the Exp Map object
   *
   */
  ~ExpMap() { delete[] segmensts_; }
  //-------------------------------------------------------------------------------------
  /**
   * @brief
   *
   * @param key
   * @param value
   * @param expTime
   * @return true
   * @return false
   */
  bool add(const K &key, const T &value, uint32_t expTime) {
    uint64_t keyval = hash_(key);
    if (segmensts_[getSegment(keyval)].add(keyval, value, expTime)) {
      count_++;
      return true;
    }
    return false;
  }

  //-------------------------------------------------------------------------------------
  /**
   * @brief
   *
   * @param key
   * @param value
   * @param expTime
   * @param func Match function,  used for expireCheck
   * @return true
   * @return false
   */
  bool addAndCheck(const K &key, const T &value, uint32_t expTime,
                   MatchFunc func = nullptr) {
    expireCheck(0, func);
    return add(key, value, expTime);
  }
  //-------------------------------------------------------------------------------------
  /**
   * @brief
   *
   * @param key
   * @param func
   * @return true
   * @return false
   */
  bool remove(const K &key, MatchFunc func = nullptr) {
    uint64_t keyval = hash_(key);
    if (segmensts_[getSegment(keyval)].remove(keyval, func)) {
      count_--;
      return true;
    }
    return false;
  }
  //-------------------------------------------------------------------------------------
  /**
   * @brief
   *
   * @param key
   * @param func
   * @return true
   * @return false
   */
  bool findR(const K &key, MatchFunc func = nullptr) {
    uint64_t keyval = hash_(key);
    return segmensts_[getSegment(keyval)].findR(keyval, func);
  }
  //-------------------------------------------------------------------------------------
  /**
   * @brief
   *
   * @param key
   * @param func
   * @return true
   * @return false
   */
  bool findW(const K &key, MatchFunc func = nullptr) {
    uint64_t keyval = hash_(key);
    return segmensts_[getSegment(keyval)].findW(keyval, func);
  }
  //-------------------------------------------------------------------------------------
  /**
   * @brief
   *
   * @param func
   * @return size_t
   */
  size_t forEach(MatchFunc func = nullptr) const {
    size_t total = 0;
    for (uint32_t i = 0; i < SEGCOUNT; i++) {
      total += segmensts_[i].forEach(func);
    }
    return total;
  }
  //-------------------------------------------------------------------------------------
  /**
   * @brief
   *
   * @param cTime
   * @param func
   * @return size_t
   */
  size_t expireCheck(uint32_t cTime, MatchFunc func = nullptr) {
    uint32_t index = (checkIndex_++) % SEGCOUNT;

    if (!cTime) {
      cTime = libzrvan::utils::Time::getTime();
    }

    if (size_t ec = segmensts_[index].expireCheck(cTime, func); ec > 0) {
      count_ -= ec;
      return ec;
    }
    return 0;
  }
  //-------------------------------------------------------------------------------------
  /**
   * @brief Flush all the items and clean it
   *
   * @param func
   */
  void flush(MatchFunc func = nullptr) {
    count_ = 0;
    for (uint32_t i = 0; i < SEGCOUNT; i++) {
      segmensts_[i].flush(func);
    }
  }
  //-------------------------------------------------------------------------------------
  /**
   * @brief Get the Segments Count object
   *
   * @return constexpr uint32_t
   */
  constexpr uint32_t getSegmentsCount() const { return SEGCOUNT; }
  //-------------------------------------------------------------------------------------
  /**
   * @brief Get availabe items count
   *
   * @return constexpr uint32_t
   */
  constexpr size_t size() const { return count_; }
};
} // namespace ds
} // namespace libzrvan