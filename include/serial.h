/**
 * @file serial.h
 * @author  William Woodall <wjwwood@gmail.com>
 * @version 0.1
 *
 * @section LICENSE
 *
 * The MIT License
 *
 * Copyright (c) 2011 William Woodall
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 * @section DESCRIPTION
 *
 * This provides a cross platform interface for interacting with Serial Ports.
 */


#ifndef SERIAL_H
#define SERIAL_H

#include <iostream>
#include <sstream>
#include <string>

// #define BOOST_ASIO_DISABLE_KQUEUE

#include <boost/asio.hpp>
#include <boost/asio/serial_port.hpp>
#include <boost/bind.hpp>
#include <boost/thread.hpp>
#include <boost/scoped_ptr.hpp>

// A macro to disallow the copy constructor and operator= functions
// This should be used in the private: declarations for a class
#define DISALLOW_COPY_AND_ASSIGN(TypeName) \
  TypeName(const TypeName&);               \
  void operator=(const TypeName&)

// DEFINES
#ifndef DEFAULT_BAUDRATE
#define DEFAULT_BAUDRATE 9600
#endif
#ifndef DEFAULT_TIMEOUT
#define DEFAULT_TIMEOUT 0.0
#endif
#ifndef DEFAULT_BYTESIZE
#define DEFAULT_BYTESIZE EIGHTBITS
#endif
#ifndef DEFAULT_PARITY
#define DEFAULT_PARITY NONE
#endif
#ifndef DEFAULT_STOPBITS
#define DEFAULT_STOPBITS STOPBITS_ONE
#endif 
#ifndef DEFAULT_FLOWCONTROL
#define DEFAULT_FLOWCONTROL FLOWCONTROL_NONE
#endif

namespace serial {

// Serial Port settings CONSTANTS
enum bytesize_t { FIVEBITS = 5, SIXBITS = 6, SEVENBITS = 7, EIGHTBITS = 8 };
enum parity_t { NONE, ODD, EVEN };
enum stopbits_t { STOPBITS_ONE, STOPBITS_ONE_POINT_FIVE, STOPBITS_TWO };
enum flowcontrol_t { FLOWCONTROL_NONE, FLOWCONTROL_SOFTWARE, FLOWCONTROL_HARDWARE };

class Serial {
public:
    /** Constructor, Creates a Serial object but doesn't open the serial port. */
    Serial();
    
    /**
    * Constructor, creates a SerialPortBoost object and opens the port.
    * 
    * @param port A std::string containing the address of the serial port,
    *        which would be something like 'COM1' on Windows and '/dev/ttyS0'
    *        on Linux.
    * 
    * @param baudrate An integer that represents the buadrate
    * 
    * @param timeout A long that represents the time (in milliseconds) until a 
    *        timeout on reads occur.  Setting this to zero (0) will cause reading
    *        to be non-blocking, i.e. the available data will be returned immediately,
    *        but it will not block to wait for more.  Setting this to a number less than
    *        zero (-1) will result in infinite blocking behaviour, i.e. the serial port will
    *        block until either size bytes have been read or an exception has occured.
    * 
    * @param bytesize Size of each byte in the serial transmission of data, 
    *        default is EIGHTBITS, possible values are: FIVEBITS, 
    *        SIXBITS, SEVENBITS, EIGHTBITS
    * 
    * @param parity Method of parity, default is PARITY_NONE, possible values
    *        are: PARITY_NONE, PARITY_ODD, PARITY_EVEN
    * 
    * @param stopbits Number of stop bits used, default is STOPBITS_ONE, possible 
    *        values are: STOPBITS_ONE, STOPBITS_ONE_POINT_FIVE, STOPBITS_TWO
    * 
    * @param flowcontrol Type of flowcontrol used, default is FLOWCONTROL_NONE, possible
    *        values are: FLOWCONTROL_NONE, FLOWCONTROL_SOFTWARE, FLOWCONTROL_HARDWARE
    * 
    * @throw SerialPortAlreadyOpenException
    * @throw SerialPortFailedToOpenException
    */
    Serial(std::string port,
           int baudrate = DEFAULT_BAUDRATE,
           long timeout = DEFAULT_TIMEOUT,
           bytesize_t bytesize = DEFAULT_BYTESIZE,
           parity_t parity = DEFAULT_PARITY,
           stopbits_t stopbits = DEFAULT_STOPBITS,
           flowcontrol_t flowcontrol = DEFAULT_FLOWCONTROL);
    
    /** Destructor */
    ~Serial();
    
    /** 
    * Opens the serial port as long as the portname is set and the port isn't alreay open.
    * 
    * @throw SerialPortAlreadyOpenException
    * @throw SerialPortFailedToOpenException
    */
    void open();
    
    /** Closes the serial port and terminates threads. */
    void close();
    
    /** Read size bytes from the serial port.
    * If a timeout is set it may return less characters than requested. With no timeout
    * it will block until the requested number of bytes have been read.
    * 
    * @param buffer A char[] of length >= the size parameter to hold incoming data.
    * 
    * @param size An integer defining how many bytes to be read.
    * 
    * @return An integer representing the number of bytes read.
    */
    const int read(char* buffer, int size = 1);
    
    /** Read size bytes from the serial port.
    * If a timeout is set it may return less characters than requested. With no timeout
    * it will block until the requested number of bytes have been read.
    * 
    * @param size An integer defining how many bytes to be read.
    * 
    * @return A std::string containing the data read.
    */
    const std::string read(int size = 1);
    
    /** Write length bytes from buffer to the serial port.
    * 
    * @param data A char[] with data to be written to the serial port.
    * 
    * @param length An integer representing the number of bytes to be written.
    * 
    * @return An integer representing the number of bytes written.
    */
    const int write(char data[], int length);
    
    /** Write a string to the serial port.
    * 
    * @param data A std::string to be written to the serial port. (must be null terminated)
    * 
    * @return An integer representing the number of bytes written to the serial port.
    */
    const int write(std::string data);
    
    /** Sets the logic level of the RTS line.
    * 
    * @param level The logic level to set the RTS to. Defaults to true.
    */
    void setRTS(bool level = true);
    
    /** Sets the logic level of the DTR line.
    * 
    * @param level The logic level to set the DTR to. Defaults to true.
    */
    void setDTR(bool level = true);
    
    /** Gets the status of the CTS line.
    * 
    * @return A boolean value that represents the current logic level of the CTS line.
    */
    const bool getCTS() const;
    
    /** Gets the status of the DSR line.
    * 
    * @return A boolean value that represents the current logic level of the DSR line.
    */
    const bool getDSR() const;
    
    /** Sets the timeout for reads in seconds.
    * 
    * @param timeout A long that represents the time (in milliseconds) until a 
    *        timeout on reads occur.  Setting this to zero (0) will cause reading
    *        to be non-blocking, i.e. the available data will be returned immediately,
    *        but it will not block to wait for more.  Setting this to a number less than
    *        zero (-1) will result in infinite blocking behaviour, i.e. the serial port will
    *        block until either size bytes have been read or an exception has occured.
    */
    void setTimeoutMilliseconds(long timeout);
    
    /** Gets the timeout for reads in seconds.
    * 
    * @param timeout A long that represents the time (in milliseconds) until a 
    *        timeout on reads occur.  Setting this to zero (0) will cause reading
    *        to be non-blocking, i.e. the available data will be returned immediately,
    *        but it will not block to wait for more.  Setting this to a number less than
    *        zero (-1) will result in infinite blocking behaviour, i.e. the serial port will
    *        block until either size bytes have been read or an exception has occured.
    */
    const long getTimeoutMilliseconds() const;
    
    /** Sets the baudrate for the serial port.
    * 
    * @param baudrate An integer that sets the baud rate for the serial port.
    */
    void setBaudrate(int baudrate);
    
    /** Gets the baudrate for the serial port.
    * 
    * @return An integer that sets the baud rate for the serial port.
    */
    const int getBaudrate() const;
    
    /** Sets the bytesize for the serial port.
    * 
    * @param bytesize Size of each byte in the serial transmission of data, 
    *        default is EIGHTBITS, possible values are: FIVEBITS, 
    *        SIXBITS, SEVENBITS, EIGHTBITS
    * 
    * @throw InvalidBytesizeException
    */
    void setBytesize(bytesize_t bytesize);
    
    /** Gets the bytesize for the serial port.
    * 
    * @return Size of each byte in the serial transmission of data, 
    *         default is EIGHTBITS, possible values are: FIVEBITS, 
    *         SIXBITS, SEVENBITS, EIGHTBITS
    * 
    * @throw InvalidBytesizeException
    */
    const bytesize_t getBytesize() const;
    
    /** Sets the parity for the serial port.
    * 
    * @param parity Method of parity, default is PARITY_NONE, possible values
    *        are: PARITY_NONE, PARITY_ODD, PARITY_EVEN
    * 
    * @throw InvalidParityException
    */
    void setParity(parity_t parity);
    
    /** Gets the parity for the serial port.
    * 
    * @return Method of parity, default is PARITY_NONE, possible values
    *         are: PARITY_NONE, PARITY_ODD, PARITY_EVEN
    * 
    * @throw InvalidParityException
    */
    const parity_t getParity() const;
    
    /** Sets the stopbits for the serial port.
    * 
    * @param stopbits Number of stop bits used, default is STOPBITS_ONE, possible 
    *        values are: STOPBITS_ONE, STOPBITS_ONE_POINT_FIVE, STOPBITS_TWO
    * 
    * @throw InvalidStopbitsException
    */
    void setStopbits(stopbits_t stopbits);
    
    /** Gets the stopbits for the serial port.
    * 
    * @return Number of stop bits used, default is STOPBITS_ONE, possible 
    *         values are: STOPBITS_ONE, STOPBITS_ONE_POINT_FIVE, STOPBITS_TWO
    * 
    * @throw InvalidStopbitsException
    */
    const stopbits_t getStopbits() const;
    
    /** Sets the flow control for the serial port.
    * 
    * @param flowcontrol Type of flowcontrol used, default is FLOWCONTROL_NONE, possible
    *        values are: FLOWCONTROL_NONE, FLOWCONTROL_SOFTWARE, FLOWCONTROL_HARDWARE
    * 
    * @throw InvalidFlowcontrolException
    */
    void setFlowcontrol(flowcontrol_t flowcontrol);
    
    /** Gets the flow control for the serial port.
    * 
    * @return Type of flowcontrol used, default is FLOWCONTROL_NONE, possible
    *         values are: FLOWCONTROL_NONE, FLOWCONTROL_SOFTWARE, FLOWCONTROL_HARDWARE
    * 
    * @throw InvalidFlowcontrolException
    */
    const flowcontrol_t getFlowcontrol() const;
private:
    DISALLOW_COPY_AND_ASSIGN(Serial);
    void init();
    void read_complete(const boost::system::error_code& error, std::size_t bytes_transferred);
    void timeout_callback(const boost::system::error_code& error);
    
    boost::asio::io_service io_service;
    
    boost::asio::io_service::work work;
    
    boost::scoped_ptr<boost::asio::serial_port> serial_port;
    
    boost::asio::deadline_timer timeout_timer;
    
    std::string port;
    boost::asio::serial_port_base::baud_rate baudrate;
    boost::posix_time::time_duration timeout;
    boost::asio::serial_port_base::character_size bytesize;
    boost::asio::serial_port_base::parity parity;
    boost::asio::serial_port_base::stop_bits stopbits;
    boost::asio::serial_port_base::flow_control flowcontrol;
    
    int bytes_read;
    int bytes_to_read;
    bool reading;
    bool nonblocking;
};

class SerialPortAlreadyOpenException : public std::exception {
    const char * port;
public:
    SerialPortAlreadyOpenException(const char * port) {this->port = port;}
    
    virtual const char* what() const throw() {
        std::stringstream ss;
        ss << "Serial Port already open: " << this->port;
        return ss.str().c_str();
    }
};

class SerialPortFailedToOpenException : public std::exception {
    const char * e_what;
public:
    SerialPortFailedToOpenException(const char * e_what) {this->e_what = e_what;}
    
    virtual const char* what() const throw() {
        std::stringstream ss;
        ss << "Serial Port failed to open: " << this->e_what;
        return ss.str().c_str();
    }
};

class InvalidBytesizeException : public std::exception {
    int bytesize;
public:
    InvalidBytesizeException(int bytesize) {this->bytesize = bytesize;}
    
    virtual const char* what() const throw() {
        std::stringstream ss;
        ss << "Invalid bytesize provided: " << this->bytesize;
        return ss.str().c_str();
    }
};

class InvalidParityException : public std::exception {
    int parity;
public:
    InvalidParityException(int parity) {this->parity = parity;}
    
    virtual const char* what() const throw() {
        std::stringstream ss;
        ss << "Invalid parity provided: " << this->parity;
        return ss.str().c_str();
    }
};

class InvalidStopbitsException : public std::exception {
    int stopbits;
public:
    InvalidStopbitsException(int stopbits) {this->stopbits = stopbits;}
    
    virtual const char* what() const throw() {
        std::stringstream ss;
        ss << "Invalid stopbits provided: " << this->stopbits;
        return ss.str().c_str();
    }
};

class InvalidFlowcontrolException : public std::exception {
    int flowcontrol;
public:
    InvalidFlowcontrolException(int flowcontrol) {this->flowcontrol = flowcontrol;}
    
    virtual const char* what() const throw() {
        std::stringstream ss;
        ss << "Invalid flowcontrol provided: " << this->flowcontrol;
        return ss.str().c_str();
    }
};

} // namespace serial

#endif