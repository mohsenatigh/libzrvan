#pragma once
#include <gtest/gtest.h>
#include "../../../include/utils/Defer.hpp"
#include "../../../include/utils/SpinLock.hpp"

//---------------------------------------------------------------------------------------
TEST(utils, defer_test) {
  libzrvan::utils::SpinLock lock;

  auto testFunc = [&]() {
    libzrvan::utils::Defer d([&](){lock.unlock();});
    lock.lock();
  };

  testFunc();

  EXPECT_EQ(lock.try_lock(), true);
  lock.unlock();
}