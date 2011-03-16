#include "ros/ros.h"

#include <string>
#include <iostream>

#include "serial.h"

Serial *serial;

std::string toHex(std::string input) {
    std::stringstream ss;
    for(int i = 0; i != input.length(); i++) {
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
    
    serial = new Serial(port);
    
    ros::Rate loop_rate(10);
    
    while (ros::ok()) {
        std::string result = serial->read(1);
        
        ROS_INFO("%d,%s", result.length(), toHex(result).c_str());
        
        ros::spinOnce();
        
        loop_rate.sleep();
    }
    
    return 0;
}