#include <iostream>

#include <serial/serial.h>
#include <serial/serial_listener.h>

using namespace serial;

void default_handler(std::string line) {
  std::cout << "default_handler got a: " << line << std::endl;
}

void callback(std::string line) {
  std::cout << "callback got a: " << line << std::endl;
}

#if 1
int main(void) {
  Serial serial("/dev/tty.usbmodemfd1231", 115200);

  SerialListener listener;
  // Set the time to live for messages to 10 milliseconds
  listener.setTimeToLive(10);
  listener.startListening(serial);

  listener.listenFor(SerialListener::startsWith("V="), callback);

  serial.write("?$1E\r");
  if (!listener.listenForStringOnce("?$1E")) {
    std::cerr << "Didn't get conformation of device version!" << std::endl;
    return 1;
  }

  serial.write("?V\r");
  serial.write("# 1\r");

  while (true) {
    // Sleep 100 ms
    SerialListener::sleep(100);
  }

}
#endif

#if 0
int main(void) {
  Serial serial("/dev/tty.usbmodemfd1231", 115200);

  serial.write("?$1E\r");
  SerialListener::sleep(10);
  // if ("?$1E\r" != serial.read(5)) {
  //   std::cerr << "Didn't get conformation of device version!" << std::endl;
  //   return 1;
  // }

  serial.write("?V\r");
  serial.write("# 1\r");

  while (true) {
    std::cout << serial.read(5) << std::endl;
  }

}
#endif
