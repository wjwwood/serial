/*!
 * \file serial/impl/unix.h
 * \author  William Woodall <wjwwood@gmail.com>
 * \author  John Harrison <ash@greaterthaninfinity.com>
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
 * This provides a unix based pimpl for the Serial class.
 */

#ifndef SERIAL_IMPL_UNIX_H
#define SERIAL_IMPL_UNIX_H

#include "serial/serial.h"

namespace {

using std::string;

class serial::Serial::SerialImpl {
public:
  SerialImpl (const string &port,
             int baudrate,
             long timeout,
             bytesize_t bytesize,
             parity_t parity,
             stopbits_t stopbits,
             flowcontrol_t flowcontrol);

  virtual ~SerialImpl ();

  void open ();
  void close ();
  bool isOpen ();

	size_t available ();
  string read (size_t size = 1);
  size_t write (const string &data);
	
	void flush ();
	void flushInput ();
	void flushOutput ();

	void sendBreak();
	void setBreak();
	void setRTS();
	void setDTR();
	void getCTS();
	void getDSR();
	void getRI();
	void getCD();

  void setPort (const string &port);
  string getPort () const;

  void setTimeout (long timeout);
  long getTimeout () const;

  void setBaudrate (int baudrate);
  int getBaudrate () const;

  void setBytesize (bytesize_t bytesize);
  bytesize_t getBytesize () const;

  void setParity (parity_t parity);
  parity_t getParity () const;

  void setStopbits (stopbits_t stopbits);
  stopbits_t getStopbits () const;

  void setFlowcontrol (flowcontrol_t flowcontrol);
  flowcontrol_t getFlowcontrol () const;

protected:
	void reconfigurePort ();

private:
  int fd_; // The current file descriptor.

	bool isOpen_;
	
	int interCharTimeout_;
	int xonxoff_;
	int rtscts_;

	string port_;               // Path to the file descriptor
	int baudrate_;              // Baudrate
	long timeout_;              // Timeout for read operations
	bytesize_t bytesize_;       // Size of the bytes
	parity_t parity_;           // Parity
	stopbits_t stopbits_;       // Stop Bits
	flowcontrol_t flowcontrol_; // Flow Control
};

}

#endif // SERIAL_IMPL_UNIX_H