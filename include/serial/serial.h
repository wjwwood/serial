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

#include <limits>
#include <vector>
#include <string.h>
#include <sstream>
#include <exception>
#include <stdexcept>

namespace serial {

/*!
 * Enumeration defines the possible bytesizes for the serial port.
 */
typedef enum {
  fivebits = 5,
  sixbits = 6,
  sevenbits = 7,
  eightbits = 8
} bytesize_t;

/*!
 * Enumeration defines the possible parity types for the serial port.
 */
typedef enum {
  parity_none = 0,
  parity_odd = 1,
  parity_even = 2
} parity_t;

/*!
 * Enumeration defines the possible stopbit types for the serial port.
 */
typedef enum {
  stopbits_one = 1,
  stopbits_one_point_five,
  stopbits_two = 2
} stopbits_t;

/*!
 * Enumeration defines the possible flowcontrol types for the serial port.
 */
typedef enum {
  flowcontrol_none = 0,
  flowcontrol_software,
  å
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
  * default is eightbits, possible values are: fivebits, sixbits, sevenbits,
  * eightbits
  *
  * \param parity Method of parity, default is parity_none, possible values
  * are: parity_none, parity_odd, parity_even
  *
  * \param stopbits Number of stop bits used, default is stopbits_one,
  * possible values are: stopbits_one, stopbits_one_point_five, stopbits_two
  *
  * \param flowcontrol Type of flowcontrol used, default is
  * flowcontrol_none, possible values are: flowcontrol_none,
  * flowcontrol_software, flowcontrol_hardware
  *
  * \param buffer_size The maximum size of the internal buffer, defaults
  * to 256 bytes (2^8).
  *
  * \throw PortNotOpenedException
  */
  Serial (const std::string &port = "",
          unsigned long baudrate = 9600,
          long timeout = 0,
          bytesize_t bytesize = eightbits,
          parity_t parity = parity_none,
          stopbits_t stopbits = stopbits_one,
          flowcontrol_t flowcontrol = flowcontrol_none);

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
  * \throw std::invalid_argument
  * \throw serial::SerialExecption
  * \throw serial::IOException
  */
  void
  open ();

  /*! Gets the open status of the serial port.
  *
  * \return Returns true if the port is open, false otherwise.
  */
  bool
  isOpen () const;

  /*! Closes the serial port. */
  void
  close ();

  /*! Return the number of characters in the buffer. */
  size_t
  available();

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

  /*! Reads in a line or until a given delimiter has been processed
  *
  * Reads from the serial port until a single line has been read.
  *
  * \param size A maximum length of a line defaults to size_t::max()
  * \param eol A string to match against for the EOL.
  *
  * \return A std::string containing the line.
  */
  std::string
  readline(size_t size = std::numeric_limits<std::size_t>::max(),
           std::string eol  = "\n");

  /*! Reads in multiple lines until the serail port times out.
  *
  * This requires a timeout > 0 before it can be run. It will read until a
  * timeout occurs and return a list of strings.
  *
  * \param eol A string to match against for the EOL.
  *
  * \return A vector<string> containing the lines.
  */
  std::vector<std::string>
  readlines(std::string eol = "\n");

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
  setBaudrate (unsigned long baudrate);

  /*! Gets the baudrate for the serial port.
  *
  * \return An integer that sets the baud rate for the serial port.
  *
  * \see Serial::setBaudrate
  *
  * \throw InvalidConfigurationException
  */
  unsigned long
  getBaudrate () const;

  /*! Sets the bytesize for the serial port.
  *
  * \param bytesize Size of each byte in the serial transmission of data,
  * default is eightbits, possible values are: fivebits, sixbits, sevenbits,
  * eightbits
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
  * \param parity Method of parity, default is parity_none, possible values
  * are: parity_none, parity_odd, parity_even
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
  * \param stopbits Number of stop bits used, default is stopbits_one,
  * possible values are: stopbits_one, stopbits_one_point_five, stopbits_two
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
  * \param flowcontrol Type of flowcontrol used, default is flowcontrol_none,
  * possible values are: flowcontrol_none, flowcontrol_software,
  * flowcontrol_hardware
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

  /*! Flush the input and output buffers */
  void
  flush ();

  /*! Flush only the input buffer */
  void
  flushInput ();

  /*! Flush only the output buffer */
  void
  flushOutput ();

  void
  sendBreak (int duration);

  void
  setBreak (bool level = true);

  void
  setRTS (bool level = true);

  void
  setDTR (bool level = true);

  bool
  getCTS ();

  bool
  getDSR ();

  bool
  getRI ();

  bool
  getCD ();

private:
  // Disable copy constructors
  Serial(const Serial&);
  void operator=(const Serial&);
  const Serial& operator=(Serial);

  std::string read_cache_; //!< Cache for doing reads in chunks.

  // Pimpl idiom, d_pointer
  class SerialImpl;
  SerialImpl *pimpl_;

  // Scoped Lock Classes
  class ScopedReadLock;
  class ScopedWriteLock;

};

class SerialExecption : public std::exception
{
  const char* e_what_;
public:
  SerialExecption (const char *description) : e_what_ (description) {}

  virtual const char* what () const throw ()
  {
    std::stringstream ss;
    ss << "SerialException " << e_what_ << " failed.";
    return ss.str ().c_str ();
  }
};

class IOException : public std::exception
{
  const char* e_what_;
  int errno_;
public:
  explicit IOException (int errnum)
  : e_what_ (strerror (errnum)), errno_(errnum) {}
  explicit IOException (const char * description)
  : e_what_ (description), errno_(0) {}

  int getErrorNumber () { return errno_; }

  virtual const char* what () const throw ()
  {
    std::stringstream ss;
    if (errno_ == 0)
      ss << "IO Exception " << e_what_ << " failed.";
    else
      ss << "IO Exception (" << errno_ << "): " << e_what_ << " failed.";
    return ss.str ().c_str ();
  }
};

class PortNotOpenedException : public std::exception
{
  const char * e_what_;
public:
  PortNotOpenedException (const char * description) : e_what_ (description) {}

  virtual const char* what () const throw ()
  {
    std::stringstream ss;
    ss << e_what_ << " called before port was opened.";
    return ss.str ().c_str ();
  }
};

} // namespace serial

#endif
