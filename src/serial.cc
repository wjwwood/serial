#include "serial/serial.h"

#ifdef _WIN32
#include "serial/impl/win.h"
#else
#include "serial/impl/unix.h"
#endif

Serial::Serial (const std::string &port, int baudrate, 
                            long timeout, bytesize_t bytesize,
                            parity_t parity, stopbits_t stopbits,
                            flowcontrol_t flowcontrol)
: impl(new Serial_pimpl(port,baudrate,timeout,bytesize,parity,stopbits,
                        flowcontrol))
{
  
}

Serial::~Serial () {
  delete impl;
}

void
Serial::open () {
  this->impl->open ();
}

void
Serial::close () {
  this->impl->close ();
}
bool
Serial::isOpen () {
  return this->impl->isOpen ();
}

size_t
Serial::read (unsigned char* buffer, size_t size = 1) {
  return this->impl->read (buffer, size);
}

std::string
Serial::read (size_t size = 1) {
  return this->impl->read (size);
}

size_t
Serial::read (std::string &buffer, size_t size = 1) {
  return this->impl->read (buffer, size);
}

size_t
Serial::write (unsigned char* data, size_t length) {
  return this->impl->write (data, length);
}

size_t
Serial::write (const std::string &data) {
  return this->impl->write (data);
}

void
Serial::setPort (const std::string &port) {
  this->impl->setPort (port);
}

std::string
Serial::getPort () const {
  return this->impl->getPort ();
}

void
Serial::setTimeout (long timeout) {
  this->impl->setTimeout (timeout);
}

long
Serial::getTimeout () const {
  return this->impl->getTimeout ();
}

void
Serial::setBaudrate (int baudrate) {
  this->impl->setBaudrate (baudrate);
}

int
Serial::getBaudrate () const {
  return this->impl->getBaudrate ();
}

void
Serial::setBytesize (bytesize_t bytesize) {
  this->impl->setBytesize (bytesize);
}

bytesize_t
Serial::getBytesize () const {
  return this->impl->getBytesize ();
}

void
Serial::setParity (parity_t parity) {
  this->impl->setParity (parity);
}

parity_t
Serial::getParity () const {
  return this->impl->getParity ();
}

void
Serial::setStopbits (stopbits_t stopbits) {
  this->impl->setStopbits (stopbits);
}

stopbits_t
Serial::getStopbits () const {
  return this->impl->getStopbits ();
}

void
Serial::setFlowcontrol (flowcontrol_t flowcontrol) {
  this->impl->setFlowcontrol (flowcontrol);
}

flowcontrol_t
Serial::getFlowcontrol () const {
  return this->impl->getFlowcontrol ();
}




