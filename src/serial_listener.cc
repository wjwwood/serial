/* Copyright 2012 William Woodall and John Harrison */

#include "serial/serial_listener.h"

/***** Inline Functions *****/

inline void defaultExceptionCallback(const std::exception &error) {
  std::cerr << "SerialListener Unhandled Exception: " << error.what();
  std::cerr << std::endl;
}

inline bool defaultComparator(const std::string &token) {
  return token == token;
}

using namespace serial;

/***** Listener Class Functions *****/

void
SerialListener::default_handler(const std::string &token) {
  if (this->_default_handler)
    this->_default_handler(token);
}

SerialListener::SerialListener() : listening(false), chunk_size_(5) {
  // Set default callbacks
  this->handle_exc = defaultExceptionCallback;

  // Default handler stuff
  this->_default_handler = NULL;
  this->default_comparator = defaultComparator;
  DataCallback tmp = boost::bind(&SerialListener::default_handler, this, _1);
  this->default_filter = FilterPtr(new Filter(default_comparator, tmp));

  // Set default tokenizer
  this->setTokenizer(delimeter_tokenizer("\r"));
}

SerialListener::~SerialListener() {
  if (this->listening) {
    this->stopListening();
  }
}

void
SerialListener::callback() {
  try {
    // <filter id, token>
    std::pair<FilterPtr,TokenPtr> pair;
    while (this->listening) {
      if (this->callback_queue.timed_wait_and_pop(pair, 10)) {
        if (this->listening) {
          try {
            if (pair.first != NULL && pair.second != NULL) {
              pair.first->callback_((*pair.second));
            }
          } catch (std::exception &e) {
            this->handle_exc(e);
          }// try callback
        } // if listening
      } // if popped
    } // while (this->listening)
  } catch (std::exception &e) {
    this->handle_exc(SerialListenerException(e.what()));
  }
}

void
SerialListener::startListening(Serial &serial_port) {
  if (this->listening) {
    throw(SerialListenerException("Already listening."));
    return;
  }
  this->listening = true;
  
  this->serial_port_ = &serial_port;
  if (!this->serial_port_->isOpen()) {
    throw(SerialListenerException("Serial port not open."));
    return;
  }
  
  listen_thread = boost::thread(boost::bind(&SerialListener::listen, this));
  
  // Start the callback thread
  callback_thread =
   boost::thread(boost::bind(&SerialListener::callback, this));
}

void
SerialListener::stopListening() {
  // Stop listening and clear buffers
  listening = false;

  listen_thread.join();
  callback_thread.join();

  this->data_buffer = "";
  this->serial_port_ = NULL;
}

size_t
SerialListener::determineAmountToRead() {
  // TODO: Make a more intelligent method based on the length of the things 
  //  filters are looking for.  e.g.: if the filter is looking for 'V=XX\r' 
  //  make the read amount at least 5.
  return this->chunk_size_;
}

void
SerialListener::filter(std::vector<TokenPtr> &tokens) {
  // Lock the filters while filtering
  boost::mutex::scoped_lock lock(filter_mux);
  // Iterate through each new token and filter them
  std::vector<TokenPtr>::iterator it;
  for (it=tokens.begin(); it!=tokens.end(); it++) {
    TokenPtr token = (*it);
    // If it is empty then pass it
    if (token->empty()) {
      continue;
    }
    bool matched = false;
    // Iterate through each filter
    std::vector<FilterPtr>::iterator itt;
    for (itt=filters.begin(); itt!=filters.end(); itt++) {
      FilterPtr filter = (*itt);
      if (filter->comparator_((*token))) {
        callback_queue.push(std::make_pair(filter,token));
        matched = true;
        break;
      }
    } // for (itt=filters.begin(); itt!=filters.end(); itt++)
    // If matched is false then send it to the default handler
    if (!matched) {
      callback_queue.push(std::make_pair(default_filter,token));
    }
  } // for (it=tokens.begin(); it!=tokens.end(); it++)
}

void
SerialListener::listen() {
  try {
    while (this->listening) {
      // Read some data
      std::string temp;
      this->readSomeData(temp, determineAmountToRead());
      // If nothing was read then we
      //  don't need to iterate through the filters
      if (temp.length() != 0) {
        // Add the new data to the buffer
        this->data_buffer += temp;
        // Call the tokenizer on the updated buffer
        std::vector<TokenPtr> new_tokens;
        this->tokenize(this->data_buffer, new_tokens);
        // Put the last token back in the data buffer
        this->data_buffer = (*new_tokens.back());
        new_tokens.pop_back();
        // Run the new tokens through existing filters
        this->filter(new_tokens);
      }
      // Done parsing lines and buffer should now be set to the left overs
    } // while (this->listening)
  } catch (std::exception &e) {
    this->handle_exc(SerialListenerException(e.what()));
  }
}

/***** Filter Functions *****/

FilterPtr
SerialListener::createFilter(ComparatorType comparator, DataCallback callback)
{
  FilterPtr filter_ptr(new Filter(comparator, callback));

  boost::mutex::scoped_lock l(filter_mux);
  this->filters.push_back(filter_ptr);

  return filter_ptr;
}

BlockingFilterPtr
SerialListener::createBlockingFilter(ComparatorType comparator) {
  return BlockingFilterPtr(
    new BlockingFilter(comparator, (*this)));
}

BufferedFilterPtr
SerialListener::createBufferedFilter(ComparatorType comparator,
                                     size_t buffer_size)
{
  return BufferedFilterPtr(
    new BufferedFilter(comparator, buffer_size, (*this)));
}

void
SerialListener::removeFilter(FilterPtr filter_ptr) {
  boost::mutex::scoped_lock l(filter_mux);
  filters.erase(std::find(filters.begin(),filters.end(),filter_ptr));
}

void
SerialListener::removeFilter(BlockingFilterPtr blocking_filter) {
  this->removeFilter(blocking_filter->filter_ptr);
}

void
SerialListener::removeFilter(BufferedFilterPtr buffered_filter) {
  this->removeFilter(buffered_filter->filter_ptr);
}

void
SerialListener::removeAllFilters() {
  boost::mutex::scoped_lock l(filter_mux);
  filters.clear();
  callback_queue.clear();
}

