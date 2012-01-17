/* Copyright 2012 William Woodall and John Harrison */

#include <alloca.h>

#include <cstring>
#include <algorithm>

#include <boost/thread/mutex.hpp>

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

using boost::mutex;

Serial::Serial (const string &port, unsigned long baudrate, long timeout,
                bytesize_t bytesize, parity_t parity, stopbits_t stopbits,
                flowcontrol_t flowcontrol, const size_t buffer_size)
 : buffer_size_(buffer_size)
{
  mutex::scoped_lock scoped_lock(mut);
  pimpl_ = new SerialImpl (port, baudrate, timeout, bytesize, parity,
                           stopbits, flowcontrol);
  read_cache_ = new char[buffer_size_];
  memset (read_cache_, 0, buffer_size_ * sizeof (char));
}

Serial::~Serial ()
{
  delete   pimpl_;
  delete[] read_cache_;
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
  memset (read_cache_, 0, buffer_size_ * sizeof (char));
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
  mutex::scoped_lock scoped_lock (mut);
  size_t cache_size = strlen (read_cache_);
  if (cache_size >= size)
  {
    // Don't need to do a new read.
    string result (read_cache_, size);
    memmove (read_cache_, read_cache_ + size, cache_size - size);
    *(read_cache_ + cache_size - size) = '\0';
    return result;
  }
  else
  {
    // Needs to read, loop until we have read enough... or timeout
    size_t chars_left = 0;
    string result = "";

    if (cache_size > 0)
    {
      result.append (read_cache_, cache_size);
      memset (read_cache_, 0, buffer_size_);
      chars_left = size - cache_size;
    }
    else
    {
      chars_left = size;
    }

    while (true)
    {
      size_t chars_read = pimpl_->read (read_cache_, buffer_size_ - 1);
      if (chars_read > 0)
      {
        *(read_cache_ + chars_read) = '\0';
        if (chars_left > chars_read)
        {
          memset (read_cache_, 0, buffer_size_);
          result.append (read_cache_);
          chars_left -= chars_read;
        }
        else
        {
          result.append (read_cache_, static_cast<size_t> (chars_left));
          memmove (read_cache_, read_cache_ + chars_left, chars_read - chars_left);
          *(read_cache_ + chars_read - chars_left) = '\0';
          memset (read_cache_ + chars_read - chars_left, 0,
                  buffer_size_ - chars_read - chars_left);
          // Finished reading all of the data
          break;
        }
      }
      else
        break; // Timeout occured
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
    throw "Error, must be set for readlines";
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
  mutex::scoped_lock scoped_lock(mut);
  return pimpl_->write (data);
}

void
Serial::setPort (const string &port)
{
  mutex::scoped_lock scoped_lock(mut);
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
  mutex::scoped_lock scoped_lock (mut);
  pimpl_->flush ();
  memset (read_cache_, 0, buffer_size_);
}

void Serial::flushInput ()
{
  pimpl_->flushInput ();
}

void Serial::flushOutput ()
{
  mutex::scoped_lock scoped_lock (mut);
  pimpl_->flushOutput ();
  memset (read_cache_, 0, buffer_size_);
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
