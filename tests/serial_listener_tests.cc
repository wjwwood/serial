/* To run these tests you need to change the define below to the serial port 
 * with a loop back device attached.
 * 
 * Alternatively you could use an Arduino:
 * 
 *     void setup()
 *     {
 *       Serial.begin(115200);
 *     }
 *     
 *     void loop()
 *     {
 *       while (Serial.available() > 0) {
 *         Serial.write(Serial.read());
 *       }
 *     }
 * 
 */

// #define SERIAL_PORT_NAME "/dev/tty.usbserial-A900cfJA"
#define SERIAL_PORT_NAME "p0"

#include "gtest/gtest.h"

#include <boost/bind.hpp>

// OMG this is so nasty...
#define private public
#define protected public

#include "serial/serial_listener.h"
using namespace serial;

static size_t global_count, global_listen_count;
static bool matched;

void filter_handler(std::string token) {
  global_listen_count++;
  std::cout << "filter_handler got: " << token << std::endl;
}

void default_handler(std::string line) {
  global_count++;
  std::cout << "default_handler got: " << line << std::endl;
}

namespace {

void my_sleep(long milliseconds) {
  boost::this_thread::sleep(boost::posix_time::milliseconds(milliseconds));
}

class SerialListenerTests : public ::testing::Test {
protected:
  virtual void SetUp() {
    port1 = new Serial("/dev/pty"SERIAL_PORT_NAME, 115200, 10);
    port2 = new Serial("/dev/tty"SERIAL_PORT_NAME, 115200, 250);

    listener.setDefaultHandler(default_handler);
    listener.startListening((*port1));
  }

  virtual void TearDown() {
    listener.stopListening();
    delete port1;
    delete port2;
  }

  SerialListener listener;
  Serial * port1;
  Serial * port2;

};

TEST_F(SerialListenerTests, handlesPartialMessage) {
  global_count = 0;
  std::string input_str = "?$1E\r$1E=Robo";

  std::cout << "writing: ?$1E<cr>$1E=Robo" << std::endl;
  port2->write(input_str);
  // Allow time for processing
  my_sleep(50);

  ASSERT_EQ(1, global_count);

  input_str = "?$1E\r$1E=Roboteq\r";
  std::cout << "writing: ?$1E<cr>$1E=Roboteq<cr>" << std::endl;
  port2->write(input_str);
  // Allow time for processing
  my_sleep(50);

  ASSERT_EQ(3, global_count);
}

TEST_F(SerialListenerTests, normalFilterWorks) {
  global_count = 0;
  global_listen_count = 0;
  std::string input_str = "?$1E\r$1E=Robo\rV=1334:1337\rT=123";

  // Setup filter
  FilterPtr filt_1 =
    listener.createFilter(SerialListener::startsWith("V="), filter_handler);

  std::cout << "writing: ?$1E<cr>$1E=Robo<cr>V=1334:1337<cr>T=123";
  std::cout << std::endl;
  port2->write(input_str);
  // Allow time for processing
  my_sleep(50);

  ASSERT_EQ(2, global_count);
  ASSERT_EQ(1, global_listen_count);
}

void run_blocking_filter(BlockingFilterPtr filt_1) {
  // Wait 100 ms for a match
  std::string temp = filt_1->wait(100);
  if (temp.empty()) {
    return;
  }
  std::cout << "blocking filter matched: " << temp << std::endl;
  global_listen_count++;
  matched = true;
}

TEST_F(SerialListenerTests, blockingFilterWorks) {
  global_count = 0;
  global_listen_count = 0;
  matched = false;
  std::string input_str = "?$1E\r$1E=Robo\rV=1334:1337\rT=123";

  // Setup blocking filter
  BlockingFilterPtr filt_1 =
    listener.createBlockingFilter(SerialListener::startsWith("$1E="));

  boost::thread t(boost::bind(run_blocking_filter, filt_1));

  std::cout << "writing: ?$1E<cr>$1E=Robo<cr>V=1334:1337<cr>T=123";
  std::cout << std::endl;
  port2->write(input_str);
  // Allow time for processing
  my_sleep(50);

  using boost::posix_time::milliseconds;
  ASSERT_TRUE(t.timed_join(milliseconds(10)));
  ASSERT_EQ(2, global_count);
  ASSERT_EQ(1, global_listen_count);
  ASSERT_TRUE(matched);
}

TEST_F(SerialListenerTests, blockingFilterTimesOut) {
  global_count = 0;
  global_listen_count = 0;
  matched = false;
  std::string input_str = "?$1E\r$1E=Robo\rV=1334:1337\rT=123";

  // Setup blocking filter
  BlockingFilterPtr filt_1 =
    listener.createBlockingFilter(SerialListener::startsWith("T="));

  boost::thread t(boost::bind(run_blocking_filter, filt_1));

  std::cout << "writing: ?$1E<cr>$1E=Robo<cr>V=1334:1337<cr>T=123";
  std::cout << std::endl;
  port2->write(input_str);
  // Allow time for processing
  my_sleep(50);

  using boost::posix_time::milliseconds;
  // First one should not be within timeout, should be false
  ASSERT_FALSE(t.timed_join(milliseconds(10)));
  // Second one should capture timeout and return true to join
  ASSERT_TRUE(t.timed_join(milliseconds(60)));
  ASSERT_EQ(3, global_count);
  ASSERT_EQ(0, global_listen_count);
  ASSERT_FALSE(matched);
}

void write_later(Serial *port, std::string input_str, long wait_for) {
  my_sleep(wait_for);
  port->write(input_str);
}

TEST_F(SerialListenerTests, bufferedFilterWorks) {
  global_count = 0;
  std::string input_str = "?$1E\r+\r$1E=Robo\rV=1334:1337\rT=123";

  // Setup buffered filter, buffer size 3
  BufferedFilterPtr filt_1 =
    listener.createBufferedFilter(SerialListener::exactly("+"), 3);

  // Write the string to the port 10 ms in the future
  boost::thread t(boost::bind(write_later, port2, input_str, 10));

  // This should be empty because of a timeout
  ASSERT_TRUE(filt_1->wait(2).empty());
  // Make sure wait works properly
  ASSERT_EQ("+", filt_1->wait(20));
  // This should be empty cause there was only one
  ASSERT_TRUE(filt_1->wait(2).empty());
  // The queue in the filter should be empty
  ASSERT_EQ(0, filt_1->queue.size());
  ASSERT_EQ(3, global_count);
  t.join();
}

TEST_F(SerialListenerTests, bufferedFilterQueueWorks) {
  global_count = 0;
  std::string input_str = "?$1E$\r+\r$1E=Robo$\rV=1334:1337$\rT=123$\r";

  // Setup buffered filter, buffer size 3
  BufferedFilterPtr filt_1 =
    listener.createBufferedFilter(SerialListener::endsWith("$"), 3);

  // write the string
  port2->write(input_str);

  my_sleep(20); // Let things process
  // There should have been four matches
  //   therefore the first one should the second match.
  ASSERT_EQ("$1E=Robo$", filt_1->wait(1));
  ASSERT_EQ("V=1334:1337$", filt_1->wait(1));
  ASSERT_EQ("T=123$", filt_1->wait(1));
  ASSERT_EQ(0, filt_1->queue.size());

  ASSERT_EQ(1, global_count);
}

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
