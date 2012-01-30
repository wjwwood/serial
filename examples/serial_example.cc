#include <string>
#include <iostream>
#include <stdio.h>

#include <boost/thread.hpp>

#include "serial/serial.h"

int run(int argc, char **argv)
{
    if(argc < 3) {
        std::cerr << "Usage: test_serial <serial port address> <baudrate>" << std::endl;
        return 0;
    }
    std::string port(argv[1]);
    unsigned long baud = 0;

    sscanf(argv[2], "%lu", &baud);

    // port, baudrate, timeout in milliseconds
    serial::Serial serial(port, baud, 30000);
    
    std::cout << "Is the serial port open?";
    if(serial.isOpen())
        std::cout << " Yes." << std::endl;
    else
        std::cout << " No." << std::endl;
    
    int count = 0;
    while (count >= 0) {
        size_t bytes_wrote = serial.write("Testing.\n");
        std::string result = serial.readline();
        std::cout << ">" << count << ">" << bytes_wrote << ">";
        std::cout << result.length() << "<" << result << std::endl;
        
        count += 1;
        boost::this_thread::sleep(boost::posix_time::milliseconds(100));
    }
    
    return 0;
}

int main(int argc, char **argv) {
  // try {
    return run(argc, argv);
  // } catch (std::exception &e) {
  //   std::cerr << "Unhandled Exception: " << e.what() << std::endl;
  // }
}
