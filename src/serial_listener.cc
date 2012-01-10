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
    // <filter uuid, token uuid>
    std::pair<uuid_type,uuid_type> pair;
    DataCallback _callback;
    while (this->listening) {
      if (this->callback_queue.timed_wait_and_pop(pair, 10)) {
        if (this->listening) {
          try {
            // If default handler
            if (pair.first.is_nil()) {
              if (this->default_handler)
                this->default_handler(this->tokens[pair.second]);
            // Else use provided callback
            } else {
              bool go = false;
              // Grab the callback as to not hold the mutex while executing
              {
                boost::mutex::scoped_lock l(callback_mux);
                // Make sure the filter hasn't been removed
                if ((go = (this->callbacks.count(pair.first) > 0)))
                  _callback = this->callbacks[pair.first];
              }
              // Execute callback
              if (go)
                _callback(this->tokens[pair.second]);
            }
          } catch (std::exception &e) {
            this->handle_exc(e);
          }// try callback
        } // if listening
        // Erase the used and executed callback
        this->eraseToken(pair.second);
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
SerialListener::startListening(Serial &serial_port) {
  if (this->listening) {
    throw(SerialListenerException("Already listening."));
    return;
  }
  this->listening = true;
  
  this->serial_port = &serial_port;
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
  filters.clear();
  comparators.clear();
  boost::mutex::scoped_lock l2(callback_mux);
  callbacks.clear();
  callback_queue.clear();
}

size_t
SerialListener::determineAmountToRead() {
  // TODO: Make a more intelligent method based on the length of the things 
  //  filters are looking for.  i.e.: if the filter is looking for 'V=XX\r' 
  //  make the read amount at least 5.
  return 1024;
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
                             std::vector<uuid_type> &new_uuids,
                             std::string &left_overs)
{
  std::vector<std::string>::iterator it_new;
  for (it_new=new_tokens.begin(); it_new != new_tokens.end(); it_new++) {
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
    // If the token is empty ignore it
    if ((*it_new).length() == 0)
      continue;
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
  std::vector<std::pair<uuid_type,uuid_type> > tbd;
  boost::mutex::scoped_lock l(filter_mux);
  std::vector<uuid_type>::iterator it;
  for (it=filters.begin(); it!=filters.end(); it++) {
    std::vector<std::pair<uuid_type,uuid_type> > temp =
      this->filter((*it), new_uuids);
    if (temp.size() > 0)
      tbd.insert(tbd.end(), temp.begin(), temp.end());
  } // for (it=filters.begin(); it!=filters.end(); it++)
  // Dispatch
  std::vector<std::pair<uuid_type,uuid_type> >::iterator it_tbd;
  for (it_tbd = tbd.begin(); it_tbd != tbd.end(); it_tbd++) {
    callback_queue.push((*it_tbd));
  }
}

// <filter,token>
std::vector<std::pair<uuid_type,uuid_type> >
SerialListener::filter(uuid_type filter_uuid,
                       std::vector<uuid_type> &token_uuids)
{
  std::vector<uuid_type> to_be_erased;
  std::vector<std::pair<uuid_type,uuid_type> > to_be_dispatched;
  // Iterate through the token uuids and run each against the filter
  std::vector<uuid_type>::iterator it;
  for (it=token_uuids.begin(); it!=token_uuids.end(); it++) {
    bool matched = false;
    uuid_type token_uuid = (*it);
    if (this->comparators[filter_uuid](this->tokens[token_uuid])) {
      matched = true;
      to_be_dispatched.push_back(std::make_pair(filter_uuid,token_uuid));
    }
  }
  return to_be_dispatched;
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
        // If there is a default handler pass it on
        if (this->default_handler) {
          boost::mutex::scoped_lock l(callback_mux);
          callback_queue.push(std::make_pair(nil_generator(),uuid));
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
SerialListener::listenForStringOnce(std::string token, size_t milliseconds) {
  return this->listenForOnce(exactly(token), milliseconds);
}

void
SerialListener::notifyListenForOnce(shared_cond_var_ptr_t cond_ptr) {
  cond_ptr->notify_all();
}

bool
SerialListener::listenForOnce(ComparatorType comparator, size_t ms)
{
  shared_cond_var_ptr_t cond_ptr(new boost::condition_variable());
  boost::mutex mut;

  // Create blocking filter
  const uuid_type uuid = random_generator();
  {
    boost::mutex::scoped_lock l(filter_mux);
    filters.push_back(uuid);
    comparators.insert(std::make_pair(uuid,comparator));
  }
  {
    boost::mutex::scoped_lock l(callback_mux);
    callbacks.insert(std::make_pair(uuid,
      boost::bind(&SerialListener::notifyListenForOnce, this, cond_ptr)));
  }

  // Run this filter through all tokens onces
  std::vector<uuid_type> token_uuids;
  std::map<const uuid_type,std::string>::iterator it;
  for (it = tokens.begin(); it != tokens.end(); it++)
    token_uuids.push_back((*it).first);
  std::vector<std::pair<uuid_type,uuid_type> > pairs =
    this->filter(uuid, token_uuids);

  // If there is at least one
  if (pairs.size() > 0) {
    // If there is more than one find the oldest
    size_t index = 0;
    if (pairs.size() > 1) {
      using namespace boost::posix_time;
      ptime oldest_time = ttls[pairs[index].second];
      size_t i = 0;
      std::vector<std::pair<uuid_type,uuid_type> >::iterator it;
      for (it = pairs.begin(); it != pairs.end(); it++) {
        if (ttls[(*it).second] < oldest_time) {
          oldest_time = ttls[(*it).second];
          index = i;
        }
        i++;
      }
    }
    // Either way put the final index into the callback queue
    callback_queue.push(pairs[index]);
  }

  bool result = false;

  // Wait
  boost::unique_lock<boost::mutex> lock(mut);
  using namespace boost::posix_time;
  if (cond_ptr->timed_wait(lock, milliseconds(ms)))
    result = true;

  // Destroy the filter
  {
    boost::mutex::scoped_lock l(filter_mux);
    filters.erase(std::find(filters.begin(),filters.end(),uuid));
    comparators.erase(uuid);
  }
  {
    boost::mutex::scoped_lock l(callback_mux);
    callbacks.erase(uuid);
  }

  return result;
}

uuid_type
SerialListener::listenFor(ComparatorType comparator, DataCallback callback)
{
  // Create Filter
  uuid_type uuid = random_generator();
  std::pair<const uuid_type,ComparatorType>
   comparator_pair(uuid, comparator);
  std::pair<const uuid_type,DataCallback>
   callback_pair(uuid, callback);

  {
    boost::mutex::scoped_lock l(filter_mux);
    filters.push_back(uuid);
    comparators.insert(comparator_pair);
  }
  {
    boost::mutex::scoped_lock l(callback_mux);
    callbacks.insert(callback_pair);
  }

  // Run this filter through all tokens onces
  std::vector<uuid_type> token_uuids;
  std::map<const uuid_type,std::string>::iterator it;
  for (it = tokens.begin(); it != tokens.end(); it++)
    token_uuids.push_back((*it).first);
  std::vector<std::pair<uuid_type,uuid_type> > pairs =
    this->filter(uuid, token_uuids);

  // Dispatch
  std::vector<std::pair<uuid_type,uuid_type> >::iterator it_cb;
  for (it_cb = pairs.begin(); it_cb != pairs.end(); it_cb++) {
    callback_queue.push((*it_cb));
  }

  return uuid;
}

void
SerialListener::stopListeningFor(uuid_type filter_uuid) {
  // Delete filter
  boost::mutex::scoped_lock l(filter_mux);
  filters.erase(std::find(filters.begin(),filters.end(),filter_uuid));
  comparators.erase(filter_uuid);
  boost::mutex::scoped_lock l2(callback_mux);
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

ComparatorType
SerialListener::exactly(std::string exact_str) {
  return boost::bind(&SerialListener::_exactly, _1, exact_str);
}

bool
SerialListener::_exactly(const std::string &token, std::string exact_str) {
  return token == exact_str;
}

