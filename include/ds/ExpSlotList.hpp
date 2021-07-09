#pragma once

#include <cstdint>
#include <functional>
#include "../utils/RWSpinLock.hpp"
#include "../utils/StaticLoop.hpp"
#include "../utils/Time.hpp"
namespace libzrvan {
namespace ds {

/**
 * @brief Thread-safe slot-link list with expiration capability. like many other tools in
 * this library, it uses high memory to increase performance. It uses 2 separate lists,
 * one for storing the key elements and another list for storing actual objects. using this
 * technique will cause a constant list traversal and search speed without dependency on the
 * object size.
 *
 * It is possible to have a duplicate key in one list
 *
 * To prevent race conditions and object escape, it is important to access the objects in
 * the callback functions and don't keep a reference to a the stored objects
 *
 * This list support TTL (time to live) It means it is possible to remove objects after a defined
 * interval from the last access
 *
 * if EXTEND_LIFE_ON_ACCESS equal true the DS extend the lifetime of the object after each access
 * by the defined interval
 */
template <class T, bool EXTEND_LIFE_ON_ACCESS = true>
class ExpSlotList {
 public:
  /**
   * @brief Match function. Allowing access to the stored object in the list in read-only mode
   */
  using MatchFunction = std::function<bool(T&)>;

 private:
  /**
   * @brief Slot class, the main list is a linked list of this slot object. each slot is a constant
   *  array of objects
   *
   * @tparam T object type
   */
  class Slot {
   private:
    /**
     * @brief Hold object + TTL informations
     *
     */
    struct SlotDataInfo {
      T item;
      uint32_t accessTime;
      uint32_t lifeTime;
    };

   private:
    static constexpr uint32_t maxSlotItems_ = 64;
    static constexpr uint64_t slotFullFlag_ = 0xffffffffffffffff;
#define __SLOTLIST_STATIC_LOOP_FUNC STATIC_LOOP64

    uint64_t keyList_[maxSlotItems_];
    SlotDataInfo itemsList_[maxSlotItems_];
    uint64_t slotMask_ = 0;
    Slot* next_ = nullptr;
    Slot* prev_ = nullptr;

   public:
    //------------------------------------------------------------------------------------
    /**
     * @brief Add a new object to the slot
     *
     * @param key object key
     * @param object
     * @param expTime TTL value
     * @return true if the object was successfully added to the list
     * @return false
     */
    inline bool add(uint64_t key, const T& object, uint32_t expTime) {
      //
      auto addFunc = [&](uint32_t index, uint64_t mask) {
        keyList_[index] = key;
        itemsList_[index].item = object;
        itemsList_[index].lifeTime = expTime;
        itemsList_[index].accessTime = libzrvan::utils::Time::getTime();
        slotMask_ |= mask;
      };
      //
      if (full()) {
        return false;
      }

      // loop
      __SLOTLIST_STATIC_LOOP_FUNC({
        if ((_static_mask & slotMask_) == 0) {
          addFunc(_static_index, _static_mask);
          return true;
        }
      });
      return false;
    }
    //------------------------------------------------------------------------------------
    /**
     * @brief
     *
     * @param key object key
     * @param matchFunc  this function can be used to specify the object in case of a duplicate
     * key. it should return true if to object matches
     * @return true if the object was successfully removed from the list
     * @return false
     */
    inline bool remove(uint64_t key, MatchFunction matchFunc = nullptr) {
      //
      auto checkMatchFunc = [&](uint32_t index, uint64_t mask) -> bool {
        if (matchFunc && !matchFunc(itemsList_[index].item)) {
          return false;
        }
        slotMask_ &= ~mask;
        return true;
      };

      if (empty()) {
        return false;
      }

      __SLOTLIST_STATIC_LOOP_FUNC({
        if (key == keyList_[_static_index] && (_static_mask & slotMask_)) {
          if (checkMatchFunc(_static_index, _static_mask)) {
            return true;
          }
        }
      })
      return false;
    }
    //------------------------------------------------------------------------------------
    /**
     * @brief
     *
     * @param key object key
     * @param findFunc Allow access to the stored object and also find the desired object in case of a duplicate key.
     * @return true
     * @return false
     */
    inline bool find(uint64_t key, MatchFunction findFunc = nullptr) {
      //
      auto checkMatchFunc = [&](uint32_t index) -> bool {
        if (findFunc && !findFunc(itemsList_[index].item)) {
          return false;
        }

        if (EXTEND_LIFE_ON_ACCESS) {
          itemsList_[index].accessTime = libzrvan::utils::Time::getTime();
        }

        return true;
      };

      __SLOTLIST_STATIC_LOOP_FUNC({
        if (key == keyList_[_static_index] && (_static_mask & slotMask_)) {
          if (checkMatchFunc(_static_index)) {
            return true;
          }
        }
      });
      return false;
    }
    //------------------------------------------------------------------------------------
    /**
     * @brief Iterating through all the objects in the list
     *
     * @param matchFunc This function called for each object in the list. The iteration will
     * stop when it returns false
     * @return size_t number of items
     */
    inline size_t forEach(MatchFunction matchFunc) {
      size_t cnt = 0;
      //
      auto callFunc = [&](uint32_t index) -> bool {
        if (matchFunc) {
          matchFunc(itemsList_[index].item);
        }
        cnt++;
        return false;
      };

      __SLOTLIST_STATIC_LOOP_FUNC({
        if ((_static_mask & slotMask_)) {
          if (callFunc(_static_index)) {
            return cnt;
          }
        }
      });
      return cnt;
    }
    //------------------------------------------------------------------------------------
    /**
     * @brief
     *
     * @return true
     * @return false
     */
    inline bool empty() const { return slotMask_ == 0; }
    //------------------------------------------------------------------------------------
    /**
     * @brief
     *
     * @return true
     * @return false
     */
    inline bool full() const { return slotMask_ == slotFullFlag_; }
    //------------------------------------------------------------------------------------
    /**
     * @brief
     *
     * @param ctime current time  (relative time)
     * @param matchFunc Remove the expired object from the list if this function returns true
     * @return size_t
     */
    inline size_t expireCheck(uint32_t ctime, MatchFunction matchFunc = nullptr) {
      size_t count = 0;

      //
      auto checkFunc = [&](uint32_t index, uint64_t mask) {
        SlotDataInfo* info = &itemsList_[index];
        if (ctime - info->accessTime > info->lifeTime) {
          if (matchFunc && !matchFunc(itemsList_[index].item)) {
            return;
          }
          slotMask_ &= ~mask;
          count++;
        }
      };

      //
      __SLOTLIST_STATIC_LOOP_FUNC({
        if ((_static_mask & slotMask_)) {
          checkFunc(_static_index, _static_mask);
        }
      })
      return count;
    }
    //------------------------------------------------------------------------------------
    /**
     * @brief Add this slot to the slots link list
     *
     * @param root
     */
    inline void addToChain(Slot*& root) {
      if (root) {
        next_ = root;
        root->prev_ = this;
      }
      root = this;
    }
    //------------------------------------------------------------------------------------
    /**
     * @brief Remove this slot from slots link list
     *
     * @param root
     */
    inline void removeFromChain(Slot*& root) {
      if (next_) {
        next_->prev_ = prev_;
      }

      if (prev_) {
        prev_->next_ = next_;
      }

      if (root == this) {
        root = next_;
      }
    }
    //------------------------------------------------------------------------------------
    /**
     * @brief
     *
     * @return next slot in the list
     */
    inline Slot* next() { return next_; }
  };

 private:
  libzrvan::utils::RWSpinLock<> lock_;
  ExpSlotList::Slot* root_ = nullptr;
  size_t count_ = 0;
  //------------------------------------------------------------------------------------
  inline bool findI(uint64_t key, MatchFunction func) {
    ExpSlotList::Slot* slot = root_;
    while (slot) {
      if (slot->find(key, func)) {
        return true;
      }
      slot = slot->next();
    }
    return false;
  }
  //------------------------------------------------------------------------------------
  inline ExpSlotList::Slot* addNewSlot() {
    ExpSlotList::Slot* slot;
    slot = new ExpSlotList::Slot();
    slot->addToChain(root_);
    return slot;
  }
  //------------------------------------------------------------------------------------
  inline bool addI(uint64_t key, const T& object, uint32_t expTime) {
    ExpSlotList::Slot* slot = root_;
    // add to existings items

    if (slot && !slot->full() && slot->add(key, object, expTime)) {
      return true;
    }

    // add new item
    slot = addNewSlot();
    slot->add(key, object, expTime);
    return true;
  }
  //------------------------------------------------------------------------------------
  inline bool removeI(uint64_t key, MatchFunction func) {
    ExpSlotList::Slot* slot = root_;
    // check list
    while (slot) {
      if (slot->remove(key, func)) {
        if (slot->empty()) {
          slot->removeFromChain(root_);
          delete slot;
        }
        return true;
      }
      slot = slot->next();
    }
    return false;
  }
  //------------------------------------------------------------------------------------
  inline size_t checkI(uint32_t ctime, MatchFunction func) {
    size_t cnt = 0;
    ExpSlotList::Slot* slot = root_;
    while (slot) {
      cnt += slot->expireCheck(ctime, func);
      if (slot->empty()) {
        Slot* n = slot->next();
        slot->removeFromChain(root_);
        delete slot;
        slot = n;
      } else {
        slot = slot->next();
      }
    }
    return cnt;
  }
  //------------------------------------------------------------------------------------
  inline size_t swapI(ExpSlotList::Slot*& out) {
    size_t outCnt;
    lock_.lock();
    out = root_;
    root_ = nullptr;
    outCnt = count_;
    count_ = 0;
    lock_.unlock();
    return outCnt;
  }

 public:
  ExpSlotList() = default;

  //------------------------------------------------------------------------------------
  /**
   * @brief remove copy constructor
   *
   */
  ExpSlotList(const ExpSlotList&) = delete;
  //------------------------------------------------------------------------------------
  /**
   * @brief Move constructor
   *
   * @param obj
   */
  ExpSlotList(ExpSlotList&& obj) {
    lock_.lock();
    count_ = obj.swapI(root_);
    lock_.unlock();
  };
  //------------------------------------------------------------------------------------
  /**
   * @brief Destroy the Exp Slot List object
   *
   */
  ~ExpSlotList() { flush(); }
  //------------------------------------------------------------------------------------
  /**
   * @brief
   *
   * @param key object key
   * @param object
   * @param expTime TTL
   * @return true
   * @return false
   */
  bool add(uint64_t key, const T& object, uint32_t expTime) {
    lock_.lock();
    if (addI(key, object, expTime)) {
      count_++;
    }
    lock_.unlock();
    return true;
  }
  //------------------------------------------------------------------------------------
  /**
   * @brief
   *
   * @param key
   * @param func
   * @return true
   * @return false
   */
  bool remove(uint64_t key, MatchFunction func = nullptr) {
    bool res;
    lock_.lock();
    res = removeI(key, func);
    if (res) {
      count_--;
    }
    lock_.unlock();
    return res;
  }
  //------------------------------------------------------------------------------------
  /**
   * @brief Use this function to access the object in the read-only mode
   *
   * @param key
   * @param func
   * @return true
   * @return false
   */
  bool findR(uint64_t key, MatchFunction func = nullptr) {
    bool res;
    lock_.lock_shared();
    res = findI(key, func);
    lock_.unlock_shared();
    return res;
  }
  //------------------------------------------------------------------------------------
  /**
   * @brief Use this function to access the object in the read-write mode
   *
   * @param key
   * @param func
   * @return true
   * @return false
   */
  bool findW(uint64_t key, MatchFunction func = nullptr) {
    bool res;
    lock_.lock();
    res = findI(key, func);
    lock_.unlock();
    return res;
  }
  //------------------------------------------------------------------------------------
  /**
   * @brief
   *
   * @param func
   */
  void flush(MatchFunction func = nullptr) {
    lock_.lock();
    ExpSlotList::Slot* slot = root_;
    while (slot) {
      ExpSlotList::Slot* temp = slot;
      slot = slot->next();
      temp->forEach(func);
      delete temp;
    }
    root_ = nullptr;
    count_ = 0;
    lock_.unlock();
  }
  //------------------------------------------------------------------------------------
  /**
   * @brief Iterating through all the objects in the list
   *
   * @param matchFunc This function called for each object in the list. The iteration will
   * stop when it returns false
   * @return size_t number of items
   */
  size_t forEach(MatchFunction func) {
    size_t cnt = 0;
    lock_.lock_shared();
    ExpSlotList::Slot* slot = root_;
    while (slot) {
      cnt += slot->forEach(func);
      slot = slot->next();
    }
    lock_.unlock_shared();
    return cnt;
  }
  //------------------------------------------------------------------------------------
  /**
   * @brief
   *
   * @param ctime current time  (relative time)
   * @param matchFunc Remove the expired object from the list, if this function returns true
   * @return size_t
   */
  size_t expireCheck(uint32_t ctime, MatchFunction func = nullptr) {
    size_t rCount;

    // expire check is a low priority functionality.
    if (!lock_.try_lock()) {
      return 0;
    }

    if (!ctime) {
      ctime = libzrvan::utils::Time().getTime();
    }

    rCount = checkI(ctime, func);
    count_ -= rCount;
    lock_.unlock();
    return rCount;
  }
  //------------------------------------------------------------------------------------
  /**
   * @brief
   *
   * @return size_t
   */
  size_t size() { return count_; }
  //------------------------------------------------------------------------------------
  /**
   * @brief
   *
   * @return size_t
   */
  void preLoad() {
    if (root_ == nullptr) {
      lock_.lock();
      addNewSlot();
      lock_.unlock();
    }
  }
  //------------------------------------------------------------------------------------

};  // namespace ds
}  // namespace ds
}  // namespace libzrvan
