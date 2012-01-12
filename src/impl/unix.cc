#include "serial/impl/unix.h"

using namespace serial;

Serial::Serial_pimpl::Serial_pimpl (const std::string &port, int baudrate,
                            long timeout, bytesize_t bytesize,
                            parity_t parity, stopbits_t stopbits,
                            flowcontrol_t flowcontrol)
: port(port), baudrate(baudrate), timeout(timeout), bytesize(bytesize),
  parity(parity), stopbits(stopbits), flowcontrol(flowcontrol)
{
  this->fd = -1;
}

Serial::Serial_pimpl::~Serial_pimpl () {
  if (this->isOpen())
    this->close();
}

void
Serial::Serial_pimpl::open () {
  
}

void
Serial::Serial_pimpl::close () {
  this->fd = -1;
}
bool
Serial::Serial_pimpl::isOpen () {
  return false;
}

size_t
Serial::Serial_pimpl::read (unsigned char* buffer, size_t size) {
  return 0;
}

std::string
Serial::Serial_pimpl::read (size_t size) {
  return "";
}

size_t
Serial::Serial_pimpl::read (std::string &buffer, size_t size) {
  return 0;
}

size_t
Serial::Serial_pimpl::write (unsigned char* data, size_t length) {
  return 0;
}

size_t
Serial::Serial_pimpl::write (const std::string &data) {
  return 0;
}

void
Serial::Serial_pimpl::setPort (const std::string &port) {
  
}

std::string
Serial::Serial_pimpl::getPort () const {
  return this->port;
}

void
Serial::Serial_pimpl::setTimeout (long timeout) {
  
}

long
Serial::Serial_pimpl::getTimeout () const {
  return this->timeout;
}

void
Serial::Serial_pimpl::setBaudrate (int baudrate) {
  
}

int
Serial::Serial_pimpl::getBaudrate () const {
  return this->baudrate;
}

void
Serial::Serial_pimpl::setBytesize (bytesize_t bytesize) {
  
}

bytesize_t
Serial::Serial_pimpl::getBytesize () const {
  return this->bytesize;
}

void
Serial::Serial_pimpl::setParity (parity_t parity) {
  
}

parity_t
Serial::Serial_pimpl::getParity () const {
  return this->parity;
}

void
Serial::Serial_pimpl::setStopbits (stopbits_t stopbits) {
  
}

stopbits_t
Serial::Serial_pimpl::getStopbits () const {
  return this->stopbits;
}

void
Serial::Serial_pimpl::setFlowcontrol (flowcontrol_t flowcontrol) {
  
}

flowcontrol_t
Serial::Serial_pimpl::getFlowcontrol () const {
  return this->flowcontrol;
}






