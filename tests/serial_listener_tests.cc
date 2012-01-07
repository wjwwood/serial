#include "gtest/gtest.h"

#include <boost/bind.hpp>

// OMG this is so nasty...
#define private public
#define protected public

#include "serial/serial_listener.h"
using namespace serial;

static size_t global_count, global_listen_count;

void default_handler(std::string line) {
  global_count++;
  // std::cout << "default_handler got: " << line << std::endl;
}

namespace {

class SerialListenerTests : public ::testing::Test {
protected:
  virtual void SetUp() {
    listener.default_handler = default_handler;
  }

  void execute_listenForStringOnce() {
    listener.listenForStringOnce("?$1E", 1000);
  }

  SerialListener listener;

};

TEST_F(SerialListenerTests, ignoresEmptyString) {
  global_count = 0;

  listener.listenOnce("");
  boost::this_thread::sleep(boost::posix_time::milliseconds(11));
  listener.listenOnce("");

  ASSERT_TRUE(global_count == 0);
}

TEST_F(SerialListenerTests, ignoresPartialMessage) {
  global_count = 0;

  listener.listenOnce("?$1E\r$1E=Robo");
  boost::this_thread::sleep(boost::posix_time::milliseconds(11));
  listener.listenOnce("");

  ASSERT_EQ(global_count, 1);
}

TEST_F(SerialListenerTests, listenForOnceWorks) {
  global_count = 0;

  boost::thread t(
    boost::bind(&SerialListenerTests::execute_listenForStringOnce, this));

  boost::this_thread::sleep(boost::posix_time::milliseconds(100));

  listener.listenOnce("\r+\r?$1E\r$1E=Robo");
  boost::this_thread::sleep(boost::posix_time::milliseconds(11));
  listener.listenOnce("");

  ASSERT_TRUE(t.timed_join(boost::posix_time::milliseconds(1500)));

  // Make sure the filters are getting deleted
  ASSERT_EQ(listener.filters.size(), 0);

  ASSERT_EQ(global_count, 1);
}

// lookForOnce should not find it, but timeout after 1000ms, so it should 
//  still join.
TEST_F(SerialListenerTests, listenForOnceTimesout) {
  global_count = 0;

  boost::thread t(
    boost::bind(&SerialListenerTests::execute_listenForStringOnce, this));

  boost::this_thread::sleep(boost::posix_time::milliseconds(100));

  listener.listenOnce("\r+\r?$1ENOTRIGHT\r$1E=Robo");
  boost::this_thread::sleep(boost::posix_time::milliseconds(11));
  listener.listenOnce("");

  ASSERT_TRUE(t.timed_join(boost::posix_time::milliseconds(1500)));

  ASSERT_EQ(global_count, 2);
}

bool listenForComparator(std::string line) {
  if (line.substr(0,2) == "V=") {
    return true;
  }
  return false;
}

void listenForCallback(std::string line) {
  global_listen_count++;
}

TEST_F(SerialListenerTests, listenForWorks) {
  global_count = 0;
  global_listen_count = 0;

  boost::uuids::uuid filt_uuid = 
    listener.listenFor(listenForComparator, listenForCallback);

  listener.listenOnce("\r+\rV=05:06\r?$1E\rV=06:05\r$1E=Robo");
  boost::this_thread::sleep(boost::posix_time::milliseconds(11));
  listener.listenOnce("");

  ASSERT_EQ(global_count, 2);
  ASSERT_EQ(global_listen_count, 2);

  listener.stopListeningFor(filt_uuid);

  ASSERT_EQ(listener.filters.size(), 0);

}

}  // namespace

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
