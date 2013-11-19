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

#include <string>
#include "gtest/gtest.h"

#include <boost/bind.hpp>

// Use FRIEND_TEST... its not as nasty, thats what friends are for
// // OMG this is so nasty...
// #define private public
// #define protected public

#include "serial/serial.h"

#if defined(__linux__)
#include <pty.h>
#else
#include <util.h>
#endif

using namespace serial;

using std::string;

namespace {

class SerialTests : public ::testing::Test {
protected:
  virtual void SetUp() {
    if (openpty(&master_fd, &slave_fd, name, NULL, NULL) == -1) {
      perror("openpty");
      exit(127);
    }

    ASSERT_TRUE(master_fd > 0);
    ASSERT_TRUE(slave_fd > 0);
    ASSERT_TRUE(string(name).length() > 0);

    port1 = new Serial(string(name), 115200, Timeout::simpleTimeout(250));
  }

  virtual void TearDown() {
    port1->close();
    delete port1;
  }

  Serial * port1;
  int master_fd;
  int slave_fd;
  char name[100];
};

TEST_F(SerialTests, readWorks) {
  write(master_fd, "abc\n", 4);
  string r = port1->read(4);
  EXPECT_EQ(r, string("abc\n"));
}

TEST_F(SerialTests, writeWorks) {
  char buf[5] = "";
  port1->write("abc\n");
  read(master_fd, buf, 4);
  EXPECT_EQ(string(buf, 4), string("abc\n"));
}

TEST_F(SerialTests, timeoutWorks) {
  // Timeout a read, returns an empty string
  string empty = port1->read();
  EXPECT_EQ(empty, string(""));
  
  // Ensure that writing/reading still works after a timeout.
  write(master_fd, "abc\n", 4);
  string r = port1->read(4);
  EXPECT_EQ(r, string("abc\n"));
}

TEST_F(SerialTests, partialRead) {
  // Write some data, but request more than was written.
  write(master_fd, "abc\n", 4);

  // Should timeout, but return what was in the buffer.
  string empty = port1->read(10);
  EXPECT_EQ(empty, string("abc\n"));
  
  // Ensure that writing/reading still works after a timeout.
  write(master_fd, "abc\n", 4);
  string r = port1->read(4);
  EXPECT_EQ(r, string("abc\n"));
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
