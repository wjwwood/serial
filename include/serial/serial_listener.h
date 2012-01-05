/*!
 * \file serial/serial_listener.h
 * \author William Woodall <wjwwood@gmail.com>
 * \version 0.1
 *
 * \section LICENSE
 *
 * The BSD License
 *
 * Copyright (c) 2011 William Woodall
 *
 * Permission is hereby granted, free of charge, to any person obtaining a 
 * copy of this software and associated documentation files (the "Software"), 
 * to deal in the Software without restriction, including without limitation 
 * the rights to use, copy, modify, merge, publish, distribute, sublicense, 
 * and/or sell copies of the Software, and to permit persons to whom the 
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING 
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER 
 * DEALINGS IN THE SOFTWARE.
 *
 * \section DESCRIPTION
 *
 * This provides a class that allows for asynchronous serial port reading.
 * 
 */

// Serial
#include <serial/serial.h>

// Boost
#include <boost/function.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

#ifndef SERIAL_LISTENER_TEST
#define SERIAL_LISTENER_TEST false
#endif

namespace serial {

/*!
 * This is a general function type that is used both as the callback prototype 
 * for asynchronous functions like the default handler callback and the 
 * listenFor callbacks.
 * 
 * The function takes a std::string reference and returns nothing, it is 
 * simply passing the resulting line detected by the comparator to the user's 
 * callback for processing.
 * 
 * \see SerialListener::listenFor, SerialListener::setDefaultHandler
 */
typedef boost::function<void(const std::string&)> SerialCallback;

/*!
 * This is a general function type that is used as the comparator callback 
 * prototpe for the listenFor* type functions.
 * 
 * The function takes a std::string reference and returns true if the string 
 * matches what the comparator is looking for and false if it does not, unless 
 * otherwise specified.
 * 
 * \see SerialListener::listenFor, SerialListener::listenForOnce
 */
typedef boost::function<bool(const std::string&)> ComparatorType;


typedef boost::function<void(const std::string&)> InfoCallback;
typedef boost::function<void(const std::string&)> WarningCallback;
typedef boost::function<void(const std::string&)> DebugCallback;
typedef boost::function<void(const std::exception&)> ExceptionCallback;

typedef boost::uuids::uuid uuid_t;

class SerialListenerException : public std::exception {
  const char * e_what;
public:
  SerialListenerException(const char * e_what) {this->e_what = e_what;}

  virtual const char* what() const throw() {
    std::stringstream ss;
    ss << "Error listening to serial port: " << this->e_what;
    return ss.str().c_str();
  }
};

/*!
 * Listens to a serial port, facilitates asynchronous reading
 */
class SerialListener
{
public:
  /*!
   * Creates a new Serial Listener.
   */
  SerialListener ();
  
  /*!
   * Destructor.
   */
  virtual ~SerialListener ();

  /*!
   * Sets the time-to-live (ttl) for messages waiting to be processsed.
   * 
   * \param ms Time in milliseconds until messages are purged from the buffer.
   */
  void setTimeToLive (size_t ms);

  /*!
   * Starts a thread to listen for messages and process them through filters.
   * 
   * \param serial_port Pointer to a serial::Serial object that is used to 
   * retrieve new data.
   */
  void startListening (serial::Serial * serial_port);

  /*!
   * Stops the listening thread and blocks until it completely stops.
   */
  void stopListening ();

  /*!
   * Blocks until the given string is detected or until the timeout occurs.
   * 
   * \param token std::string that should be watched for, this string must 
   * match the message exactly.
   * 
   * \param timeout in milliseconds before timing out and returning false.
   * Defaults to 1000 milliseconds or 1 second.
   * 
   * \return bool If true then the token was detected before the token, false 
   * if the token was not heard and the timeout occured.
   */
  bool listenForStringOnce (std::string token, size_t timeout = 1000);

  boost::uuids::uuid listenFor (ComparatorType, SerialCallback);
  void stopListeningFor (boost::uuids::uuid filter_uuid);

  InfoCallback info;
  WarningCallback warn;
  DebugCallback debug;
  ExceptionCallback handle_exc;
  SerialCallback default_handler;

private:
  void listen();
  std::string listenOnce(std::string data);
  size_t determineAmountToRead();
  bool listenForOnceComparator(std::string line);

  bool listening;

  serial::Serial * serial_port;

  boost::thread listen_thread;
  boost::uuids::random_generator random_generator();

  std::string buffer;
  std::map<const uuid_t,std::string> lines;
  std::map<const uuid_t,boost::posix_time::ptime> ttls;
  boost::posix_time::time_duration ttl;

  // map<uuid, filter type (blocking/non-blocking)>
  std::map<const uuid_t,std::string> filters;
  // map<uuid, comparator>
  std::map<const uuid_t,ComparatorType> comparators;
  // map<uuid, callback>
  std::map<const uuid_t,SerialCallback> callbacks;
  // map<uuid, conditional_variables>
  std::map<const uuid_t,boost::condition_variable*>
  condition_vars;

  // ptime time_start(microsec_clock::local_time());
  // //... execution goes here ...
  // ptime time_end(microsec_clock::local_time());
  // time_duration duration(time_end - time_start);

  std::string current_listen_for_one_target;

  boost::mutex filter_mux;
};

}