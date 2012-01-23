/* Copyright 2012 William Woodall and John Harrison */

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <paths.h>
#include <sysexits.h>
#include <termios.h>
#include <sys/param.h>
#include <sys/select.h>
#include <sys/time.h>
#include <time.h>

#include "serial/impl/unix.h"

#ifndef TIOCINQ
#ifdef FIONREAD
#define TIOCINQ FIONREAD
#else
#define TIOCINQ 0x541B
#endif
#endif

using std::string;
using std::invalid_argument;
using serial::Serial;
using serial::SerialExecption;
using serial::PortNotOpenedException;
using serial::IOException;

Serial::SerialImpl::SerialImpl (const string &port, unsigned long baudrate,
                                long timeout, bytesize_t bytesize,
                                parity_t parity, stopbits_t stopbits,
                                flowcontrol_t flowcontrol)
: port_ (port), fd_ (-1), isOpen_ (false), xonxoff_ (true), rtscts_ (false),
  timeout_ (timeout), baudrate_ (baudrate), parity_ (parity), bytesize_ (bytesize),
  stopbits_ (stopbits), flowcontrol_ (flowcontrol)
{
  if (port_.empty () == false)
    open ();
}

Serial::SerialImpl::~SerialImpl ()
{
  close();
}

void
Serial::SerialImpl::open ()
{
  if (port_.empty())
  {
    throw invalid_argument ("bad port specified");
  }
  if (isOpen_ == true)
  {
    throw SerialExecption ("port already open");
  }

  fd_ = ::open (port_.c_str(), O_RDWR | O_NOCTTY | O_NONBLOCK);

  if (fd_ == -1)
  {
    switch (errno)
    {
      case EINTR:
        // Recurse because this is a recoverable error.
        open ();
        return;
      case ENFILE:
      case EMFILE:
        throw IOException ("to many file handles open");
        break;
      default:
        throw IOException (errno);
    }
  }

  reconfigurePort();
  isOpen_ = true;
}

void
Serial::SerialImpl::reconfigurePort ()
{
  if (fd_ == -1)
  {
    // Can only operate on a valid file descriptor
    throw IOException ("invalid file descriptor");
  }

  struct termios options; // The options for the file descriptor

  if (tcgetattr(fd_, &options) == -1)
  {
    throw IOException ("::tcgetattr");
  }

  // set up raw mode / no echo / binary
  options.c_cflag |= (unsigned long)  (CLOCAL|CREAD);
  options.c_lflag &= (unsigned long) ~(ICANON|ECHO|ECHOE|ECHOK|ECHONL|
                                       ISIG|IEXTEN); //|ECHOPRT

  options.c_oflag &= (unsigned long) ~(OPOST);
  options.c_iflag &= (unsigned long) ~(INLCR|IGNCR|ICRNL|IGNBRK);
#ifdef IUCLC
  options.c_iflag &= (unsigned long) ~IUCLC;
#endif
#ifdef PARMRK
  options.c_iflag &= (unsigned long) ~PARMRK;
#endif

  // setup baud rate
  // TODO(ash_git): validate baud rate
  cfsetspeed(&options, baudrate_);

  // setup char len
  options.c_cflag &= (unsigned long) ~CSIZE;
  if (bytesize_ == EIGHTBITS)
      options.c_cflag |= CS8;
  else if (bytesize_ == SEVENBITS)
      options.c_cflag |= CS7;
  else if (bytesize_ == SIXBITS)
      options.c_cflag |= CS6;
  else if (bytesize_ == FIVEBITS)
      options.c_cflag |= CS5;
  else
      throw invalid_argument ("invalid char len");
  // setup stopbits
  if (stopbits_ == STOPBITS_ONE)
      options.c_cflag &= (unsigned long) ~(CSTOPB);
  else if (stopbits_ == STOPBITS_ONE_POINT_FIVE)
      options.c_cflag |=  (CSTOPB);  // XXX same as TWO.. there is no POSIX support for 1.5
  else if (stopbits_ == STOPBITS_TWO)
      options.c_cflag |=  (CSTOPB);
  else
      throw invalid_argument ("invalid stop bit");
  // setup parity
  options.c_iflag &= (unsigned long) ~(INPCK|ISTRIP);
  if (parity_ == PARITY_NONE)
  {
    options.c_cflag &= (unsigned long) ~(PARENB|PARODD);
  }
  else if (parity_ == PARITY_EVEN)
  {
    options.c_cflag &= (unsigned long) ~(PARODD);
    options.c_cflag |=  (PARENB);
  }
  else if (parity_ == PARITY_ODD)
  {
    options.c_cflag |=  (PARENB|PARODD);
  }
  else
  {
    throw invalid_argument ("invalid parity");
  }
  // setup flow control
  // xonxoff
#ifdef IXANY
  if (xonxoff_)
    options.c_iflag |=  (IXON|IXOFF); //|IXANY)
  else
    options.c_iflag &= (unsigned long) ~(IXON|IXOFF|IXANY);
#else
  if (xonxoff_)
    options.c_iflag |=  (IXON|IXOFF);
  else
    options.c_iflag &= (unsigned long) ~(IXON|IXOFF);
#endif
  // rtscts
#ifdef CRTSCTS
  if (rtscts_)
    options.c_cflag |=  (CRTSCTS);
  else
    options.c_cflag &= (unsigned long) ~(CRTSCTS);
#elif defined CNEW_RTSCTS
  if (rtscts_)
    options.c_cflag |=  (CNEW_RTSCTS);
  else
    options.c_cflag &= (unsigned long) ~(CNEW_RTSCTS);
#else
#error "OS Support seems wrong."
#endif

  options.c_cc[VMIN] = 1; // Minimum of 1 character in the buffer
  options.c_cc[VTIME] = 0; // timeout on waiting for new data

  // activate settings
  ::tcsetattr (fd_, TCSANOW, &options);
}

void
Serial::SerialImpl::close ()
{
  if (isOpen_ == true)
  {
    if (fd_ != -1)
    {
      ::close (fd_); // Ignoring the outcome
      fd_ = -1;
    }
    isOpen_ = false;
  }
}

bool
Serial::SerialImpl::isOpen () const
{
  return isOpen_;
}

size_t
Serial::SerialImpl::available ()
{
  if (!isOpen_)
  {
    return 0;
  }
  int count = 0;
  int result = ioctl (fd_, TIOCINQ, &count);
  if (result == 0)
  {
    return static_cast<size_t> (count);
  }
  else
  {
    throw IOException (errno);
  }
}

size_t
Serial::SerialImpl::read (char* buf, size_t size)
{
  if (!isOpen_)
  {
    throw PortNotOpenedException ("Serial::read");
  }
  fd_set readfds;
  ssize_t bytes_read = 0;
  while (true)
  {
    if (timeout_ != -1)
    {
      FD_ZERO (&readfds);
      FD_SET (fd_, &readfds);
      struct timeval timeout;
      timeout.tv_sec =                    timeout_ / 1000;
      timeout.tv_usec = static_cast<int> (timeout_ % 1000) * 1000;
      int r = select (fd_ + 1, &readfds, NULL, NULL, &timeout);

      if (r == -1 && errno == EINTR)
        continue;

      if (r == -1)
      {
        throw IOException (errno);
      }
    }

    if (timeout_ == -1 || FD_ISSET (fd_, &readfds))
    {
      bytes_read = ::read (fd_, buf, size);
      // read should always return some data as select reported it was
      // ready to read when we get to this point.
      if (bytes_read < 1)
      {
        // Disconnected devices, at least on Linux, show the
        // behavior that they are always ready to read immediately
        // but reading returns nothing.
        throw SerialExecption ("device reports readiness to read but "
                               "returned no data (device disconnected?)");
      }
      break;
    }
    else
    {
      break;
    }
  }
  return static_cast<size_t> (bytes_read);
}

size_t
Serial::SerialImpl::write (const string &data)
{
  if (isOpen_ == false)
  {
    throw PortNotOpenedException ("Serial::write");
  }
  
  fd_set writefds;
  ssize_t bytes_written = 0;
  while (true)
  {
    if (timeout_ != -1)
    {
      FD_ZERO (&writefds);
      FD_SET (fd_, &writefds);
      struct timeval timeout;
      timeout.tv_sec =                    timeout_ / 1000;
      timeout.tv_usec = static_cast<int> (timeout_ % 1000) * 1000;
      int r = select (fd_ + 1, NULL, &writefds, NULL, &timeout);

      if (r == -1 && errno == EINTR)
        continue;

      if (r == -1)
      {
        throw IOException (errno);
      }
    }

    if (timeout_ == -1 || FD_ISSET (fd_, &writefds))
    {
      bytes_written = ::write (fd_, data.c_str (), data.length ());
      // read should always return some data as select reported it was
      // ready to read when we get to this point.
      if (bytes_written < 1)
      {
        // Disconnected devices, at least on Linux, show the
        // behavior that they are always ready to read immediately
        // but reading returns nothing.
        throw SerialExecption ("device reports readiness to read but "
                               "returned no data (device disconnected?)");
      }
      break;
    }
    else
    {
      break;
    }
  }
  if (bytes_written != static_cast<ssize_t> (data.length ()))
  {
    throw IOException ("Write did not complete");
  }
  return static_cast<size_t> (bytes_written);
}

void
Serial::SerialImpl::setPort (const string &port)
{
  port_ = port;
}

string
Serial::SerialImpl::getPort () const
{
  return port_;
}

void
Serial::SerialImpl::setTimeout (long timeout)
{
  timeout_ = timeout;
}

long
Serial::SerialImpl::getTimeout () const
{
  return timeout_;
}

void
Serial::SerialImpl::setBaudrate (unsigned long baudrate)
{
  baudrate_ = baudrate;
  if (isOpen_)
    reconfigurePort ();
}

unsigned long
Serial::SerialImpl::getBaudrate () const
{
  return baudrate_;
}

void
Serial::SerialImpl::setBytesize (serial::bytesize_t bytesize)
{
  bytesize_ = bytesize;
  if (isOpen_)
    reconfigurePort ();
}

serial::bytesize_t
Serial::SerialImpl::getBytesize () const
{
  return bytesize_;
}

void
Serial::SerialImpl::setParity (serial::parity_t parity)
{
  parity_ = parity;
  if (isOpen_)
    reconfigurePort ();
}

serial::parity_t
Serial::SerialImpl::getParity () const
{
  return parity_;
}

void
Serial::SerialImpl::setStopbits (serial::stopbits_t stopbits)
{
  stopbits_ = stopbits;
  if (isOpen_)
    reconfigurePort ();
}

serial::stopbits_t
Serial::SerialImpl::getStopbits () const
{
  return stopbits_;
}

void
Serial::SerialImpl::setFlowcontrol (serial::flowcontrol_t flowcontrol)
{
  flowcontrol_ = flowcontrol;
  if (isOpen_)
    reconfigurePort ();
}

serial::flowcontrol_t
Serial::SerialImpl::getFlowcontrol () const
{
  return flowcontrol_;
}

void
Serial::SerialImpl::flush ()
{
  if (isOpen_ == false)
  {
    throw PortNotOpenedException ("Serial::flush");
  }
  tcdrain (fd_);
}

void
Serial::SerialImpl::flushInput ()
{
  if (isOpen_ == false)
  {
    throw PortNotOpenedException ("Serial::flushInput");
  }
  tcflush (fd_, TCIFLUSH);
}

void
Serial::SerialImpl::flushOutput ()
{
  if (isOpen_ == false)
  {
    throw PortNotOpenedException ("Serial::flushOutput");
  }
  tcflush (fd_, TCOFLUSH);
}

void
Serial::SerialImpl::sendBreak (int duration)
{
  if (isOpen_ == false)
  {
    throw PortNotOpenedException ("Serial::sendBreak");
  }
  tcsendbreak (fd_, static_cast<int> (duration/4));
}

void
Serial::SerialImpl::setBreak (bool level)
{
  if (isOpen_ == false)
  {
    throw PortNotOpenedException ("Serial::setBreak");
  }
  if (level)
  {
    ioctl (fd_, TIOCSBRK);
  }
  else {
    ioctl (fd_, TIOCCBRK);
  }
}

void
Serial::SerialImpl::setRTS (bool level)
{
  if (isOpen_ == false)
  {
    throw PortNotOpenedException ("Serial::setRTS");
  }
  if (level)
  {
    ioctl (fd_, TIOCMBIS, TIOCM_RTS);
  }
  else {
    ioctl (fd_, TIOCMBIC, TIOCM_RTS);
  }
}

void
Serial::SerialImpl::setDTR (bool level)
{
  if (isOpen_ == false)
  {
    throw PortNotOpenedException ("Serial::setDTR");
  }
  if (level)
  {
    ioctl (fd_, TIOCMBIS, TIOCM_DTR);
  }
  else
  {
    ioctl (fd_, TIOCMBIC, TIOCM_DTR);
  }
}

bool
Serial::SerialImpl::getCTS ()
{
  if (isOpen_ == false)
  {
    throw PortNotOpenedException ("Serial::getCTS");
  }
  int s = ioctl (fd_, TIOCMGET, 0);
  return (s & TIOCM_CTS) != 0;
}

bool
Serial::SerialImpl::getDSR()
{
  if (isOpen_ == false)
  {
    throw PortNotOpenedException ("Serial::getDSR");
  }
  int s = ioctl(fd_, TIOCMGET, 0);
  return (s & TIOCM_DSR) != 0;
}

bool
Serial::SerialImpl::getRI()
{
  if (isOpen_ == false)
  {
    throw PortNotOpenedException ("Serial::getRI");
  }
  int s = ioctl (fd_, TIOCMGET, 0);
  return (s & TIOCM_RI) != 0;
}

bool
Serial::SerialImpl::getCD()
{
  if (isOpen_ == false)
  {
    throw PortNotOpenedException ("Serial::getCD");
  }
  int s = ioctl (fd_, TIOCMGET, 0);
  return (s & TIOCM_CD) != 0;
}
