#include "gtest/gtest.h"

#include <boost/bind.hpp>

// OMG this is so nasty...
// #define private public
// #define protected public
#include <string>
#include <iostream>

#include "serial/serial.h"

using std::string;
using std::cout;
using std::endl;
using serial::Serial;

int main(int argc, char **argv) {
  Serial s("/dev/tty.usbserial-A900adHq", 9600, 2000);
  s.flush();
  int count = 0;
  while (count < 10) {
    size_t available = s.available();
    cout << "avialable: " << available << endl;
    string line = s.readline();
    cout << count << ": " << line;
    count++;
  }
}
