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

bool comparator(std::string line) {
  if (line.substr(0,2) == "V=")
    return true;
  return false;
}

int main(void) {
  Serial serial("/dev/tty.usbmodemfd1231", 115200);

  SerialListener listener;
  // Set the time to live for messages to 1 second
  listener.setTimeToLive(1000);
  listener.startListening(&serial);

  listener.listenFor(comparator, callback);

  serial.write("?$1E\r");
  if (!listener.listenForStringOnce("?$1E")) {
    std::cerr << "Didn't get conformation of device version!" << std::endl;
    return 1;
  }

}

/*
TODO:

listenForOnce -> listenForStringOnce
listenForOnce(ComparatorType comparator, std::string& result, size_t timeout)

*/