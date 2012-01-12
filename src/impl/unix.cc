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

using ::serial::Serial;
using std::string;

Serial::SerialImpl::SerialImpl (const string &port, int baudrate,
                                    long timeout, bytesize_t bytesize,
                                    parity_t parity, stopbits_t stopbits,
                                    flowcontrol_t flowcontrol)
: fd_(-1), isOpen_(false), interCharTimeout_(-1), port_(port), baudrate_(baudrate),
  timeout_(timeout), bytesize_(bytesize), parity_(parity), stopbits_(stopbits),
  flowcontrol_(flowcontrol)
{
  printf("Got port: %s\n", port.c_str());
  if (!port_.empty()) {
    this->open();
  }
}

Serial::SerialImpl::~SerialImpl () {
  this->close();
}

void
Serial::SerialImpl::open () {
  if (port_.empty()) {
    printf("Port was empty\n");
    throw "error";
  }
  if (isOpen_ == true) {
    printf("Port already opened\n");
    throw "error";
  }
  
  fd_ = ::open (port_.c_str(), O_RDWR | O_NOCTTY | O_NONBLOCK);

  if (fd_ == -1) {
    printf("Error opening serial port %s - %s(%d).\n",
           port_.c_str(), strerror(errno), errno);
    throw "Error"; // Error
  }

  reconfigurePort();
  isOpen_ = true;
}

void
Serial::SerialImpl::reconfigurePort () {
  if (fd_ == -1) {
    throw "Error"; // Can only operate on a valid file descriptor
  }
  
  struct termios options; // The current options for the file descriptor
  struct termios originalTTYAttrs; // The orignal file descriptor options
  
  uint8_t vmin = 0, vtime = 0;                // timeout is done via select
  if (interCharTimeout_ == -1) {
    vmin = 1;
    vtime = int(interCharTimeout_ * 10);
  }
  
  if (tcgetattr(fd_, &originalTTYAttrs) == -1) {
    throw "Error";
  }
  
  options = originalTTYAttrs;
  
  // set up raw mode / no echo / binary
  options.c_cflag |=  (CLOCAL|CREAD);
  options.c_lflag &= ~(ICANON|ECHO|ECHOE|ECHOK|ECHONL|
                     ISIG|IEXTEN); //|ECHOPRT

  options.c_oflag &= ~(OPOST);
  options.c_iflag &= ~(INLCR|IGNCR|ICRNL|IGNBRK);
#ifdef IUCLC
  options.c_iflag &= ~IUCLC;
#endif
#ifdef PARMRK
  options.c_iflag &= ~PARMRK;
#endif

  // setup baud rate
  // TODO(ash_git): validate baud rate
  cfsetspeed(&options, baudrate_);

  // setup char len
  options.c_cflag &= ~CSIZE;
  if (bytesize_ == EIGHTBITS)
      options.c_cflag |= CS8;
  else if (bytesize_ == SEVENBITS)
      options.c_cflag |= CS7;
  else if (bytesize_ == SIXBITS)
      options.c_cflag |= CS6;
  else if (bytesize_ == FIVEBITS)
      options.c_cflag |= CS5;
  else
      throw "ValueError(Invalid char len: %%r)";
  // setup stopbits
  if (stopbits_ == STOPBITS_ONE)
      options.c_cflag &= ~(CSTOPB);
  else if (stopbits_ == STOPBITS_ONE_POINT_FIVE)
      options.c_cflag |=  (CSTOPB);  // XXX same as TWO.. there is no POSIX support for 1.5
  else if (stopbits_ == STOPBITS_TWO)
      options.c_cflag |=  (CSTOPB);
  else 
      throw "ValueError(Invalid stop bit specification:)";
  // setup parity
  options.c_iflag &= ~(INPCK|ISTRIP);
  if (parity_ == PARITY_NONE) {
    options.c_cflag &= ~(PARENB|PARODD);
  }
  else if (parity_ == PARITY_EVEN) {
    options.c_cflag &= ~(PARODD);
    options.c_cflag |=  (PARENB);
  } 
  else if (parity_ == PARITY_ODD) {
    options.c_cflag |=  (PARENB|PARODD);
  }
  else {
    throw "ValueError(Invalid parity:";
  }
  // setup flow control
  // xonxoff
#ifdef IXANY
  if (xonxoff_)
    options.c_iflag |=  (IXON|IXOFF); //|IXANY)
  else
    options.c_iflag &= ~(IXON|IXOFF|IXANY);
#else
  if (xonxoff_)
    options.c_iflag |=  (IXON|IXOFF);
  else
    options.c_iflag &= ~(IXON|IXOFF);
#endif
  // rtscts
#ifdef CRTSCTS
  if (rtscts_)
    options.c_cflag |=  (CRTSCTS);
  else
    options.c_cflag &= ~(CRTSCTS);
#elif defined CNEW_RTSCTS
  if (rtscts_)
    options.c_cflag |=  (CNEW_RTSCTS);
  else
    options.c_cflag &= ~(CNEW_RTSCTS);
#else
#error "OS Support seems wrong."
#endif

  // buffer
  // vmin "minimal number of characters to be read. = for non blocking"
  options.c_cc[VMIN] = vmin;
  // vtime
  options.c_cc[VTIME] = vtime;

  // activate settings
  ::tcsetattr(fd_, TCSANOW, &options);
}

void
Serial::SerialImpl::close () {
  if (isOpen_ == true) {
    if (fd_ != -1) {
      ::close(fd_);
      fd_ = -1;
    }
    isOpen_ = false;
  }
}

bool
Serial::SerialImpl::isOpen () {
  return isOpen_;
}

size_t
Serial::SerialImpl::available () {
  if (!isOpen_) {
    return 0;
  }
  int count = 0;
  int result = ioctl(fd_, TIOCINQ, &count);
  if (result == 0) {
    return count;
  }
  else {
    throw "Error";
  }
}

string
Serial::SerialImpl::read (size_t size) {
  if (!isOpen_) {
    throw "PortNotOpenError()"; //
  }
  string message = "";
  char buf[1024]; // TODO(ash_gti): Should this be 1024? or...?
  fd_set readfds;
  while (message.length() < size) {
    FD_ZERO(&readfds);
    FD_SET(fd_, &readfds);
    struct timeval timeout;
    timeout.tv_sec = timeout_ / 1000;
    timeout.tv_usec = timeout_ % 1000;
    int r = select(fd_ + 1, &readfds, NULL, NULL, &timeout);

    if (r == -1 && errno == EINTR)
      continue;

    if (r == -1) {
      perror("select()");
      exit(EXIT_FAILURE);
    }

    if (FD_ISSET(fd_, &readfds)) {
      memset(buf, 0, 1024);
      size_t bytes_read = ::read(fd_, buf, size-strlen(buf));
      // read should always return some data as select reported it was
      // ready to read when we get to this point.
      if (bytes_read < 1) {
        // Disconnected devices, at least on Linux, show the
        // behavior that they are always ready to read immediately
        // but reading returns nothing.
        throw "SerialException('device reports readiness to read but returned no data (device disconnected?)')";
      }
      message.append(buf, bytes_read);
    }
    else {
      break; // Timeout
    }
  }
  return message;
}

size_t
Serial::SerialImpl::write (const string &data) {
  if (isOpen_ == false) {
    throw "portNotOpenError";
  }
  size_t t = data.length();
  size_t n = ::write(fd_, data.c_str(), data.length());
  if (n == -1) {
    throw "Write error";
  }
  return n;
}

void
Serial::SerialImpl::setPort (const string &port) {
  port_ = port;
}

string
Serial::SerialImpl::getPort () const {
  return port_;
}

void
Serial::SerialImpl::setTimeout (long timeout) {
  timeout_ = timeout;
}

long
Serial::SerialImpl::getTimeout () const {
  return timeout_;
}

void
Serial::SerialImpl::setBaudrate (int baudrate) {
  baudrate_ = baudrate;
  reconfigurePort();
}

int
Serial::SerialImpl::getBaudrate () const {
  return baudrate_;
}

void
Serial::SerialImpl::setBytesize (serial::bytesize_t bytesize) {
  bytesize_ = bytesize;
}

serial::bytesize_t
Serial::SerialImpl::getBytesize () const {
  return bytesize_;
}

void
Serial::SerialImpl::setParity (serial::parity_t parity) {
  parity_ = parity;
}

serial::parity_t
Serial::SerialImpl::getParity () const {
  return parity_;
}

void
Serial::SerialImpl::setStopbits (serial::stopbits_t stopbits) {
  stopbits_ = stopbits;
}

serial::stopbits_t
Serial::SerialImpl::getStopbits () const {
  return stopbits_;
}

void
Serial::SerialImpl::setFlowcontrol (serial::flowcontrol_t flowcontrol) {
  flowcontrol_ = flowcontrol;
}

serial::flowcontrol_t
Serial::SerialImpl::getFlowcontrol () const {
  return flowcontrol_;
}


void Serial::SerialImpl::flush () {
  if (isOpen_ == false) {
    throw "portNotOpen";
  }
  tcdrain(fd_);
}
void Serial::SerialImpl::flushInput () {
  if (isOpen_ == false) {
    throw "portNotOpen";
  }
  tcflush(fd_, TCIFLUSH);
}
void Serial::SerialImpl::flushOutput () {
  if (isOpen_ == false) {
    throw "portNotOpen";
  }
  tcflush(fd_, TCOFLUSH);
}

void Serial::SerialImpl::sendBreak(int duration) {
  if (isOpen_ == false) {
    throw "portNotOpen";
  }
  tcsendbreak(fd_, int(duration/4));
}
void Serial::SerialImpl::setBreak(bool level) {
  if (isOpen_ == false) {
    throw "portNotOpen";
  }
  if (level) {
    ioctl(fd_, TIOCSBRK);
  }
  else {
    ioctl(fd_, TIOCCBRK);
  }
}
void Serial::SerialImpl::setRTS(bool level) {
  if (isOpen_ == false) {
    throw "portNotOpen";
  }
  if (level) {
    ioctl(fd_, TIOCMBIS, TIOCM_RTS);
  }
  else {
    ioctl(fd_, TIOCMBIC, TIOCM_RTS);
  }
}
void Serial::SerialImpl::setDTR(bool level) {
  if (isOpen_ == false) {
    throw "portNotOpen";
  }
  if (level) {
    ioctl(fd_, TIOCMBIS, TIOCM_DTR);
  }
  else {
    ioctl(fd_, TIOCMBIC, TIOCM_DTR);
  }

}
bool Serial::SerialImpl::getCTS() {
  if (isOpen_ == false) {
    throw "portNotOpen";
  }
  int s = ioctl(fd_, TIOCMGET, 0);
  return (s & TIOCM_CTS) != 0;
}
bool Serial::SerialImpl::getDSR() {
  if (isOpen_ == false) {
    throw "portNotOpen";
  }
  int s = ioctl(fd_, TIOCMGET, 0);
  return (s & TIOCM_DSR) != 0;
}
bool Serial::SerialImpl::getRI() {
  if (isOpen_ == false) {
    throw "portNotOpen";
  }
  int s = ioctl(fd_, TIOCMGET, 0);
  return (s & TIOCM_RI) != 0;
}
bool Serial::SerialImpl::getCD() {
  if (isOpen_ == false) {
    throw "portNotOpen";
  }
  int s = ioctl(fd_, TIOCMGET, 0);
  return (s & TIOCM_CD) != 0;
}







