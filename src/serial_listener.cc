#include "serial/serial_listener.h"

/***** Inline Functions *****/

inline void defaultWarningCallback(const std::string& msg) {
  std::cout << "SerialListener Warning: " << msg << std::endl;
}

inline void defaultDebugCallback(const std::string& msg) {
  std::cout << "SerialListener Debug: " << msg << std::endl;
}

inline void defaultInfoCallback(const std::string& msg) {
  std::cout << "SerialListener Info: " << msg << std::endl;
}

inline void defaultExceptionCallback(const std::exception &error) {
  std::cerr << "SerialListener Unhandled Exception: " << error.what();
  std::cerr << std::endl;
  throw(error);
}

using namespace serial;

/***** Listener Class Functions *****/

SerialListener::SerialListener() : listening(false) {
  // Set default callbacks
  this->handle_exc = defaultExceptionCallback;
  this->info = defaultInfoCallback;
  this->debug = defaultDebugCallback;
  this->warn = defaultWarningCallback;
  this->default_handler = NULL;
  
  // Set default tokenizer
  this->setTokenizer(delimeter_tokenizer("\r"));
  
  // Set default ttl
  this->setTimeToLive();
}

SerialListener::~SerialListener() {
  if (this->listening) {
    this->stopListening();
  }
}

void
SerialListener::callback() {
  try {
    std::pair<uuid_type,bool> pair;
    DataCallback _callback;
    while (this->listening) {
      if (this->callback_queue.timed_wait_and_pop(pair, 10)) {
        if (this->listening) {
          std::cout << "After pop (" << pair.second << "): ";
          std::cout << this->tokens[pair.first] << std::endl;
          try {
            // If default handler
            if (pair.second) {
              if (this->default_handler)
                this->default_handler(this->tokens[pair.first]);
            // Else use provided callback
            } else {
              // Grab the callback as to not hold the mutex while executing
              {
                boost::mutex::scoped_lock l(callback_mux);
                _callback = this->callbacks[pair.first];
              }
              // Execute callback
              _callback(this->tokens[pair.first]);
            }
          } catch (std::exception &e) {
            this->handle_exc(e);
          }// try callback
        } // if listening
        // Erase the used and executed callback
        this->eraseToken(pair.first);
      } // if popped
    } // while (this->listening)
  } catch (std::exception &e) {
    this->handle_exc(SerialListenerException(e.what()));
  }
}

void
SerialListener::setTimeToLive(size_t ms) {
  using namespace boost::posix_time;
  this->ttl = time_duration(milliseconds(ms));
}

void
SerialListener::startListening(Serial * serial_port) {
  if (this->listening) {
    throw(SerialListenerException("Already listening."));
    return;
  }
  this->listening = true;
  
  this->serial_port = serial_port;
  if (!this->serial_port->isOpen()) {
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

  callback_queue.clear();
  this->data_buffer = "";
  this->tokens.clear();
  this->ttls.clear();
  this->serial_port = NULL;

  // Delete all the filters
  this->stopListeningForAll();
}

void
SerialListener::stopListeningForAll() {
  boost::mutex::scoped_lock l(filter_mux);
  boost::mutex::scoped_lock l2(callback_mux);
  filters.clear();
  comparators.clear();
  condition_vars.clear();
  callbacks.clear();
}

size_t
SerialListener::determineAmountToRead() {
  // TODO: Make a more intelligent method based on the length of the things 
  //  filters are looking for.  i.e.: if the filter is looking for 'V=XX\r' 
  //  make the read amount at least 5.
  return 5;
}

void
SerialListener::readSomeData(std::string &temp, size_t this_many) {
  // Make sure there is a serial port
  if (this->serial_port == NULL) {
    this->handle_exc(SerialListenerException("Invalid serial port."));
  }
  // Make sure the serial port is open
  if (!this->serial_port->isOpen()) {
    this->handle_exc(SerialListenerException("Serial port not open."));
  }
  temp = this->serial_port->read(this_many);
}

void
SerialListener::addNewTokens(std::vector<std::string> &new_tokens,
                             std::vector<uuid_type> new_uuids,
                             std::string &left_overs)
{
  std::cout << "Inside SerialListener::addNewTokens:" << std::endl;
  // Iterate through new tokens and add times to them
  std::vector<std::string>::iterator it_new;
  for (it_new=new_tokens.begin(); it_new != new_tokens.end(); it_new++) {
    std::cout << "  Token (" << (*it_new).length() << "): " << (*it_new) << std::endl;
    // The last token needs to be put back in the buffer always:
    //  (in the case that the delimeter is \r)...
    //  In the case that the string ends with \r the last element will be 
    //  empty ("").  In the case that it does not the last element will be 
    //  what is left over from the next message that hasn't sent 
    //  everything.  Ex.: "?$1E\r" -> ["?$1E", ""] and 
    //  "?$1E\r$1E=Robo" -> ["?$1E","$1E=Robo"]
    if (it_new == new_tokens.end()-1) {
      left_overs = (*it_new);
      continue;
    }
    // Create a new uuid
    uuid_type uuid = random_generator();
    // Put the new uuid in the list of new uuids
    new_uuids.push_back(uuid);
    // Create a uuid, token pair
    std::pair<const uuid_type,std::string> token_pair(uuid,(*it_new));
    // Create a uuid, ttl pair
    using namespace boost::posix_time;
    std::pair<const uuid_type,ptime> 
     ttl_pair(uuid,ptime(microsec_clock::local_time()));
    // Insert the new pairs
    {
      boost::mutex::scoped_lock l(token_mux);
      this->tokens.insert(token_pair);
      ttls.insert(ttl_pair);
    }
  } // for (it_new=new_tokens.begin(); it_new != new_tokens.end(); it_new++)
}

void
SerialListener::eraseToken(uuid_type &uuid) {
  boost::mutex::scoped_lock l(token_mux);
  this->tokens.erase(uuid);
  this->ttls.erase(uuid);
}

void
SerialListener::eraseTokens(std::vector<uuid_type> &uuids) {
  std::vector<uuid_type>::iterator it;
  for (it=uuids.begin(); it != uuids.end(); it++) {
    this->eraseToken((*it));
  }
}

// TODO: Investigate possible race condition where the filter processing takes 
//  longer than the ttl
void
SerialListener::filterNewTokens(std::vector<uuid_type> new_uuids) {
  // Iterate through the filters, checking each against new tokens
  boost::mutex::scoped_lock l(filter_mux);
  std::map<const uuid_type,filter_type::FilterType>::iterator it;
  for (it=filters.begin(); it!=filters.end(); it++) {
    this->filter((*it).first, new_uuids);
  } // for (it=filters.begin(); it!=filters.end(); it++)
  // Get the filter lock
  boost::mutex::scoped_lock l(filter_mux);
  std::vector<uuid_type> to_be_erased;
  // Iterate through the tokens checking for a match
  std::vector<uuid_type>::iterator it_uuids;
  for (it_uuids=new_uuids.begin(); it_uuids!=new_uuids.end(); it_uuids++) {
    bool matched = false;
    uuid_type uuid = (*it_uuids);
    // If the line is empty, continue
    if (tokens[uuid].length() == 0) {
      continue;
    }
    // Iterate through each filter
    std::map<const uuid_type,filter_type::FilterType>::iterator it;
    for (it=filters.begin(); it!=filters.end(); it++) {
      // If comparator matches line
      if (comparators[(*it).first](tokens[uuid])) {
        // If non-blocking run the callback
        if ((*it).second == filter_type::nonblocking) {
          callback_queue.push(std::pair<uuid_type,bool>(uuid,false));
        // If blocking then notify the waiting call to continue
        } else if ((*it).second == filter_type::blocking) {
          condition_vars[(*it).first]->notify_all();
          to_be_erased.push_back(uuid);
        }
        matched = true;
        break; // It matched, continue to next line
      }
    } // for(it=filters.begin(); it!=filters.end(); it++)
  } // for(it_lines=lines.begin(); it_lines!=lines.end(); it_lines++)
  // Remove any lines that need to be erased
  //  (this must be done outside the iterator to prevent problems incrementing
  //   the iterator)
  this->eraseTokens(to_be_erased);
}

void
filter(uuid_type filter_uuid, std::vector<uuid_type> token_uuids) {
  std::vector<uuid_type> to_be_erased;
  // Iterate through the token uuids and run each against the filter
  std::vector<uuid_type>::iterator it_uuids;
  for (it_uuids=new_uuids.begin(); it_uuids!=new_uuids.end(); it_uuids++) {
    
  }
  // Remove any lines that need to be erased
  //  (this must be done outside the iterator to prevent problems incrementing
  //   the iterator)
  this->eraseTokens(to_be_erased);
}

void
SerialListener::pruneTokens() {
  // Iterate through the buffered tokens
  std::vector<uuid_type> to_be_erased;
  std::map<const uuid_type,std::string>::iterator it;

  {
    boost::mutex::scoped_lock l(token_mux);
    for (it = this->tokens.begin(); it != this->tokens.end(); it++) {
      uuid_type uuid = (*it).first;
      using namespace boost::posix_time;
      // If the current time - the creation time is greater than the ttl, 
      //  then prune it
      if (ptime(microsec_clock::local_time())-this->ttls[uuid] > this->ttl) {
        std::cout << "Pruning (" << this->tokens[uuid].length();
        std::cout << "): " << this->tokens[uuid] << std::endl;
        // If there is a default handler pass it on
        if (this->default_handler) {
          boost::mutex::scoped_lock l(callback_mux);
          callback_queue.push(std::pair<uuid_type,bool>(uuid,true));
        } else {
          // Otherwise delete it
          to_be_erased.push_back(uuid);
        }
      }
    }
  }
  // Remove any lines that need to be erased
  //  (this must be done outside the iterator to prevent problems incrementing
  //   the iterator)
  this->eraseTokens(to_be_erased);
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
        std::vector<std::string> new_tokens;
        this->tokenize(this->data_buffer, new_tokens);
        // Add the new tokens to the new token buffer, get a list of new uuids 
        //  to filter once, and put left_overs in the data buffer.
        std::vector<uuid_type> new_uuids;
        this->addNewTokens(new_tokens, new_uuids, this->data_buffer);
        // Run the new tokens through existing filters
        this->filterNewTokens(new_uuids);
      }
      // Look for old data and pass to the default handler or delete
      this->pruneTokens();
      // Done parsing lines and buffer should now be set to the left overs
    } // while (this->listening)
  } catch (std::exception &e) {
    this->handle_exc(SerialListenerException(e.what()));
  }
}

bool
SerialListener::listenForOnceComparator(std::string token) {
  if (token == current_listen_for_one_target)
    return true;
  return false;
}

bool
SerialListener::listenForStringOnce(std::string token, size_t milliseconds) {
  boost::condition_variable cond;
  boost::mutex mut;
  current_listen_for_one_target = token;

  // Create blocking filter
  uuid_type uuid = random_generator();
  std::pair<const uuid_type,filter_type::FilterType>
   filter_pair(uuid, filter_type::blocking);
  std::pair<const uuid_type,ComparatorType>
   comparator_pair(uuid,
    boost::bind(&SerialListener::listenForOnceComparator, this, _1));
  std::pair<const uuid_type,boost::condition_variable*>
   condition_pair(uuid, &cond);
  {
    boost::mutex::scoped_lock l(filter_mux);
    filters.insert(filter_pair);
    comparators.insert(comparator_pair);
    condition_vars.insert(condition_pair);
  }

  this->processNewFilter(uuid);

  bool result = false;

  // Wait
  boost::unique_lock<boost::mutex> lock(mut);
  if (cond.timed_wait(lock, boost::posix_time::milliseconds(milliseconds)))
    result = true;

  // Destroy the filter
  {
    boost::mutex::scoped_lock l(filter_mux);
    filters.erase(uuid);
    comparators.erase(uuid);
    condition_vars.erase(uuid);
  }

  return result;
}

uuid_type
SerialListener::listenFor(ComparatorType comparator, DataCallback callback)
{
  // Create Filter
  uuid_type uuid = random_generator();
  std::pair<const uuid_type,filter_type::FilterType>
   filter_pair(uuid, filter_type::nonblocking);
  std::pair<const uuid_type,ComparatorType>
   comparator_pair(uuid, comparator);
  std::pair<const uuid_type,DataCallback>
   callback_pair(uuid, callback);

  {
    boost::mutex::scoped_lock l(filter_mux);
    filters.insert(filter_pair);
    comparators.insert(comparator_pair);
  }
  {
    boost::mutex::scoped_lock l(callback_mux);
    callbacks.insert(callback_pair);
  }

  return uuid;
}

void
SerialListener::stopListeningFor(uuid_type filter_uuid) {
  // Delete filter
  boost::mutex::scoped_lock l(filter_mux);
  filters.erase(filter_uuid);
  comparators.erase(filter_uuid);
  callbacks.erase(filter_uuid);
}

TokenizerType
SerialListener::delimeter_tokenizer (std::string delimeter) {
  return boost::bind(&SerialListener::_delimeter_tokenizer,
                     _1, _2, delimeter);
}

void
SerialListener::_delimeter_tokenizer (std::string &data,
                                      std::vector<std::string> &tokens,
                                      std::string delimeter)
{
  boost::split(tokens, data, boost::is_any_of(delimeter));
}

