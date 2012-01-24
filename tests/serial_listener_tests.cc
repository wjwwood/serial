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

void filter_handler(std::string token) {
  global_listen_count++;
  std::cout << "filter_handler got: " << token << std::endl;
  return true;
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
