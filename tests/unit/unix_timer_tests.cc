#include <chrono>
#include <sstream>
#include <thread>

#include "gtest/gtest.h"
#include "serial/impl/unix.h"

using serial::MillisecondTimer;

namespace {

typedef std::chrono::high_resolution_clock Clock;


template <typename T>
std::chrono::milliseconds as_milliseconds(T duration)
{
    return std::chrono::duration_cast<std::chrono::milliseconds>(duration);
}


struct test_timer {
    MillisecondTimer timer;
    std::chrono::time_point<Clock> start;
    test_timer(unsigned value): start(Clock::now()), timer(value) {}
};


/**
 * Do 100 trials of timing gaps between 0 and 19 milliseconds.
 * Expect accuracy within one millisecond.
 */
TEST(timer_tests, short_intervals) {
  for (int trial = 0; trial < 100; trial++)
  {
      int ms = rand() % 20;
      auto t1 = Clock::now();
      MillisecondTimer mt(ms);
      std::this_thread::sleep_for(std::chrono::milliseconds(ms));
      int elapsed = as_milliseconds(Clock::now() - t1).count();
      EXPECT_NEAR(ms - elapsed, mt.remaining(), 1);
  }
}


TEST(timer_tests, overlapping_long_intervals) {

  std::vector<test_timer> timers;

  // Set up the timers to each time one second, 1 ms apart.
  while (timers.size() < 10) {
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
    struct test_timer timer(1000);  // 1 s
    timers.push_back(timer);
  }

  // Check in on them after 500 ms.
  std::this_thread::sleep_for(std::chrono::milliseconds(500));
  for (auto& t: timers) {
    auto elapsed = as_milliseconds(Clock::now() - t.start).count();
    EXPECT_NEAR(1000 - elapsed, t.timer.remaining(), 1);
  }

  // Check in on them again after another 500 ms.
  std::this_thread::sleep_for(std::chrono::milliseconds(500));
  for (auto& t: timers) {
    auto elapsed = as_milliseconds(Clock::now() - t.start).count();
    EXPECT_NEAR(1000 - elapsed, t.timer.remaining(), 1);
  }
}

}  // namespace


int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
