/* To run these tests you need to change the define below to the serial port 
 * with a loop back device attached.
 * 
 * Alternatively you could use an Arduino:
 
 void setup()
 {
   Serial.begin(115200);
 }

 void loop()
 {
   while (Serial.available() > 0) {
     Serial.write(Serial.read());
   }
 }
 
 */

#define SERIAL_PORT_NAME "/dev/tty.usbserial"

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
  std::cout << "default_handler got: " << line << std::endl;
}

namespace {

class SerialListenerTests : public ::testing::Test {
protected:
  virtual void SetUp() {
    port1 = new Serial(SERIAL_PORT_NAME, 115200, 250);

    listener.setDefaultHandler(default_handler);
    listener.startListening((*port1));
  }

  virtual void TearDown() {
    listener.stopListening();
    port1->close();
    delete port1;
  }

  SerialListener listener;
  Serial * port1;

};

void my_sleep(long milliseconds) {
  boost::this_thread::sleep(boost::posix_time::milliseconds(milliseconds));
}

TEST_F(SerialListenerTests, handlesPartialMessage) {
  global_count = 0;
  std::string input_str = "?$1E\r$1E=Robo";

  port1->write(input_str);

  // give some time for the callback thread to finish
  my_sleep(1000);

  ASSERT_EQ(1, global_count);
}

// TEST_F(SerialListenerTests, listenForOnceWorks) {
//   global_count = 0;
// 
//   boost::thread t(
//     boost::bind(&SerialListenerTests::execute_listenForStringOnce, this));
// 
//   boost::this_thread::sleep(boost::posix_time::milliseconds(5));
// 
//   simulate_loop("\r+\r?$1E\r$1E=Robo");
// 
//   ASSERT_TRUE(t.timed_join(boost::posix_time::milliseconds(60)));
// 
//   // Make sure the filters are getting deleted
//   ASSERT_EQ(listener.filters.size(), 0);
// 
//   // give some time for the callback thread to finish
//   stopCallbackThread();
// 
//   ASSERT_EQ(global_count, 1);
// }
// 
// // lookForOnce should not find it, but timeout after 1000ms, so it should 
// //  still join.
// TEST_F(SerialListenerTests, listenForOnceTimesout) {
//   global_count = 0;
// 
//   boost::thread t(
//     boost::bind(&SerialListenerTests::execute_listenForStringOnce, this));
// 
//   boost::this_thread::sleep(boost::posix_time::milliseconds(55));
// 
//   simulate_loop("\r+\r?$1ENOTRIGHT\r$1E=Robo");
// 
//   ASSERT_TRUE(t.timed_join(boost::posix_time::milliseconds(60)));
// 
//   // give some time for the callback thread to finish
//   stopCallbackThread();
// 
//   ASSERT_EQ(global_count, 2);
// }
// 
// bool listenForComparator(std::string line) {
//   // std::cout << "In listenForComparator(" << line << ")" << std::endl;
//   if (line.substr(0,2) == "V=") {
//     return true;
//   }
//   return false;
// }
// 
// void listenForCallback(std::string line) {
//   // std::cout << "In listenForCallback(" << line << ")" << std::endl;
//   global_listen_count++;
// }
// 
// TEST_F(SerialListenerTests, listenForWorks) {
//   global_count = 0;
//   global_listen_count = 0;
// 
//   FilterPtr filt_uuid =
//     listener.listenFor(listenForComparator, listenForCallback);
// 
//   simulate_loop("\r+\rV=05:06\r?$1E\rV=06:05\r$1E=Robo");
// 
//   // give some time for the callback thread to finish
//   stopCallbackThread();
// 
//   ASSERT_EQ(global_count, 2);
//   ASSERT_EQ(global_listen_count, 2);
// 
//   listener.stopListeningFor(filt_uuid);
// 
//   ASSERT_EQ(listener.filters.size(), 0);
// 
// }

}  // namespace

int main(int argc, char **argv) {
  try {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
  } catch (std::exception &e) {
    std::cerr << "Unhandled Exception: " << e.what() << std::endl;
  }
  return 1;
}
