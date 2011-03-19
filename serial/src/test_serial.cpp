#include "ros/ros.h"

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
    ros::init(argc, argv, "serial_test_node");
    
    ros::NodeHandle n;
    
    std::string port("/dev/tty.usbserial-A900cfJA");
    // std::string port("/dev/tty.usbmodemfa141");
    
    serial = new Serial(port, 9600, 250);
    
    ros::Rate loop_rate(0.5);
    
    int count = 0;
    while (ros::ok() and count != 30) {
        // serial->write("Testing.");
        // ROS_INFO("Out of write");
        std::string result = serial->read(1);
        std::cout << ">" << result << std::endl;
        
        
        // ROS_INFO("Here.");
        
        // ROS_INFO(result.c_str());
        // ROS_INFO("%d,%s", result.length(), toHex(result).c_str());
        
        
        ros::spinOnce();
        
        // loop_rate.sleep();
        // count += 1;
    }
    
    return 0;
}
