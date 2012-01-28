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

#ifndef SERIAL_LISTENER_DEBUG
#define SERIAL_LISTENER_DEBUG 0
#endif

// STL
#include <queue>
#include <stdint.h>
#include <iostream>

// Serial
#include <serial/serial.h>

// Boost
#include <boost/function.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/thread.hpp>

#if SERIAL_LISTENER_DEBUG
# warning SerialListener in debug mode
#endif

namespace serial {

/*!
 * This is an alias to boost::shared_ptr<const std::string> used for tokens.
 *
 * This is the type used internally and is the type returned in a vector by
 * the tokenizer.  The shared_ptr allows for the token to be stored and kept
 * around long enough to be used by the comparators and callbacks, but no
 * longer.  This internal storage is passed as a const std::string reference
 * to callbacks, like the DataCallback function type, to prevent implicit
 * copying.
 *
 * \see serial::TokenizerType, serial::SerialListener::setTokenizer
 */
typedef boost::shared_ptr<const std::string> TokenPtr;

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
 * This function type describes the prototype for the tokenizer callback.
 * 
 * The function should take a std::string reference and tokenize it into a
 * several TokenPtr's and store them in the given std::vector<TokenPtr> 
 * reference.  There are some default ones or the user can create their own.
 * 
 * The last element in the std::vector of TokenPtr's should always be
 * either an empty string ("") or the last partial message.  The last element
 * in the std::vector will be put back into the data buffer so that if it is
 * incomplete it can be completed when more data is read.
 * 
 * Example: A delimeter tokenizer with a delimeter of "\r".  The result would
 * be: "msg1\rmsg2\r" -> ["msg1", "msg2", ""] for two complete messages, or:
 * "msg1\rpartial_msg2" -> ["msg1","partial_msg2"] for one complete message 
 * and one partial message.
 * 
 * \see SerialListener::setTokenizer, serial::delimeter_tokenizer,
 * serial::TokenPtr
 */
typedef boost::function<void(const std::string&, std::vector<TokenPtr>&)>
TokenizerType;

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
 * Represents a filter which new data is passed through.
 *
 * The filter consists of a comparator and a callback.  The comparator takes a
 * token and returns true if it matches, false if it doesn't.  If a match
 * occurs the serial listener will dispatch a call of the callback with the
 * matched data in a another thread.  The comparator should be as short as
 * possible, but the callback can be longer since it is executed in a thread
 * or thread pool.
 *
 * \param comparator A ComparatorType that matches incoming data, returns true
 * for a match, false othewise.
 *
 * \param callback A DataCallback that gets called when a match occurs.
 *
 * \see serial::ComparatorType, serial::DataCallback, serial::FilterPtr
 */
class Filter
{
public:
  Filter (ComparatorType comparator, DataCallback callback)
  : comparator_(comparator), callback_(callback) {}
  virtual ~Filter () {}

  ComparatorType comparator_;
  DataCallback callback_;

private:
  // Disable copy constructors
  Filter(const Filter&);
  void operator=(const Filter&);
  const Filter& operator=(Filter);
};

/*!
 * This is an alias to boost::shared_ptr<Filter> used for tokens.
 *
 * This is used internally and is returned from SerialListener::listenFor like
 * functions so that users can later remove those filters by passing the
 * FilterPtr.
 *
 * \see serial::Filter, serial::SerialListener::listenFor,
 * serial::SerialListener::listenForOnce
 */
typedef boost::shared_ptr<Filter> FilterPtr;

class BlockingFilter; // Forward declaration

/*!
 * Shared Pointer of BlockingFilter, returned by
 * SerialListener::createBlockingFilter.
 *
 * \see serial::BlockingFilter, SerialListener::createBlockingFilter
 */
typedef boost::shared_ptr<BlockingFilter> BlockingFilterPtr;

class BufferedFilter; // Forward declaration

/*!
 * Shared Pointer of BufferedFilter, returned by
 * SerialListener::createBufferedFilter.
 *
 * \see serial::BufferedFilter, SerialListener::createBufferedFilter
 */
typedef boost::shared_ptr<BufferedFilter> BufferedFilterPtr;

/*!
 * This is a general exception generated by the SerialListener class.
 *
 * Check the SerialListenerException::what function for the cause.
 *
 * \param e_what is a std::string that describes the cause of the error.
 */
class SerialListenerException : public std::exception {
  const std::string e_what_;
public:
  SerialListenerException(const std::string e_what) : e_what_(e_what) {}
  ~SerialListenerException() throw() {}

  virtual const char* what() const throw() {
    std::stringstream ss;
    ss << "SerialListenerException: " << this->e_what_;
    return ss.str().c_str();
  }
};

// Based on: http://www.justsoftwaresolutions.co.uk/threading/implementing-a-thread-safe-queue-using-condition-variables.html
template<typename Data>
class ConcurrentQueue
{
private:
  std::queue<Data> the_queue;
  mutable boost::mutex the_mutex;
  boost::condition_variable the_condition_variable;
public:
  void push(Data const& data) {
    boost::mutex::scoped_lock lock(the_mutex);
    the_queue.push(data);
    lock.unlock();
    the_condition_variable.notify_one();
  }

  bool empty() const {
    boost::mutex::scoped_lock lock(the_mutex);
    return the_queue.empty();
  }

  bool try_pop(Data& popped_value) {
    boost::mutex::scoped_lock lock(the_mutex);
    if(the_queue.empty()) {
      return false;
    }

    popped_value=the_queue.front();
    the_queue.pop();
    return true;
  }

  bool timed_wait_and_pop(Data& popped_value, long timeout) {
    using namespace boost::posix_time;
    bool result;
    boost::mutex::scoped_lock lock(the_mutex);
    result = !the_queue.empty();
    if (!result) {
      result = the_condition_variable.timed_wait(lock, milliseconds(timeout));
    }

    if (result) {
      popped_value=the_queue.front();
      the_queue.pop();
    }
    return result;
  }

  void wait_and_pop(Data& popped_value) {
    boost::mutex::scoped_lock lock(the_mutex);
    while(the_queue.empty()) {
      the_condition_variable.wait(lock);
    }

    popped_value=the_queue.front();
    the_queue.pop();
  }

  size_t size() const {
    return the_queue.size();
  }

  void cancel() {
    the_condition_variable.notify_one();
  }

  void clear() {
    boost::mutex::scoped_lock lock(the_mutex);
    while (!the_queue.empty()) {
      the_queue.pop();
    }
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
   * Sets the tokenizer to be used when tokenizing the data into tokens.
   *
   * This function is given a std::string of data and is responsible for
   * tokenizing that data into a std::vector<TokenPtr> of data tokens.
   * The default tokenizer splits the data by the ascii return carriage.
   * The user can create their own tokenizer or use one of the default ones.
   *
   * \param tokenizer Function for tokenizing the incoming data.
   *
   * \see serial::TokenizerType, serial::delimeter_tokenizer
   */
  void
  setTokenizer (TokenizerType tokenizer) {
    this->tokenize = tokenizer;
  }

  /*!
   * Sets the number of bytes to be read at a time by the listener.
   *
   * \param chunk_size Number of bytes to be read at a time.
   */
  void
  setChunkSize (size_t chunk_size) {
    this->chunk_size_ = chunk_size;
  }

/***** Start and Stop Listening ******/

  /*!
   * Starts a thread to listen for messages and process them through filters.
   *
   * \param serial_port Pointer to a serial::Serial object that is used to
   * retrieve new data.
   */
  void
  startListening (serial::Serial &serial_port);

  /*!
   * Stops the listening thread and blocks until it completely stops.
   *
   * This function also clears all of the active filters from listenFor and
   * similar functions.
   */
  void
  stopListening ();

/***** Filter Functions ******/

  /*!
   * Creates a filter that calls a callback when the comparator returns true.
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
   * \return boost::shared_ptr<Filter> so you can remove it later.
   *
   * \see SerialListener::removeFilter
   */
  FilterPtr
  createFilter (ComparatorType comparator, DataCallback callback);

  /*!
   * Creates a BlockingFilter which blocks until the comparator returns true.
   *
   * The user provides a comparator, and every time a line is
   * received the comparator is called and the comparator has to evaluate the
   * line and return true if it matches and false if it doesn't.  If it does
   * match, any threads that have called BlockingFilter::wait will be
   * notified.  The BlockingFilter will remove itself when its destructor is
   * called, i.e. when it leaves the scope, so in those cases an explicit call
   * to SerialListener::removeFilter is not needed.
   *
   * \param comparator This is a comparator for detecting if a line matches.
   * The comparartor receives a std::string reference and must return a true
   * if it matches and false if it doesn't.
   *
   * \return BlockingFilterPtr So you can call BlockingFilter::wait on it.
   *
   * \see SerialListener::removeFilter, serial::BlockingFilter,
   * serial::BlockingFilterPtr
   */
  BlockingFilterPtr
  createBlockingFilter (ComparatorType comparator);

  /*!
   * Creates a BlockingFilter blocks until the comparator returns true.
   *
   * The user provides a comparator, and every time a line is
   * received the comparator is called and the comparator has to evaluate the
   * line and return true if it matches and false if it doesn't.  If it does
   * match, any threads that have called BlockingFilter::wait will be
   * notified.  The BlockingFilter will remove itself when its destructor is
   * called, i.e. when it leaves the scope, so in those cases an explicit call
   * to SerialListener::removeFilter is not needed.
   *
   * \param comparator This is a comparator for detecting if a line matches.
   * The comparartor receives a std::string reference and must return a true
   * if it matches and false if it doesn't.
   *
   * \param buffer_size This is the number of tokens to be buffered by the
   * BufferedFilter, defaults to 1024.
   *
   * \return BlockingFilter So you can call BlockingFilter::wait on it.
   *
   * \see SerialListener::removeFilter, serial::BufferedFilter,
   * serial::BufferedFilterPtr
   */
  BufferedFilterPtr
  createBufferedFilter (ComparatorType comparator, size_t buffer_size = 1024);

  /*!
   * Removes a filter by a given FilterPtr.
   *
   * \param filter_ptr A shared pointer to the filter to be removed.
   *
   * \see SerialListener::createFilter
   */
  void
  removeFilter (FilterPtr filter_ptr);

  /*!
   * Removes a BlockingFilter.
   *
   * The BlockingFilter will remove itself if the destructor is called.
   *
   * \param blocking_filter A BlockingFilter to be removed.
   *
   * \see SerialListener::createBlockingFilter
   */
  void
  removeFilter (BlockingFilterPtr blocking_filter);

  /*!
   * Removes a BufferedFilter.
   *
   * The BufferedFilter will remove itself if the destructor is called.
   *
   * \param buffered_filter A BufferedFilter to be removed.
   *
   * \see SerialListener::createBufferedFilter
   */
  void
  removeFilter (BufferedFilterPtr buffered_filter);

  /*!
   * Removes all filters.
   */
  void
  removeAllFilters ();

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
    this->_default_handler = default_handler;
  }

  /*!
   * Sets the function to be called when an exception occurs internally.
   * 
   * This allows you to hook into the exceptions that occur in threads inside 
   * the serial listener library.
   * 
   * \param exception_handler A function pointer to the callback to handle new 
   * interal exceptions.
   * 
   * \see serial::ExceptionCallback
   */
  void
  setExceptionHandler (ExceptionCallback exception_handler) {
    this->handle_exc = exception_handler;
  }

/***** Static Functions ******/

  /*!
   * Sleeps for a given number of milliseconds.
   *
   * \param milliseconds number of milliseconds to sleep.
   */
  static void
  sleep (long milliseconds) {
    boost::int64_t ms(milliseconds);
    boost::this_thread::sleep(boost::posix_time::milliseconds(ms));
  }

  /*!
   * This returns a tokenizer that splits on a given delimeter.
   *
   * The delimeter is passed into the function and a TokenizerType is returned
   * that can be passed to SerialListener::setTokenizer.
   *
   * Example:
   * <pre>
   *   my_listener.setTokenizer(SerialListener::delimeter_tokenizer("\r"));
   * <\pre>
   *
   * \param delimeter A std::string that is used as a delimeter when
   * tokenizing data.
   *
   * \return TokenizerType A tokenizer function type that can be passed to
   * SerialListener::setTokenizer.
   *
   * \see SerialListener::setTokenizer, serial::TokenizerType
   */
  static TokenizerType
  delimeter_tokenizer (std::string delimeter) {
    return boost::bind(&SerialListener::_delimeter_tokenizer,
                       _1, _2, delimeter);
  }

  /*!
   * This returns a comparator that matches only the exact string given.
   *
   * This can be used with listenFor or listenForOnce:
   *
   * Example:
   * <pre>
   *   my_listener.listenFor(SerialListener::exactly("my_string"),
   *                         my_callback);
   * <\pre>
   *
   * \param exact_str A std::string that is used as the exact string to match
   * when comparing tokens for matching.
   *
   * \return ComparatorType A comparator function type that can be passed to
   * SerialListener::listenFor or SerialListener::listenForOnce.
   *
   * \see SerialListener::listenFor, SerialListener::listenForOnce,
   * serial::ComparatorType
   */
  static ComparatorType
  exactly (std::string exact_str) {
    return boost::bind(&SerialListener::_exactly, _1, exact_str);
  }

  /*!
   * This returns a comparator that looks for a given prefix.
   *
   * This can be used with listenFor or listenForOnce:
   *
   * Example:
   * <pre>
   *   my_listener.listenFor(SerialListener::startsWith("V="), my_callback);
   * <\pre>
   *
   * \param prefix A std::string that is used as the prefix string to match
   * when comparing tokens for matching.
   *
   * \return ComparatorType A comparator function type that can be passed to
   * SerialListener::listenFor or SerialListener::listenForOnce.
   *
   * \see SerialListener::listenFor, SerialListener::listenForOnce,
   * serial::ComparatorType
   */
  static ComparatorType
  startsWith (std::string prefix) {
    return boost::bind(&SerialListener::_startsWith, _1, prefix);
  }

  /*!
   * This returns a comparator that looks for a given postfix.
   *
   * This can be used with listenFor or listenForOnce:
   *
   * Example:
   * <pre>
   *   my_listener.listenFor(SerialListener::endsWith(";"), my_callback);
   * <\pre>
   *
   * \param postfix A std::string that is used as the postfix string to match
   * when comparing tokens for matching.
   *
   * \return ComparatorType A comparator function type that can be passed to
   * SerialListener::listenFor or SerialListener::listenForOnce.
   *
   * \see SerialListener::listenFor, SerialListener::listenForOnce,
   * serial::ComparatorType
   */
  static ComparatorType
  endsWith (std::string postfix) {
    return boost::bind(&SerialListener::_endsWith, _1, postfix);
  }

  /*!
   * This returns a comparator that looks for a given substring in the token.
   *
   * This can be used with listenFor or listenForOnce:
   *
   * Example:
   * <pre>
   *   my_listener.listenFor(SerialListener::contains("some string"),
   *                         my_callback);
   * <\pre>
   *
   * \param substr A std::string that is used as the search substring to match
   * when comparing tokens for matching.
   *
   * \return ComparatorType A comparator function type that can be passed to
   * SerialListener::listenFor or SerialListener::listenForOnce.
   *
   * \see SerialListener::listenFor, SerialListener::listenForOnce,
   * serial::ComparatorType
   */
  static ComparatorType
  contains (std::string substr) {
    return boost::bind(_contains, _1, substr);
  }

private:
  // Disable copy constructors
  SerialListener(const SerialListener&);
  void operator=(const SerialListener&);
  const SerialListener& operator=(SerialListener);
  // delimeter tokenizer function
  static void
  _delimeter_tokenizer (const std::string &data,
                        std::vector<TokenPtr> &tokens,
                        std::string delimeter)
  {
    typedef std::vector<std::string> find_vector_type;
    find_vector_type t;
    boost::split(t, data, boost::is_any_of(delimeter));
    for (find_vector_type::iterator it = t.begin(); it != t.end(); it++)
      tokens.push_back(TokenPtr( new std::string(*it) ));
  }
  // exact comparator function
  static bool
  _exactly (const std::string& token, std::string exact_str) {
#if SERIAL_LISTENER_DEBUG
    std::cerr << "In exactly callback(" << token.length() << "): ";
    std::cerr << token << " == " << exact_str << ": ";
    if (token == exact_str)
      std::cerr << "True";
    else
      std::cerr << "False";
    std::cerr << std::endl;
#endif
    return token == exact_str;
  }
  // startswith comparator function
  static bool
  _startsWith (const std::string& token, std::string prefix) {
#if SERIAL_LISTENER_DEBUG
    std::cerr << "In startsWith callback(" << token.length() << "): ";
    std::cerr << token << " starts with " << prefix;
    std::cerr << "?: ";
    if (token.substr(0,prefix.length()) == prefix)
      std::cerr << "True";
    else
      std::cerr << "False";
    std::cerr << std::endl;
#endif
    return token.substr(0,prefix.length()) == prefix;
  }
  // endswith comparator function
  static bool
  _endsWith (const std::string& token, std::string postfix) {
#if SERIAL_LISTENER_DEBUG
    std::cerr << "In endsWith callback(";
    std::cerr << token.length();
    std::cerr << "): " << token;
    std::cerr << " ends with " << postfix << "?: ";
    if (token.substr(token.length()-postfix.length()) == postfix)
      std::cerr << "True";
    else
      std::cerr << "False";
    std::cerr << std::endl;
#endif
    return token.substr(token.length()-postfix.length()) == postfix;
  }
  // contains comparator function
  static bool
  _contains (const std::string& token, std::string substr) {
    return token.find(substr) != std::string::npos;
  }

  // Gets some data from the serial port
  void readSomeData (std::string &temp, size_t this_many) {
    // Make sure there is a serial port
    if (this->serial_port_ == NULL) {
      this->handle_exc(SerialListenerException("Invalid serial port."));
    }
    // Make sure the serial port is open
    if (!this->serial_port_->isOpen()) {
      this->handle_exc(SerialListenerException("Serial port not open."));
    }
    temp = this->serial_port_->read(this_many);
  }
  // Runs the new_tokens through all the filters
  void filter (std::vector<TokenPtr> &tokens);
  // Function that loops while listening is true
  void listen ();
  // Target of callback thread
  void callback ();
  // Determines how much to read on each loop of listen
  size_t determineAmountToRead ();

  // Tokenizer
  TokenizerType tokenize;

  // Exception handler
  ExceptionCallback handle_exc;

  // Default handler
  FilterPtr default_filter;
  DataCallback _default_handler;
  ComparatorType default_comparator;
  void default_handler(const std::string &token);

  // Persistent listening variables
  bool listening;
  char serial_port_padding[7];
  serial::Serial * serial_port_;
  boost::thread listen_thread;
  std::string data_buffer;
  size_t chunk_size_;

  // Callback related variables
  // filter id, token
  // filter id == 0 is going to be default handled
  ConcurrentQueue<std::pair<FilterPtr,TokenPtr> >
  callback_queue;
  boost::thread callback_thread;

  // Mutex for locking use of filters
  boost::mutex filter_mux;
  // vector of filter ids
  std::vector<FilterPtr> filters;

};

/*!
 * This is the a filter that provides a wait function for blocking until a
 * match is found.
 *
 * This should probably not be created manually, but instead should be
 * constructed using SerialListener::createBlockingFilter(ComparatorType)
 * function which returns a BlockingFilter instance.
 *
 * \see serial::SerialListener::ComparatorType,
 * serial::SerialListener::createBlockingFilter
 */
class BlockingFilter
{
public:
  BlockingFilter (ComparatorType comparator, SerialListener &listener) {
    this->listener_ = &listener;
    DataCallback cb = boost::bind(&BlockingFilter::callback, this, _1);
    this->filter_ptr = this->listener_->createFilter(comparator, cb);
  }

  virtual ~BlockingFilter () {
    this->listener_->removeFilter(filter_ptr);
    this->result = "";
    this->cond.notify_all();
  }

  /*!
   * Waits a given number of milliseconds or until a token is matched.  If a
   * token is matched it is returned, otherwise an empty string is returned.
   *
   * \param ms Time in milliseconds to wait on a new token.
   *
   * \return std::string token that was matched or "" if none were matched.
   */
 std::string wait(long ms) {
    this->result = "";
    boost::unique_lock<boost::mutex> lock(this->mutex);
    this->cond.timed_wait(lock, boost::posix_time::milliseconds(ms));
    return this->result;
  }

  FilterPtr filter_ptr;

  void callback(const std::string& token) {
#if SERIAL_LISTENER_DEBUG
    std::cerr << "In BlockingFilter callback(" << token.length() << "): ";
    std::cerr << token << std::endl;
#endif
    this->cond.notify_all();
    this->result = token;
  }

private:
  SerialListener * listener_;
  boost::condition_variable cond;
  boost::mutex mutex;
  std::string result;

};

/*!
 * This is the a filter that provides a wait function for blocking until a
 * match is found.  It will also buffer up to a given buffer size of tokens so
 * that they can be counted or accessed after they are matched by the filter.
 *
 * This should probably not be created manually, but instead should be
 * constructed using SerialListener::createBufferedFilter(ComparatorType)
 * function which returns a BufferedFilter instance.
 *
 * The internal buffer is a circular queue buffer, so when the buffer is full,
 * the oldest token is dropped and the new one is added.  Additionally, when
 * wait is a called the oldest available token is returned.
 *
 * \see serial::SerialListener::ComparatorType,
 * serial::SerialListener::createBufferedFilter
 */
class BufferedFilter
{
public:
  BufferedFilter (ComparatorType comparator, size_t buffer_size,
                  SerialListener &listener)
  : buffer_size_(buffer_size)
  {
    this->listener_ = &listener;
    DataCallback cb = boost::bind(&BufferedFilter::callback, this, _1);
    this->filter_ptr = this->listener_->createFilter(comparator, cb);
  }

  virtual ~BufferedFilter () {
    this->listener_->removeFilter(filter_ptr);
    this->queue.clear();
    this->result = "";
  }

  /*!
   * Waits a given number of milliseconds or until a matched token is
   * available in the buffer.  If a token is matched it is returned, otherwise
   * an empty string is returned.
   *
   * \param ms Time in milliseconds to wait on a new token.  If ms is set to 0
   * then it will try to get a new token if one is available but will not
   * block.
   *
   * \return std::string token that was matched or "" if none were matched.
   */
  std::string wait(long ms) {
    if (ms == 0) {
      if (!this->queue.try_pop(this->result)) {
        this->result = "";
      }
    } else {
      if (!this->queue.timed_wait_and_pop(this->result, ms)) {
        this->result = "";
      }
    }
    return result;
  }

  /*!
   * Clears the buffer of any tokens.
   */
  void clear() {
    queue.clear();
  }

  /*!
   * Returns the number of tokens waiting in the buffer.
   */
  size_t count() {
    return queue.size();
  }

  /*!
   * Returns the capacity of the buffer.
   */
  size_t capacity() {
    return buffer_size_;
  }

  FilterPtr filter_ptr;

  void callback(const std::string &token) {
#if SERIAL_LISTENER_DEBUG
    std::cerr << "In BufferedFilter callback(" << token.length() << "): ";
    std::cerr << token << std::endl;
#endif
    std::string throw_away;
    if (this->queue.size() == this->buffer_size_) {
      this->queue.wait_and_pop(throw_away);
    }
    this->queue.push(token);
  }

private:
  size_t buffer_size_;
  SerialListener * listener_;
  ConcurrentQueue<std::string> queue;
  std::string result;

};

} // namespace serial

#endif // SERIAL_LISTENER_H
