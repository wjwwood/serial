#include <string>
#include <iostream>

#include <boost/thread.hpp>

#include "serial/serial.h"

int run(int argc, char **argv)
{
    if(argc < 2) {
        std::cerr << "Usage: test_serial <serial port address>" << std::endl;
        return 0;
    }
    std::string port(argv[1]);
    
    // port, baudrate, timeout in milliseconds
    serial::Serial serial(port, 115200, 250);
    
    std::cout << "Is the serial port open?";
    if(serial.isOpen())
        std::cout << " Yes." << std::endl;
    else
        std::cout << " No." << std::endl;
    
    int count = 0;
    while (count >= 0) {
        size_t bytes_wrote = serial.write("Testing.");
        std::string result = serial.read(8);
        std::cout << ">" << count << ">" << bytes_wrote << ">";
        std::cout << result.length() << "<" << result << std::endl;
        
        count += 1;
        boost::this_thread::sleep(boost::posix_time::milliseconds(100));
    }
    
    return 0;
}

int main(int argc, char **argv) {
  try {
    return run(argc, argv);
  } catch (std::exception &e) {
    std::cerr << "Unhandled Exception: " << e.what() << std::endl;
  }
}
