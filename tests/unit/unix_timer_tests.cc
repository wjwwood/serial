#include "gtest/gtest.h"
#include "serial/impl/unix.h"

#include <unistd.h>
#include <stdlib.h>

using serial::MillisecondTimer;

namespace {

/**
 * Do 100 trials of timing gaps between 0 and 19 milliseconds.
 * Expect accuracy within one millisecond.
 */
TEST(timer_tests, short_intervals) {
  for (int trial = 0; trial < 100; trial++)
  {
    uint32_t ms = rand() % 20;
    MillisecondTimer mt(ms);
    usleep(1000 * ms);
    int32_t r = mt.remaining(); 

    // 1ms slush, for the cost of calling usleep.
    EXPECT_NEAR(r+1, 0, 1);
  }
}

TEST(timer_tests, overlapping_long_intervals) {
  MillisecondTimer* timers[10];

  // Experimentally determined. Corresponds to the extra time taken by the loops,
  // the big usleep, and the test infrastructure itself.
  const int slush_factor = 14;

  // Set up the timers to each time one second, 1ms apart.
  for (int t = 0; t < 10; t++)
  {
    timers[t] = new MillisecondTimer(1000);
    usleep(1000);
  }

  // Check in on them after 500ms.
  usleep(500000);
  for (int t = 0; t < 10; t++)
  {
    EXPECT_NEAR(timers[t]->remaining(), 500 - slush_factor + t, 5);
  }

  // Check in on them again after another 500ms and free them.
  usleep(500000);
  for (int t = 0; t < 10; t++)
  {
    EXPECT_NEAR(timers[t]->remaining(), -slush_factor + t, 5);
    delete timers[t];
  }
}

}  // namespace

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
