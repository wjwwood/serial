#include "serial.h"

Serial::Serial() {
    ;
}

Serial::Serial(std::string port, int baudrate, double timeout) {
    this->port = port;
    this->baudrate = baudrate;
    this->timeout = timeout;
    this->open();
}

Serial::~Serial() {
    ;
}

void Serial::open() {
    this->serial_port = new boost::asio::serial_port(this->io_service, this->port);
    
    if(not serial_port->is_open()) {
        std::cerr << "[AX2550] Failed to open serial port: " << this->port << std::endl;
        return;
    }
    
    try {
        this->serial_port->set_option(boost::asio::serial_port_base::baud_rate(this->baudrate));
        // this->serial_port->set_option(
        //     boost::asio::serial_port_base::flow_control(
        //         boost::asio::serial_port_base::flow_control::none));
        // this->serial_port->set_option(
        //     boost::asio::serial_port_base::parity(
        //         boost::asio::serial_port_base::parity::even));
        // this->serial_port->set_option(
        //     boost::asio::serial_port_base::stop_bits(
        //         boost::asio::serial_port_base::stop_bits::one));
        // this->serial_port->set_option(boost::asio::serial_port_base::character_size(7));
    } catch(std::exception &e) {
        std::cerr << "[AX2550] Error opening serial port: " << e.what() << std::endl;
        try {
            if(this->serial_port->is_open())
                this->serial_port->close();
        } catch(std::exception &e) {
            std::cerr << "[AX2550] Error closing serial port: " << e.what() << std::endl;
        }
        this->serial_port = NULL;
    }
}

void Serial::close() {
    ;
}

int Serial::read(char* buffer, int size) {
    return this->serial_port->read_some(boost::asio::buffer(buffer, size));
}

std::string Serial::read(int size) {
    char serial_buffer[size];
    int number_read = this->serial_port->read_some(boost::asio::buffer(serial_buffer, size));
    serial_buffer[number_read] = NULL;
    return std::string(serial_buffer);
}

int Serial::write(char data[], int length) {
    return -1;
}

int Serial::write(std::string data) {
    return -1;
}

int Serial::inWaiting() {
    return -1;
}

void Serial::flush() {
    ;
}

void Serial::setRTS(bool level) {
    ;
}

void Serial::setDTR(bool level) {
    ;
}

bool Serial::getCTS() {
    return false;
}

bool Serial::getDSR() {
    return false;
}