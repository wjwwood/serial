/*!
 * \file serial/serial.h
 * \author  William Woodall <wjwwood@gmail.com>
 * \author  John Harrison   <ash.gti@gmail.com>
 * \version 0.1
 *
 * \section LICENSE
 *
 * The MIT License
 *
 * Copyright (c) 2011 William Woodall
 *
 * Permission is hereby granted, free of charge, to any person obtaining a 
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING 
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER 
 * DEALINGS IN THE SOFTWARE.
 *
 * \section DESCRIPTION
 *
 * This provides a cross platform interface for interacting with Serial Ports.
 */

#ifndef SERIAL_H
#define SERIAL_H

#include <string>
#include <sstream>
#include <vector>
#include <limits>

namespace serial {

/*!
 * Enumeration defines the possible bytesizes for the serial port.
 */
typedef enum {
  FIVEBITS = 5,
  SIXBITS = 6,
  SEVENBITS = 7,
  EIGHTBITS = 8
} bytesize_t;

/*!
 * Enumeration defines the possible parity types for the serial port.
 */
typedef enum {
  PARITY_NONE = 0,
  PARITY_ODD = 1,
  PARITY_EVEN = 2
} parity_t;

/*!
 * Enumeration defines the possible stopbit types for the serial port.
 */
typedef enum {
  STOPBITS_ONE = 1,
  STOPBITS_ONE_POINT_FIVE,
  STOPBITS_TWO = 2
} stopbits_t;

/*!
 * Enumeration defines the possible flowcontrol types for the serial port.
 */
typedef enum {
  FLOWCONTROL_NONE = 0,
  FLOWCONTROL_SOFTWARE,
  FLOWCONTROL_HARDWARE
} flowcontrol_t;

/*!
 * Class that provides a portable serial port interface.
 */
class Serial {
public:
  /*!
  * Constructor, creates a SerialPortBoost object and opens the port.
  * 
  * \param port A std::string containing the address of the serial port,
  *        which would be something like 'COM1' on Windows and '/dev/ttyS0'
  *        on Linux.
  * 
  * \param baudrate An integer that represents the buadrate
  * 
  * \param timeout A long that represents the time (in milliseconds) until a 
  * timeout on reads occur. Setting this to zero (0) will cause reading to
  * be non-blocking, i.e. the available data will be returned immediately,
  * but it will not block to wait for more. Setting this to a number less
  * than zero (-1) will result in infinite blocking behaviour, i.e. the
  * serial port will block until either size bytes have been read or an
  * exception has occured.
  * 
  * \param bytesize Size of each byte in the serial transmission of data, 
  * default is EIGHTBITS, possible values are: FIVEBITS, SIXBITS, SEVENBITS, 
  * EIGHTBITS
  * 
  * \param parity Method of parity, default is PARITY_NONE, possible values
  * are: PARITY_NONE, PARITY_ODD, PARITY_EVEN
  * 
  * \param stopbits Number of stop bits used, default is STOPBITS_ONE, 
  * possible values are: STOPBITS_ONE, STOPBITS_ONE_POINT_FIVE, STOPBITS_TWO
  * 
  * \param flowcontrol Type of flowcontrol used, default is  
  * FLOWCONTROL_NONE, possible values are: FLOWCONTROL_NONE, 
  * FLOWCONTROL_SOFTWARE, FLOWCONTROL_HARDWARE
  * 
  * \throw PortNotOpenedException
  */
  Serial (const std::string &port = "",
          int baudrate = 9600,
          long timeout = 0,
          bytesize_t bytesize = EIGHTBITS,
          parity_t parity = PARITY_NONE,
          stopbits_t stopbits = STOPBITS_ONE,
          flowcontrol_t flowcontrol = FLOWCONTROL_NONE);

  /*! Destructor */
  virtual ~Serial ();

  /*!
  * Opens the serial port as long as the portname is set and the port isn't
  * alreay open.
  * 
  * If the port is provided to the constructor then an explicit call to open 
  * is not needed.
  * 
  * \see Serial::Serial
  * 
  * \throw PortNotOpenedException
  */
  void
  open ();

  /*! Gets the open status of the serial port.
  * 
  * \return Returns true if the port is open, false otherwise.
  */
  bool
  isOpen ();

  /*! Closes the serial port. */
  void
  close ();

  /* Return the number of characters in the buffer.
  */
  size_t
  available();

  /*! Read a given amount of bytes from the serial port.
  * 
  * If a timeout is set it may return less characters than requested. With
  * no timeout it will block until the requested number of bytes have been
  * read or until an exception occurs.
  * 
  * \param buffer An unsigned char array large enough to hold incoming data 
  * up to the requested size.
  * 
  * \param size A size_t defining how many bytes to be read.
  * 
  * \return A size_t representing the number of bytes actually read.
  */
  //size_t
  //read (unsigned char* buffer, size_t size = 1);

  /*! Read a given amount of bytes from the serial port.
  * 
  * If a timeout is set it may return less characters than requested. With
  * no timeout it will block until the requested number of bytes have been
  * read or until an exception occurs.
  * 
  * \param size A size_t defining how many bytes to be read.
  * 
  * \return A std::string containing the data read.
  */
  std::string
  read (size_t size = 1);

  std::string readline(size_t size = std::numeric_limits<std::size_t>::max(),
                       std::string eol = "\n");

  std::vector<std::string> readlines(std::string eol = "\n");

  /*! Read a given amount of bytes from the serial port.
  * 
  * Reads into a std::string by reference rather than returning it.
  * 
  * \param buffer A std::string reference for reading to.
  * \param size A size_t defining how many bytes to be read.
  * 
  * \return A size_t that represents how many bytes were read.
  * 
  * \see Serial::read(size_t)
  */
  //size_t
  //read (std::string &buffer, size_t size = 1);

  /*! Write bytes from the data to the serial port by given length.
  * 
  * \param data An unsigned char array containing data to be written to the
  * serial port.
  * 
  * \param length A size_t representing the number of bytes to be written.
  * 
  * \return A size_t representing the number of bytes actually written.
  */
  //size_t
  //write (unsigned char* data, size_t length);

  /*! Write a string to the serial port.
  * 
  * \param data A const std::string reference containg the data to be written 
  * to the serial port.
  * 
  * \return A size_t representing the number of bytes actually written to
  * the serial port.
  */
  size_t
  write (const std::string &data);

  /*! Sets the serial port identifier.
  * 
  * \param port A const std::string reference containing the address of the 
  * serial port, which would be something like 'COM1' on Windows and 
  * '/dev/ttyS0' on Linux.
  * 
  * \throw InvalidConfigurationException
  */
  void
  setPort (const std::string &port);

  /*! Gets the serial port identifier.
  * 
  * \see Serial::setPort
  * 
  * \throw InvalidConfigurationException
  */
  std::string
  getPort () const;

  /*! Sets the timeout for reads in milliseconds.
  * 
  * \param timeout A long that represents the time (in milliseconds) until a
  * timeout on reads occur.  Setting this to zero (0) will cause reading to be
  * non-blocking, i.e. the available data will be returned immediately, but it
  * will not block to wait for more.  Setting this to a number less than
  * zero (-1) will result in infinite blocking behaviour, i.e. the serial port 
  * will block until either size bytes have been read or an exception has 
  * occured.
  */
  void
  setTimeout (long timeout);

  /*! Gets the timeout for reads in seconds.
  * 
  * \see Serial::setTimeout
  */
  long
  getTimeout () const;

  /*! Sets the baudrate for the serial port.
  * 
  * Possible baudrates depends on the system but some safe baudrates include:
  * 110, 300, 600, 1200, 2400, 4800, 9600, 14400, 19200, 28800, 38400, 56000,
  * 57600, 115200
  * Some other baudrates that are supported by some comports:
  * 128000, 153600, 230400, 256000, 460800, 921600
  * 
  * \param baudrate An integer that sets the baud rate for the serial port.
  * 
  * \throw InvalidConfigurationException
  */
  void
  setBaudrate (int baudrate);

  /*! Gets the baudrate for the serial port.
  * 
  * \return An integer that sets the baud rate for the serial port.
  * 
  * \see Serial::setBaudrate
  * 
  * \throw InvalidConfigurationException
  */
  int
  getBaudrate () const;

  /*! Sets the bytesize for the serial port.
  * 
  * \param bytesize Size of each byte in the serial transmission of data, 
  * default is EIGHTBITS, possible values are: FIVEBITS, SIXBITS, SEVENBITS, 
  * EIGHTBITS
  * 
  * \throw InvalidConfigurationException
  */
  void
  setBytesize (bytesize_t bytesize);

  /*! Gets the bytesize for the serial port.
  * 
  * \see Serial::setBytesize
  * 
  * \throw InvalidConfigurationException
  */
  bytesize_t
  getBytesize () const;

  /*! Sets the parity for the serial port.
  * 
  * \param parity Method of parity, default is PARITY_NONE, possible values 
  * are: PARITY_NONE, PARITY_ODD, PARITY_EVEN
  * 
  * \throw InvalidConfigurationException
  */
  void
  setParity (parity_t parity);

  /*! Gets the parity for the serial port.
  * 
  * \see Serial::setParity
  * 
  * \throw InvalidConfigurationException
  */
  parity_t
  getParity () const;

  /*! Sets the stopbits for the serial port.
  * 
  * \param stopbits Number of stop bits used, default is STOPBITS_ONE, 
  * possible values are: STOPBITS_ONE, STOPBITS_ONE_POINT_FIVE, STOPBITS_TWO
  * 
  * \throw InvalidConfigurationException
  */
  void
  setStopbits (stopbits_t stopbits);

  /*! Gets the stopbits for the serial port.
  * 
  * \see Serial::setStopbits
  * 
  * \throw InvalidConfigurationException
  */
  stopbits_t
  getStopbits () const;

  /*! Sets the flow control for the serial port.
  * 
  * \param flowcontrol Type of flowcontrol used, default is FLOWCONTROL_NONE, 
  * possible values are: FLOWCONTROL_NONE, FLOWCONTROL_SOFTWARE, 
  * FLOWCONTROL_HARDWARE
  * 
  * \throw InvalidConfigurationException
  */
  void
  setFlowcontrol (flowcontrol_t flowcontrol);

  /*! Gets the flow control for the serial port.
  * 
  * \see Serial::setFlowcontrol
  * 
  * \throw InvalidConfigurationException
  */
  flowcontrol_t
  getFlowcontrol () const;

private:
  // Disable copy constructors
  Serial(const Serial&);
  void operator=(const Serial&);
  const Serial& operator=(Serial);

  // Pimpl idiom, d_pointer
  class SerialImpl;
  SerialImpl *pimpl;
};

class IOException : public std::exception {
    const char * e_what;
public:
    IOException(const char * e_what) {this->e_what = e_what;}
    
    virtual const char* what() const throw() {
        std::stringstream ss;
        ss << "Serial Port failed to open: " << this->e_what;
        return ss.str().c_str();
    }
};

class PortNotOpenedException : public std::exception {
    const char * e_what;
public:
    PortNotOpenedException(const char * e_what) {this->e_what = e_what;}
    
    virtual const char* what() const throw() {
        std::stringstream ss;
        ss << "Serial Port failed to open: " << this->e_what;
        return ss.str().c_str();
    }
};

class InvalidConfigurationException : public std::exception {
    int bytesize;
public:
    InvalidConfigurationException(int bytesize) {this->bytesize = bytesize;}
    
    virtual const char* what() const throw() {
        std::stringstream ss;
        ss << "Invalid configuration provided: " << this->bytesize;
        return ss.str().c_str();
    }
};

} // namespace serial

#endif