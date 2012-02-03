/* Copyright 2012 William Woodall and John Harrison */

#include "serial/serial.h"

#ifdef _WIN32
#include "serial/impl/win.h"
#else
#include "serial/impl/unix.h"
#endif

using std::invalid_argument;
using std::memset;
using std::min;
using std::numeric_limits;
using std::vector;
using std::size_t;
using std::string;

using serial::Serial;
using serial::SerialExecption;
using serial::IOException;
using serial::bytesize_t;
using serial::parity_t;
using serial::stopbits_t;
using serial::flowcontrol_t;

class Serial::ScopedReadLock {
public:
  ScopedReadLock(SerialImpl *pimpl) : pimpl_(pimpl) {
    this->pimpl_->readLock();
  }
  ~ScopedReadLock() {
    this->pimpl_->readUnlock();
  }
private:
  SerialImpl *pimpl_;
};

class Serial::ScopedWriteLock {
public:
  ScopedWriteLock(SerialImpl *pimpl) : pimpl_(pimpl) {
    this->pimpl_->writeLock();
  }
  ~ScopedWriteLock() {
    this->pimpl_->writeUnlock();
  }
private:
  SerialImpl *pimpl_;
};

Serial::Serial (const string &port, unsigned long baudrate, long timeout,
                bytesize_t bytesize, parity_t parity, stopbits_t stopbits,
                flowcontrol_t flowcontrol)
 : read_cache_("")
{
  pimpl_ = new SerialImpl (port, baudrate, timeout, bytesize, parity,
                           stopbits, flowcontrol);
}

Serial::~Serial ()
{
  delete pimpl_;
}

void
Serial::open ()
{
  pimpl_->open ();
}

void
Serial::close ()
{
  pimpl_->close ();
}

bool
Serial::isOpen () const
{
  return pimpl_->isOpen ();
}

size_t
Serial::available ()
{
  return pimpl_->available ();
}

string
Serial::read (size_t size)
{
  ScopedReadLock(this->pimpl_);
  if (read_cache_.size() >= size)
  {
    // Don't need to do a new read.
    string result = read_cache_.substr (0, size);
    read_cache_ = read_cache_.substr (size, read_cache_.size ());
    return result;
  }
  else
  {
    // Needs to read, loop until we have read enough or timeout
    string result (read_cache_.substr (0, size));
    read_cache_.clear ();

    while (true)
    {
      char buf[256];
      size_t chars_read = pimpl_->read (buf, 256);
      if (chars_read > 0)
      {
        read_cache_.append(buf, chars_read);
      }
      else
        break; // Timeout occured
      
      if (chars_read > size)
      {
        result.append (read_cache_.substr (0, size));
        read_cache_ = read_cache_.substr (size, read_cache_.size ());
        break;
      }
      else
      {
        result.append (read_cache_.substr (0, size));
        read_cache_.clear ();
        size -= chars_read;
      }
    }
    return result;
  }
}

string
Serial::readline (size_t size, string eol)
{
  size_t leneol = eol.length ();
  string line = "";
  while (true)
  {
    string c = read (1);
    if (!c.empty ())
    {
      line.append (c);
      if (line.length () > leneol &&
          line.substr (line.length () - leneol, leneol) == eol)
        break;
      if (line.length () >= size)
      {
        break;
      }
    }
    else
      // Timeout
      break;
  }
  return line;
}

vector<string>
Serial::readlines(string eol)
{
  if (pimpl_->getTimeout () < 0)
  {
    throw invalid_argument ("Error, must be set for readlines");
  }
  size_t leneol = eol.length ();
  vector<string> lines;
  while (true)
  {
    string line = readline (numeric_limits<size_t>::max (), eol);
    if (!line.empty ())
    {
      lines.push_back (line);
      if (line.substr (line.length () - leneol, leneol) == eol)
        break;
    }
    else
      // Timeout
      break;
  }
  return lines;
}

size_t
Serial::write (const string &data)
{
  ScopedWriteLock(this->pimpl_);
  return pimpl_->write (data);
}

void
Serial::setPort (const string &port)
{
  ScopedReadLock(this->pimpl_);
  ScopedWriteLock(this->pimpl_);
  bool was_open = pimpl_->isOpen();
  if (was_open) close();
  pimpl_->setPort (port);
  if (was_open) open();
}

string
Serial::getPort () const
{
  return pimpl_->getPort ();
}

void
Serial::setTimeout (long timeout)
{
  pimpl_->setTimeout (timeout);
}

long
Serial::getTimeout () const {
  return pimpl_->getTimeout ();
}

void
Serial::setBaudrate (unsigned long baudrate)
{
  pimpl_->setBaudrate (baudrate);
}

unsigned long
Serial::getBaudrate () const
{
  return pimpl_->getBaudrate ();
}

void
Serial::setBytesize (bytesize_t bytesize)
{
  pimpl_->setBytesize (bytesize);
}

bytesize_t
Serial::getBytesize () const
{
  return pimpl_->getBytesize ();
}

void
Serial::setParity (parity_t parity)
{
  pimpl_->setParity (parity);
}

parity_t
Serial::getParity () const
{
  return pimpl_->getParity ();
}

void
Serial::setStopbits (stopbits_t stopbits)
{
  pimpl_->setStopbits (stopbits);
}

stopbits_t
Serial::getStopbits () const
{
  return pimpl_->getStopbits ();
}

void
Serial::setFlowcontrol (flowcontrol_t flowcontrol)
{
  pimpl_->setFlowcontrol (flowcontrol);
}

flowcontrol_t
Serial::getFlowcontrol () const
{
  return pimpl_->getFlowcontrol ();
}

void Serial::flush ()
{
  ScopedReadLock(this->pimpl_);
  ScopedWriteLock(this->pimpl_);
  pimpl_->flush ();
  read_cache_.clear ();
}

void Serial::flushInput ()
{
  ScopedReadLock(this->pimpl_);
  pimpl_->flushInput ();
}

void Serial::flushOutput ()
{
  ScopedWriteLock(this->pimpl_);
  pimpl_->flushOutput ();
  read_cache_.clear ();
}

void Serial::sendBreak (int duration)
{
  pimpl_->sendBreak (duration);
}

void Serial::setBreak (bool level)
{
  pimpl_->setBreak (level);
}

void Serial::setRTS (bool level)
{
  pimpl_->setRTS (level);
}

void Serial::setDTR (bool level)
{
  pimpl_->setDTR (level);
}

bool Serial::getCTS ()
{
  return pimpl_->getCTS ();
}

bool Serial::getDSR ()
{
  return pimpl_->getDSR ();
}

bool Serial::getRI ()
{
  return pimpl_->getRI ();
}

bool Serial::getCD ()
{
  return pimpl_->getCD ();
}
