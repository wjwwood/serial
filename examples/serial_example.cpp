#include <string>
#include <iostream>

#include "serial/serial.h"

int main(int argc, char **argv)
{
    if(argc < 2) {
        std::cerr << "Usage: test_serial <serial port address>" << std::endl;
        return 0;
    }
    std::string port(argv[1]);
    
    serial::Serial serial(port, 115200, 250);
    
    std::cout << "Is the serial port open?";
    if(serial.isOpen())
        std::cout << " Yes." << std::endl;
    else
        std::cout << " No." << std::endl;
    
    int count = 0;
    while (count >= 0) {
        int bytes_wrote = serial.write("Testing.");
        std::string result = serial.read(8);
        if(count % 10 == 0)
            std::cout << ">" << count << ">" << bytes_wrote << ">" << result << std::endl;
        
        count += 1;
    }
    
    return 0;
}
