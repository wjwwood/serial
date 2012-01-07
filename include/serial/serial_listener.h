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

#ifndef SERIAL_LISTENER_H
#define SERIAL_LISTENER_H

// Serial
#include <serial/serial.h>

// Boost
#include <boost/function.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

namespace serial {

/*!
 * This is a general function type that is used as the callback prototype 
 * for asynchronous functions like the default handler callback and the 
 * listenFor callbacks.
 * 
 * The function takes a std::string reference and returns nothing, it is 
 * simply passing the resulting line detected by the comparator to the user's 
 * callback for processing.
 * 
 * \see SerialListener::listenFor, SerialListener::setDefaultHandler
 */
typedef boost::function<void(const std::string&)> DataCallback;

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

/*!
 * This function type describes the prototype for the logging callbacks.
 * 
 * The function takes a std::string reference and returns nothing.  It is 
 * called from the library when a logging message occurs.  This 
 * allows the library user to hook into this and integrate it with their own 
 * logging system.  It can be set with any of the set<log level>Handler 
 * functions.
 * 
 * \see SerialListener::setInfoHandler, SerialListener::setDebugHandler, 
 * SerialListener::setWarningHandler
 */
typedef boost::function<void(const std::string&)> LoggingCallback;

/*!
 * This function type describes the prototype for the exception callback.
 * 
 * The function takes a std::exception reference and returns nothing.  It is 
 * called from the library when an exception occurs in a library thread.
 * This exposes these exceptions to the user so they can to error handling.
 * 
 * \see SerialListener::setExceptionHandler
 */
typedef boost::function<void(const std::exception&)> ExceptionCallback;

/*!
 * This function type describes the prototype for the tokenizer callback.
 * 
 * The function should take a std::string reference and tokenize it into a 
 * several std::string's and store them in the given 
 * std::vector<std::string> reference.  There are some default ones or the 
 * user can create their own.
 * 
 * The last element in the std::vector of std::string's should always be 
 * either an empty string ("") or the last partial message.  The last element 
 * in the std::vector will be put back into the data buffer so that if it is 
 * incomplete it can be completed when more data is read.
 * 
 * Example: A delimeter tokenizer with a delimeter of "\r".  The result would 
 * be: "msg1\rmsg2\r" -> ["msg1", "msg2", ""] for all complete messages, or: 
 * "msg1\rpartial_msg2" -> ["msg1","partial_msg2"] for partial messages.
 * 
 * \see SerialListener::setTokenizer, serial::delimeter_tokenizer
 */
typedef boost::function<void(std::string&,std::vector<std::string>&)>
TokenizerType;

/*! This is a convenience alias for boost::uuids::uuid. */
typedef boost::uuids::uuid uuid_type; // uuid_t is already taken! =(

void
_delimeter_tokenizer (std::string &data, std::vector<std::string> &tokens,
                      std::string delimeter);

/*!
 * This returns a tokenizer that splits on a given delimeter.
 * 
 * The delimeter is passed into the function and a TokenizerType is returned 
 * that can be passed to SerialListener::setTokenizer.
 * 
 * Example:
 * <pre>
 *   my_listener.setTokenizer(delimeter_tokenizer("\r"));
 * <\pre>
 * 
 * \see SerialListener::setTokenizer, serial::TokenizerType
 */
class delimeter_tokenizer
{
public:
  delimeter_tokenizer ();
  virtual ~delimeter_tokenizer ();

private:
  /* data */
};
TokenizerType
delimeter_tokenizer (std::string delimeter);

/*!
 * This is a general exception generated by the SerialListener class.
 * 
 * Check the SerialListenerException::what function for the cause.
 
 * \param e_what is a std::string that describes the cause of the error.
 */
class SerialListenerException : public std::exception {
  const std::string e_what;
public:
  SerialListenerException(const std::string e_what) : e_what(e_what) {}
  ~SerialListenerException() throw() {std::exception::~exception();}

  virtual const char* what() const throw() {
    std::stringstream ss;
    ss << "SerialListenerException: " << this->e_what;
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

/***** Configurations ******/

  /*!
   * Sets the time-to-live (ttl) for messages waiting to be processsed.
   * 
   * Messages are processed before checking for expiration, therefore they 
   * will always be passed through filters once before being removed 
   * due to ttl expiration. The default value for this is 10 ms.
   * 
   * \param ms Time in milliseconds until messages are purged from the buffer.
   */
  void setTimeToLive (size_t ms = 10);

  /*!
   * Sets the tokenizer to be used when tokenizing the data into tokens.
   * 
   * This function is given a std::string of data and is responsible for 
   * tokenizing that data into a std::vector<std::string> of data tokens.
   * The default tokenizer splits the data by the ascii return carriage.
   * The user can create their own tokenizer or use one of the default ones.
   * 
   * \param tokenizer Function for tokenizing the incoming data.
   * 
   * \see serial::TokenizerType, serial::delimeter_tokenizer
   */
  void setTokenizer (TokenizerType tokenizer) {
    this->tokenize = tokenizer;
  }

/***** Start and Stop Listening ******/

  /*!
   * Starts a thread to listen for messages and process them through filters.
   * 
   * \param serial_port Pointer to a serial::Serial object that is used to 
   * retrieve new data.
   */
  void startListening (serial::Serial * serial_port);

  /*!
   * Stops the listening thread and blocks until it completely stops.
   * 
   * This function also clears all of the active filters from listenFor and 
   * similar functions.
   */
  void stopListening ();

/***** Filter Functions ******/

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

  /*!
   * Setups up a filter that calls a callback when a comparator returns true.
   * 
   * The user provides a comparator and a callback, and every time a line is 
   * received the comparator is called and the comparator has to evaluate the 
   * line and return true if it matches and false if it doesn't.  If it does 
   * match, the callback is called with the resulting line.
   * 
   * \param comparator This is a comparator for detecting if a line matches.
   * The comparartor receives a std::string reference and must return a true 
   * if it matches and false if it doesn't.
   * 
   * \param callback This is the handler for when a match occurs. It is given 
   * a std::string reference of the line that matched your comparator.
   * 
   * \return boost::uuids::uuid a unique identifier used to remove the filter.
   */
  uuid_type listenFor (ComparatorType comparator, DataCallback callback);

  /*!
   * Removes a filter by a given uuid.
   * 
   * The uuid for a filter is returned by the listenFor function.
   * 
   * \param filter_uuid The uuid of the filter to be removed.
   */
  void stopListeningFor (uuid_type filter_uuid);

  /*!
   * Stops listening for anything, but doesn't stop reading the serial port.
   */
  void stopListeningForAll ();

/***** Hooks and Handlers ******/

  /*!
   * Sets the handler to be called when a lines is not caught by a filter.
   * 
   * This allows you to set a catch all function that will get called 
   * everytime a line is not matched by a filter and the ttl expires.
   * 
   * Setting the callbacks works just like SerialListener::setInfoHandler.
   * 
   * \param default_handler A function pointer to the callback to handle 
   * unmatched and expired messages.
   * 
   * \see serial::DataCallback, SerialListener::setInfoHandler
   */
  void setDefaultHandler(DataCallback default_handler) {
    this->default_handler = default_handler;
  }

  /*!
   * Sets the function to be called when an info logging message occurs.
   * 
   * This allows you to hook into the message reporting of the library and use
   * your own logging facilities.
   * 
   * The provided function must follow this prototype:
   * <pre>
   *    void yourInfoCallback(const std::string &msg)
   * </pre>
   * Here is an example:
   * <pre>
   *    void yourInfoCallback(const std::string &msg) {
   *        std::cout << "SerialListener Info: " << msg << std::endl;
   *    }
   * </pre>
   * And the resulting call to make it the callback:
   * <pre>
   *    serial::SerialListener listener;
   *    listener.setInfoCallback(yourInfoCallback);
   * </pre>
   * Alternatively you can use a class method as a callback using boost::bind:
   * <pre>
   *    #include <boost/bind.hpp>
   *    
   *    #include "serial/serial_listener.h"
   *    
   *    class MyClass
   *    {
   *    public:
   *     MyClass () {
   *      listener.setInfoHandler(
   *          boost::bind(&MyClass::handleInfo, this, _1));
   *     }
   *    
   *     void handleInfo(const std::string &msg) {
   *       std::cout << "MyClass Info: " << msg << std::endl;
   *     }
   *    
   *    private:
   *     serial::SerialListener listener;
   *    };
   * </pre>
   * 
   * \param info_handler A function pointer to the callback to handle new 
   * Info messages.
   * 
   * \see serial::LoggingCallback
   */
  void setInfoHandler(LoggingCallback info_handler) {
    this->info = info_handler;
  }
  
  /*!
   * Sets the function to be called when a debug logging message occurs.
   * 
   * This allows you to hook into the message reporting of the library and use
   * your own logging facilities.
   * 
   * This works just like SerialListener::setInfoHandler.
   * 
   * \param debug_handler A function pointer to the callback to handle new 
   * Debug messages.
   * 
   * \see serial::LoggingCallback, SerialListener::setInfoHandler
   */
  void setDebugHandler(LoggingCallback debug_handler) {
    this->debug = debug_handler;
  }
  
  /*!
   * Sets the function to be called when a warning logging message occurs.
   * 
   * This allows you to hook into the message reporting of the library and use
   * your own logging facilities.
   * 
   * This works just like SerialListener::setInfoHandler.
   * 
   * \param warning_handler A function pointer to the callback to handle new 
   * Warning messages.
   * 
   * \see serial::LoggingCallback, SerialListener::setInfoHandler
   */
  void setWarningHandler(LoggingCallback warning_handler) {
    this->warn = warning_handler;
  }

private:
  // Function that loops while listening is true
  void listen ();
  // Called by listen iteratively
  std::string listenOnce (std::string data);
  // Determines how much to read on each loop of listen
  size_t determineAmountToRead ();
  // Used in the look for string once function
  bool listenForOnceComparator(std::string line);

  // Tokenizer
  TokenizerType tokenize;

  // Logging handlers
  LoggingCallback warn;
  LoggingCallback info;
  LoggingCallback debug;

  // Exception handler
  ExceptionCallback handle_exc;

  // Default handler
  DataCallback default_handler;

  // Persistent listening variables
  bool listening;
  serial::Serial * serial_port;
  boost::thread listen_thread;
  std::string buffer;
  std::map<const uuid_type,std::string> lines;
  std::map<const uuid_type,boost::posix_time::ptime> ttls;

  // For generating random uuids
  boost::uuids::random_generator random_generator;

  // Setting for ttl on messages
  boost::posix_time::time_duration ttl;

  // map<uuid, filter type (blocking/non-blocking)>
  std::map<const uuid_type,std::string> filters;
  // map<uuid, comparator>
  std::map<const uuid_type,ComparatorType> comparators;
  // map<uuid, callback>
  std::map<const uuid_type,DataCallback> callbacks;
  // map<uuid, conditional_variables>
  std::map<const uuid_type,boost::condition_variable*> condition_vars;
  // Mutex for locking use of filters
  boost::mutex filter_mux;

  // Used as temporary storage for listenForStringOnce
  std::string current_listen_for_one_target;
};

}

#endif // SERIAL_LISTENER_H