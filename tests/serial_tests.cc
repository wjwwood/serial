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

#include "serial/serial.h"
using namespace serial;

namespace {

class SerialTests : public ::testing::Test {
protected:
  virtual void SetUp() {
    port1 = new Serial(SERIAL_PORT_NAME, 115200, 250);
  }

  virtual void TearDown() {
    port1->close();
    delete port1;
  }

  Serial * port1;

};

// TEST_F(SerialTests, throwsOnInvalidPort) {
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
