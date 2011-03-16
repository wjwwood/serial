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
#include <string>

#include <boost/asio.hpp>
#include <boost/asio/serial_port.hpp>
#include <boost/bind.hpp>
#include <boost/thread.hpp>

#define READ_BUFFER_SIZE 4
#define SERIAL_BUFFER_SIZE 1024

class Serial {
public:
    /** Constructor, creates a SerialPortBoost object without opening a port. */
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
    * @param timeout A double that represents the time (in seconds) until a 
    *        timeout on reads occur.  Setting this to a negative number will 
    *        silently disable the timeout on reads.
    */
    Serial(std::string port, int baudrate = 9600, double timeout = -1.0);
    // Serial(string port, int baudrate = 9600, int bytesize = EIGHTBITS, int parity = PARITY_NONE, int stopbits = STOPBITS_ONE, double timeout = -1.0, bool xonxoff = false, bool rtscts = false, double writeTimeout = -1.0, bool dsrdtr = false, double interCharTimeout = -1.0);
    
    /** Destructor */
    ~Serial();
    
    /** Opens the serial port as long as the portname is set and the port isn't alreay open. */
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
    int read(char* buffer, int size = 1);
    
    /** Read size bytes from the serial port.
    * If a timeout is set it may return less characters than requested. With no timeout
    * it will block until the requested number of bytes have been read.
    * 
    * @param size An integer defining how many bytes to be read.
    * 
    * @return A std::string containing the data read.
    */
    std::string read(int size=1);
    
    /** Write length bytes from buffer to the serial port.
    * 
    * @param data A char[] with data to be written to the serial port.
    * 
    * @param length An integer representing the number of bytes to be written.
    * 
    * @return An integer representing the number of bytes written.
    */
    int write(char data[], int length);
    
    /** Write a string to the serial port.
    * 
    * @param data A std::string to be written to the serial port. (must be null terminated)
    * 
    * @return An integer representing the number of bytes written to the serial port.
    */
    int write(std::string data);
    
    /** Checks the number of bytes waiting in the buffer.
    * 
    * @return An integer representing the number of bytes in the serial buffer.
    */
    int inWaiting();
    
    /** Flushes the write buffer, blocks untill all data has been written. */
    void flush();
    
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
    bool getCTS();
    
    /** Gets the status of the DSR line.
    * 
    * @return A boolean value that represents the current logic level of the DSR line.
    */
    bool getDSR();
private:
    boost::asio::io_service io_service;
    boost::asio::serial_port *serial_port;
    
    boost::thread * io_service_thread;
    
    char read_buffer[READ_BUFFER_SIZE];
    
    
    
    std::string port;
    int baudrate;
    double timeout;
};

#endif