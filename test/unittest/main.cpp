#include <iostream>
#include "utils/DeferTest.hpp"
#include "ds/ExpSlotList.hpp"
#include "ds/ExpMap.hpp"
#include "utils/CounterTest.hpp"
#include "utils/CoreHash.hpp"
#include "utils/FastHash.hpp"
#include "utils/Lock.hpp"
#include "utils/StaticLoop.hpp"
#include "utils/Time.hpp"

//---------------------------------------------------------------------------------------
int main(int argc, char* argv[]) {

  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
