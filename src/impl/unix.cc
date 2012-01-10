#include "serial/impl/unix.h"

using namespace serial;

Serial_pimpl::Serial_pimpl (const std::string &port, int baudrate,
                            long timeout, bytesize_t bytesize,
                            parity_t parity, stopbits_t stopbits,
                            flowcontrol_t flowcontrol)
: port(port), baudrate(baudrate), timeout(timeout), bytesize(bytesize),
  parity(parity), stopbits(stopbits), flowcontrol(flowcontrol)
{
  
}

Serial_pimpl::~Serial_pimpl () {
  
}

void
Serial_pimpl::open () {
  
}

void
Serial_pimpl::close () {
  
}
bool
Serial_pimpl::isOpen () {
  
}

size_t
Serial_pimpl::read (unsigned char* buffer, size_t size = 1) {
  
}

std::string
Serial_pimpl::read (size_t size = 1) {
  
}

size_t
Serial_pimpl::read (std::string &buffer, size_t size = 1) {
  
}

size_t
Serial_pimpl::write (unsigned char* data, size_t length) {
  
}

size_t
Serial_pimpl::write (const std::string &data) {
  
}

void
Serial_pimpl::setPort (const std::string &port) {
  
}

std::string
Serial_pimpl::getPort () const {
  
}

void
Serial_pimpl::setTimeout (long timeout) {
  
}

long
Serial_pimpl::getTimeout () const {
  
}

void
Serial_pimpl::setBaudrate (int baudrate) {
  
}

int
Serial_pimpl::getBaudrate () const {
  
}

void
Serial_pimpl::setBytesize (bytesize_t bytesize) {
  
}

bytesize_t
Serial_pimpl::getBytesize () const {
  
}

void
Serial_pimpl::setParity (parity_t parity) {
  
}

parity_t
Serial_pimpl::getParity () const {
  
}

void
Serial_pimpl::setStopbits (stopbits_t stopbits) {
  
}

stopbits_t
Serial_pimpl::getStopbits () const {
  
}

void
Serial_pimpl::setFlowcontrol (flowcontrol_t flowcontrol) {
  
}

flowcontrol_t
Serial_pimpl::getFlowcontrol () const {
  
}






