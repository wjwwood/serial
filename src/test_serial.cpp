#include <string>
#include <iostream>

#include "serial.h"

Serial *serial;

std::string toHex(std::string input) {
    std::stringstream ss;
    for(unsigned int i = 0; i != input.length(); i++) {
        char temp[4];
        sprintf(temp, "%.2X", input[i]);
        ss << " ";
        if(input[i] == 0x0A)
            ss << "LF";
        else if(input[i] == 0x0D)
            ss << "NL";
        else
            ss << input[i];
        ss << " " << std::hex << temp;
    }
    return ss.str();
};

int main(int argc, char **argv)
{
    std::string port("/dev/tty.usbserial-A900cfJA");
    // std::string port("/dev/tty.usbmodemfa141");
    
    serial = new Serial(port, 115200, 250);
    
    int count = 0;
    while (count >= 0) {
        int bytes_wrote = serial->write("Testing.");
        std::string result = serial->read(8);
        if(count % 10 == 0)
            std::cout << ">" << count << ">" << bytes_wrote << ">" << result << std::endl;
        
        count += 1;
    }
    
    return 0;
}
