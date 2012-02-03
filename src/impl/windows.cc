/* Copyright 2012 William Woodall and John Harrison */

#include "serial/impl/windows.h"

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
  bool custom_baud = false;
  speed_t baud;
  switch (baudrate_)
  {
#ifdef B0
    case 0: baud = B0; break;
#endif
#ifdef B50
    case 50: baud = B50; break;
#endif
#ifdef B75
    case 75: baud = B75; break;
#endif
#ifdef B110
    case 110: baud = B110; break;
#endif
#ifdef B134
    case 134: baud = B134; break;
#endif
#ifdef B150
    case 150: baud = B150; break;
#endif
#ifdef B200
    case 200: baud = B200; break;
#endif
#ifdef B300
    case 300: baud = B300; break;
#endif
#ifdef B600
    case 600: baud = B600; break;
#endif
#ifdef B1200
    case 1200: baud = B1200; break;
#endif
#ifdef B1800
    case 1800: baud = B1800; break;
#endif
#ifdef B2400
    case 2400: baud = B2400; break;
#endif
#ifdef B4800
    case 4800: baud = B4800; break;
#endif
#ifdef B7200
    case 7200: baud = B7200; break;
#endif
#ifdef B9600
    case 9600: baud = B9600; break;
#endif
#ifdef B14400
    case 14400: baud = B14400; break;
#endif
#ifdef B19200
    case 19200: baud = B19200; break;
#endif
#ifdef B28800
    case 28800: baud = B28800; break;
#endif
#ifdef B57600
    case 57600: baud = B57600; break;
#endif
#ifdef B76800
    case 76800: baud = B76800; break;
#endif
#ifdef B38400
    case 38400: baud = B38400; break;
#endif
#ifdef B115200
    case 115200: baud = B115200; break;
#endif
#ifdef B128000
    case 128000: baud = B128000; break;
#endif
#ifdef B153600
    case 153600: baud = B153600; break;
#endif
#ifdef B230400
    case 230400: baud = B230400; break;
#endif
#ifdef B256000
    case 256000: baud = B256000; break;
#endif
#ifdef B460800
    case 460800: baud = B460800; break;
#endif
#ifdef B921600
    case 921600: baud = B921600; break;
#endif
    default:
      custom_baud = true;
// Mac OS X 10.x Support
#if defined(__APPLE__) && defined(__MACH__)
#define IOSSIOSPEED _IOW('T', 2, speed_t)
      int new_baud = static_cast<int> (baudrate_);
      if (ioctl (fd_, IOSSIOSPEED, &new_baud, 1) < 0)
      {
        throw IOException (errno);
      }
// Linux Support
#elif defined(__linux__)
      struct serial_struct ser;
      ioctl(fd_, TIOCGSERIAL, &ser);
      // set custom divisor
      ser.custom_divisor = ser.baud_base / baudrate_;
      // update flags
      ser.flags &= ~ASYNC_SPD_MASK;
      ser.flags |= ASYNC_SPD_CUST;

      if (ioctl(fd_, TIOCSSERIAL, ser) < 0)
      {
        throw IOException (errno);
      }
#else
      throw invalid_argument ("OS does not currently support custom bauds");
#endif
  }
  if (custom_baud == false)
  {
#ifdef _BSD_SOURCE
    ::cfsetspeed(&options, baud);
#else
    ::cfsetispeed(&options, baud);
    ::cfsetospeed(&options, baud);
#endif
  }

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
  int count = 0;
  while (true)
  {
    count++;
    // printf("Counting: %u\n", count);
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

