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

void
_delimeter_tokenizer (std::string &data, std::vector<std::string> &tokens,
                      std::string delimeter)
{
  boost::split(tokens, data, boost::is_any_of(delimeter));
}

TokenizerType
delimeter_tokenizer (std::string delimeter) {
  return boost::bind(_delimeter_tokenizer, _1, _2, delimeter);
}

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
}

void
SerialListener::stopListening() {
  // Stop listening and clear buffers
  listening = false;
  listen_thread.join();
  this->buffer = "";
  this->lines.clear();
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
  callbacks.clear();
  condition_vars.clear();
}

void
SerialListener::listen() {
  // Make sure there is a serial port
  if (this->serial_port == NULL) {
    this->handle_exc(SerialListenerException("Invalid serial port."));
  }
  // Make sure the serial port is open
  if (!this->serial_port->isOpen()) {
    this->handle_exc(SerialListenerException("Serial port not open."));
  }
  try {
    while (this->listening) {
      // Determine how much to read in
      size_t amount_to_read = determineAmountToRead();
      // Read some
      std::string temp = this->serial_port->read(amount_to_read);
      // If nothing was read and there is nothing in the lines, then we
      //  don't need to interate through the filters
      if (temp.length() == 0 && lines.size() == 0) {
        continue;
      }
      // Add the new data to the buffer
      this->buffer += temp;
      // If there is no return carrage in the buffer, then a command hasn't 
      //  been completed and if there is no data in the lines buffer, then 
      //  continue.
      if (this->buffer.find("\r") == std::string::npos && lines.size() == 0) {
        continue;
      }
      // Listen once, this parses the buffer and filters the data in lines
      buffer = this->listenOnce(buffer);
      // Done parsing lines and buffer should now be set to the left overs
    } // while (this->listening)
  } catch (std::exception &e) {
    this->handle_exc(SerialListenerException(e.what()));
  }
}

// TODO: as it is, each line is passed to filters repeatedly until they are 
//       too old...  Change it to only send each line to each filter once and 
//       then send to new fitlers up until it is too old.
std::string
SerialListener::listenOnce(std::string data) {
  std::string left_overs;
  std::vector<uuid_type> to_be_erased;
  // Tokenize the new data
  std::vector<std::string> new_lines;
  tokenize(data, new_lines);
  // Iterate through new lines and add times to them
  std::vector<std::string>::iterator it_new;
  for(it_new=new_lines.begin(); it_new != new_lines.end(); it_new++) {
    // The last line needs to be put back in the buffer always:
    //  In the case that the string ends with \r the last element will be 
    //  empty ("").  In the case that it does not the last element will be 
    //  what is left over from the next message that hasn't sent 
    //  everything.  Ex.: "?$1E\r" -> ["?$1E", ""] and 
    //  "?$1E\r$1E=Robo" -> ["?$1E","$1E=Robo"]
    if (it_new == new_lines.end()-1) {
      left_overs = (*it_new);
      continue;
    }
    uuid_type uuid = random_generator();
    lines.insert(std::pair<const uuid_type,std::string>(uuid,(*it_new)));
    using namespace boost::posix_time;
    ttls.insert(std::pair<const uuid_type,ptime>
                  (uuid,ptime(microsec_clock::local_time())));
  }
  // Iterate through the lines checking for a match
  std::map<const uuid_type,std::string>::iterator it_lines;
  for(it_lines=lines.begin(); it_lines!=lines.end(); it_lines++) {
    std::string line = (*it_lines).second;
    uuid_type uuid = (*it_lines).first;
    // If the line is empty, continue
    if (line.length() == 0) {
      continue;
    }
    bool matched = false;
    bool erased = false;
    // Get the filter lock
    boost::mutex::scoped_lock l(filter_mux);
    // Iterate through each filter
    std::map<const uuid_type,std::string>::iterator it;
    for(it=filters.begin(); it!=filters.end(); it++) {
      if (comparators[(*it).first](line)) { // If comparator matches line
        if ((*it).second == "non-blocking") {
// TODO: Put this callback execution into a queue. And if I do, make sure to 
//        keep the line instance around until the callback is done...
          // If non-blocking run the callback
          callbacks[(*it).first](line);
          to_be_erased.push_back(uuid);
          erased = true;
        } else if ((*it).second == "blocking") {
          // If blocking then notify the waiting call to continue
          condition_vars[(*it).first]->notify_all();
          to_be_erased.push_back(uuid);
          erased = true;
        }
        matched = true;
        break; // It matched, continue to next line
      }
    } // for(it=filters.begin(); it!=filters.end(); it++)
    // If not already erased check how old it is, remove the too old
    if (!erased) {
      using namespace boost::posix_time;
      if (ptime(microsec_clock::local_time())-ttls[uuid] > ttl) {
        // If there is a default handler pass it on
        if (this->default_handler) {
// TODO: see above about callback execution queue
          this->default_handler(line);
        }
        to_be_erased.push_back(uuid);
      }
    }
  } // for(it_lines=lines.begin(); it_lines!=lines.end(); it_lines++)
  // Remove any lines that need to be erased
  //  (this must be done outside the iterator to prevent problems incrementing 
  //   the iterator)
  std::vector<uuid_type>::iterator it;
  for (it=to_be_erased.begin(); it != to_be_erased.end(); it++) {
    lines.erase((*it));
    ttls.erase((*it));
  }
  // Return the left_overs
  return left_overs;
}

size_t
SerialListener::determineAmountToRead() {
  // TODO: Make a more intelligent method based on the length of the things 
  //  filters are looking for.  i.e.: if the filter is looking for 'V=XX\r' 
  //  make the read amount at least 5.
  return 5;
}

bool
SerialListener::listenForOnceComparator(std::string line) {
  if (line == current_listen_for_one_target)
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
  std::pair<const uuid_type,std::string>
   filter_pair(uuid, "blocking");
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
  std::pair<const uuid_type,std::string>
   filter_pair(uuid, "non-blocking");
  std::pair<const uuid_type,ComparatorType>
   comparator_pair(uuid, comparator);
  std::pair<const uuid_type,DataCallback>
   callback_pair(uuid, callback);

  {
    boost::mutex::scoped_lock l(filter_mux);
    filters.insert(filter_pair);
    comparators.insert(comparator_pair);
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


