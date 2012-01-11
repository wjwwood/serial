#include "serial/serial.h"

#ifdef _WIN32
#include "serial/impl/win.h"
#else
#include "serial/impl/unix.h"
#endif

using serial::Serial;
using serial::bytesize_t;
using serial::parity_t;
using serial::stopbits_t;
using serial::flowcontrol_t;
using std::string;

Serial::Serial (const string &port, int baudrate, 
                            long timeout, bytesize_t bytesize,
                            parity_t parity, stopbits_t stopbits,
                            flowcontrol_t flowcontrol)
: pimpl(new SerialImpl(port,baudrate,timeout,bytesize,parity,stopbits,
                        flowcontrol))
{
  
}

Serial::~Serial () {
  delete pimpl;
}

void
Serial::open () {
  this->pimpl->open ();
}

void
Serial::close () {
  this->pimpl->close ();
}
bool
Serial::isOpen () {
  return this->pimpl->isOpen ();
}

size_t
Serial::read (unsigned char* buffer, size_t size) {
 // return this->pimpl->read (buffer, size);
}

string
Serial::read (size_t size) {
  return this->pimpl->read (size);
}

size_t
Serial::read (string &buffer, size_t size) {
//  return this->pimpl->read (buffer, size);
}

//size_t
//Serial::write (unsigned char* data, size_t length) {
//  return this->pimpl->write (data, length);
//}

size_t
Serial::write (const string &data) {
  return this->pimpl->write (data);
}

void
Serial::setPort (const string &port) {
  this->pimpl->setPort (port);
}

string
Serial::getPort () const {
  return this->pimpl->getPort ();
}

void
Serial::setTimeout (long timeout) {
  this->pimpl->setTimeout (timeout);
}

long
Serial::getTimeout () const {
  return this->pimpl->getTimeout ();
}

void
Serial::setBaudrate (int baudrate) {
  this->pimpl->setBaudrate (baudrate);
}

int
Serial::getBaudrate () const {
  return this->pimpl->getBaudrate ();
}

void
Serial::setBytesize (bytesize_t bytesize) {
  this->pimpl->setBytesize (bytesize);
}

bytesize_t
Serial::getBytesize () const {
  return this->pimpl->getBytesize ();
}

void
Serial::setParity (parity_t parity) {
  this->pimpl->setParity (parity);
}

parity_t
Serial::getParity () const {
  return this->pimpl->getParity ();
}

void
Serial::setStopbits (stopbits_t stopbits) {
  this->pimpl->setStopbits (stopbits);
}

stopbits_t
Serial::getStopbits () const {
  return this->pimpl->getStopbits ();
}

void
Serial::setFlowcontrol (flowcontrol_t flowcontrol) {
  this->pimpl->setFlowcontrol (flowcontrol);
}

flowcontrol_t
Serial::getFlowcontrol () const {
  return this->pimpl->getFlowcontrol ();
}




