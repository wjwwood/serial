#include "serial/serial.h"

#ifdef _WIN32
#include "serial/impl/win.h"
#else
#include "serial/impl/unix.h"
#endif

using namespace serial;

Serial::Serial (const std::string &port, int baudrate, 
                            long timeout, bytesize_t bytesize,
                            parity_t parity, stopbits_t stopbits,
                            flowcontrol_t flowcontrol)
{
  pimpl = new Serial_pimpl(port,baudrate,timeout,bytesize,parity,stopbits,
                           flowcontrol);
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
  return this->pimpl->read (buffer, size);
}

std::string
Serial::read (size_t size) {
  return this->pimpl->read (size);
}

size_t
Serial::read (std::string &buffer, size_t size) {
  return this->pimpl->read (buffer, size);
}

size_t
Serial::write (unsigned char* data, size_t length) {
  return this->pimpl->write (data, length);
}

size_t
Serial::write (const std::string &data) {
  return this->pimpl->write (data);
}

void
Serial::setPort (const std::string &port) {
  this->pimpl->setPort (port);
}

std::string
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




