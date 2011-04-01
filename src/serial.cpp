#include "serial.h"
#include <iostream>

using namespace serial;

/** Completion Conditions **/

class transfer_at_least_ignore_invalid_argument {
public:
    typedef bool result_type;
    
    explicit transfer_at_least_ignore_invalid_argument(std::size_t minimum) : minimum_(minimum) {}
    
    template <typename Error>
    bool operator()(const Error& err, std::size_t bytes_transferred) {
        if(err) {// There is an Error
            if(err == boost::asio::error::invalid_argument)
                std::cout << "Invalid Argument Error" << std::endl;
            if(err == boost::asio::error::operation_aborted) {
                return 1;
            }
            if(err != boost::asio::error::invalid_argument) {// The Error is not invalid argument
                return 1; // Stop reading
            }
        }
        if(bytes_transferred >= minimum_) {// We have all the bytes we need
            return 1; // Stop
        } else {
            return 0; // Continue
        }
    }
    
private:
    std::size_t minimum_;
};

/** Classes for Handshaking control **/

#if defined(BOOST_WINDOWS) || defined(__CYGWIN__)
# define BOOST_ASIO_OPTION_STORAGE DCB
#else
# define BOOST_ASIO_OPTION_STORAGE termios
#endif

class DTRControl {
public:
    explicit DTRControl(bool enable = false) : m_enable(enable) {};
    
    boost::system::error_code store(BOOST_ASIO_OPTION_STORAGE& storage,
                                    boost::system::error_code& ec) const
    {
        #if defined(BOOST_WINDOWS) || defined(__CYGWIN__)
            if(m_enable)
                storage.fDtrControl = DTR_CONTROL_ENABLE;
            else
                storage.fDtrControl = DTR_CONTROL_DISABLE;
        #else
            ec = boost::asio::error::operation_not_supported;
            ec = boost::system::error_code();
        #endif
        return ec;
    };
    
    boost::system::error_code load(const BOOST_ASIO_OPTION_STORAGE& storage,
                                   boost::system::error_code& ec)
    {
        #if defined(BOOST_WINDOWS) || defined(__CYGWIN__)
            if(storage.fDtrControl == DTR_CONTROL_ENABLE)
                m_enable = true;
            else
                m_enable = true;
        #else
        #endif
        return ec;
    };
private:
    bool m_enable;
};

class RTSControl {
public:
    explicit RTSControl(bool enable = false) : m_enable(enable) {};
    boost::system::error_code store(BOOST_ASIO_OPTION_STORAGE& storage,
                                    boost::system::error_code& ec) const
    {
        #if defined(BOOST_WINDOWS) || defined(__CYGWIN__)
            if(m_enable)
                storage.fRtsControl = RTS_CONTROL_ENABLE;
            else
                storage.fRtsControl = RTS_CONTROL_DISABLE;
        #else
            ec = boost::asio::error::operation_not_supported;
            ec = boost::system::error_code();
        #endif
        return ec;
    };
    
    boost::system::error_code load(const BOOST_ASIO_OPTION_STORAGE& storage,
                                   boost::system::error_code& ec)
    {
        #if defined(BOOST_WINDOWS) || defined(__CYGWIN__)
            if(storage.fRtsControl == RTS_CONTROL_ENABLE)
                m_enable = true;
            else
                m_enable = true;
        #else
        #endif
        return ec;
    };
private:
    bool m_enable;
};

/** Serial Class Implementation **/

Serial::Serial() : io_service(), work(io_service), timeout_timer(io_service) {
    this->init();
}

Serial::Serial(std::string port,
               int baudrate,
               long timeout,
               bytesize_t bytesize,
               parity_t parity,
               stopbits_t stopbits,
               flowcontrol_t flowcontrol)
               : io_service(), work(io_service), timeout_timer(io_service)
{
    // Call default constructor to initialize variables
    this->init();
    
    // Write provided settings
    this->port = port;
    this->setBaudrate(baudrate);
    this->setTimeoutMilliseconds(timeout);
    this->setBytesize(bytesize);
    this->setParity(parity);
    this->setStopbits(stopbits);
    this->setFlowcontrol(flowcontrol);
    
    // Open the serial port
    this->open();
}

void Serial::init() {
    // Boost asio variables
    this->serial_port.reset();
    
    // Serial Port settings
    this->port = "";
    this->setBaudrate(DEFAULT_BAUDRATE);
    this->setTimeoutMilliseconds(DEFAULT_TIMEOUT);
    
    // Private variables
    this->bytes_read = 0;
    this->bytes_to_read = 0;
    this->reading = false;
    this->nonblocking = false;
}

Serial::~Serial() {
    this->close();
}

void Serial::open() {
    // Make sure the Serial port is not already open.
    if(this->serial_port != NULL && this->serial_port->is_open()) {
        throw(SerialPortAlreadyOpenException(this->port.c_str()));
    }
    
    // Try to open the serial port
    try {
        this->serial_port.reset(new boost::asio::serial_port(this->io_service, this->port));
        
        this->serial_port->set_option(this->baudrate);
        this->serial_port->set_option(this->flowcontrol);
        this->serial_port->set_option(this->parity);
        this->serial_port->set_option(this->stopbits);
        this->serial_port->set_option(this->bytesize);
    } catch(std::exception &e) {
        this->serial_port.reset();
        throw(SerialPortFailedToOpenException(e.what()));
    }
}

const bool Serial::isOpen() {
    if(this->serial_port != NULL)
        return this->serial_port->is_open();
    return false;
}

void Serial::close() {
    // Cancel the current timeout timer and async reads
    this->timeout_timer.cancel();
    if(this->serial_port != NULL) {
        this->serial_port->cancel();
        this->serial_port->close();
        this->serial_port.reset();
    }
}

static const boost::posix_time::time_duration timeout_zero_comparison(boost::posix_time::milliseconds(0));

const int Serial::read(char* buffer, int size) {
    this->reading = true;
    if(this->nonblocking) {// Do not wait for data
        this->serial_port->async_read_some(boost::asio::buffer(buffer, size),
                                boost::bind(&Serial::read_complete, this,
                                boost::asio::placeholders::error,
                                boost::asio::placeholders::bytes_transferred));
    } else {               // Wait for data until size is read or timeout occurs
        boost::asio::async_read(*this->serial_port, boost::asio::buffer(buffer, size), transfer_at_least_ignore_invalid_argument(size),
                                boost::bind(&Serial::read_complete, this,
                                boost::asio::placeholders::error,
                                boost::asio::placeholders::bytes_transferred));
    }
    if(this->timeout > timeout_zero_comparison) { // Only set a timeout_timer if there is a valid timeout
        this->timeout_timer.expires_from_now(this->timeout);
        this->timeout_timer.async_wait(boost::bind(&Serial::timeout_callback, this,
                                 boost::asio::placeholders::error));
    } else if(this->nonblocking) {
        this->timeout_timer.expires_from_now(boost::posix_time::milliseconds(1));
        this->timeout_timer.async_wait(boost::bind(&Serial::timeout_callback, this,
                                 boost::asio::placeholders::error));
    }
    
    while(this->reading)
        this->io_service.run_one();
    
    this->bytes_to_read = size;
    
    return this->bytes_read;
}

const std::string Serial::read(int size) {
    char *serial_buffer = new char[size];
    int bytes_read_ = this->read(serial_buffer, size);
    std::string return_str(serial_buffer, (std::size_t)bytes_read_);
    delete[] serial_buffer;
    return return_str;
}

std::string 
Serial::read_until(char delim, size_t size) {
    using namespace std;
    string r = "";
    
    while (r.find(delim) == string::npos) {
        string s = read(1);
        if (s.length() > 0) 
            r += s;
    }
    
    return r;
}

std::string 
Serial::read_until(std::string delim, size_t size) {
    using namespace std;
    string r = "";
    
    while (r.find(delim) == string::npos) {
        string s = read(1);
        if (s.length() > 0) 
            r += s;
    }
    
    return r;
}

void Serial::read_complete(const boost::system::error_code& error, std::size_t bytes_transferred) {
    if(!error || error != boost::asio::error::operation_aborted) { // If there was no error OR the error wasn't operation aborted (canceled), Cancel the timer
        this->timeout_timer.cancel();  // will cause timeout_callback to fire with an error
    }
    
    this->bytes_read = bytes_transferred;
    
    this->reading = false;
}

void Serial::timeout_callback(const boost::system::error_code& error) {
    if (!error) {
        // The timeout wasn't canceled, so cancel the async read
        this->serial_port->cancel();
    }
}

const int Serial::write(char data[], int length) {
    return boost::asio::write(*this->serial_port, boost::asio::buffer(data, length), boost::asio::transfer_all());
}

const int Serial::write(std::string data) {
    char *cstr = new char[data.size()+1];
    std::strcpy(cstr, data.c_str());
    int bytes_wrote = this->write(cstr, data.length());
    delete[] cstr;
    return bytes_wrote;
}

void Serial::setRTS(bool level) {
    this->serial_port->set_option(RTSControl(level));
}

void Serial::setDTR(bool level) {
    this->serial_port->set_option(DTRControl(level));
}

const bool Serial::getCTS() const {
    throw(boost::asio::error::operation_not_supported);
    return false;
}

const bool Serial::getDSR() const {
    throw(boost::asio::error::operation_not_supported);
    return false;
}

void Serial::setPort(std::string port) {
    this->port = port;
}

const std::string Serial::getPort() const {
    return this->port;
}

void Serial::setTimeoutMilliseconds(long timeout) {
    // If timeout > 0 then read until size or timeout occurs
    // If timeout == 0 then read nonblocking, return data available immediately up to size
    // If timeout < 0 then read blocking, until size is read, period.
    if(timeout > 0) {
        this->timeout = boost::posix_time::time_duration(boost::posix_time::milliseconds(timeout));
    } else {
        this->timeout = boost::posix_time::time_duration(boost::posix_time::milliseconds(0));
    }
    
    if(timeout == 0)
        this->nonblocking = true;
    else // Must be negative
        this->nonblocking = false;
}

const long Serial::getTimeoutMilliseconds() const {
    return this->timeout.total_milliseconds();
}

void Serial::setBaudrate(int baudrate) {
    this->baudrate = boost::asio::serial_port_base::baud_rate(baudrate);
}

const int Serial::getBaudrate() const {
    return this->baudrate.value();
}

void Serial::setBytesize(bytesize_t bytesize) {
    switch(bytesize) {
        case FIVEBITS:
            this->bytesize = boost::asio::serial_port_base::character_size(5);
            break;
        case SIXBITS:
            this->bytesize = boost::asio::serial_port_base::character_size(6);
            break;
        case SEVENBITS:
            this->bytesize = boost::asio::serial_port_base::character_size(7);
            break;
        case EIGHTBITS:
            this->bytesize = boost::asio::serial_port_base::character_size(8);
            break;
        default:
            throw(InvalidBytesizeException(bytesize));
            break;
    }
}

const bytesize_t Serial::getBytesize() const {
    return bytesize_t(this->bytesize.value());
}

void Serial::setParity(parity_t parity) {
    switch(parity) {
        case PARITY_NONE:
            this->parity = boost::asio::serial_port_base::parity(boost::asio::serial_port_base::parity::none);
            break;
        case PARITY_ODD:
            this->parity = boost::asio::serial_port_base::parity(boost::asio::serial_port_base::parity::odd);
            break;
        case PARITY_EVEN:
            this->parity = boost::asio::serial_port_base::parity(boost::asio::serial_port_base::parity::even);
            break;
        default:
            throw(InvalidParityException(parity));
            break;
    }
}

const parity_t Serial::getParity() const {
    switch(this->parity.value()) {
        case boost::asio::serial_port_base::parity::none:
            return parity_t(PARITY_NONE);
        case boost::asio::serial_port_base::parity::odd:
            return parity_t(PARITY_ODD);
        case boost::asio::serial_port_base::parity::even:
            return parity_t(PARITY_EVEN);
        default:
            throw(InvalidParityException(this->parity.value()));
    }
}

void Serial::setStopbits(stopbits_t stopbits) {
    switch(stopbits) {
        case STOPBITS_ONE:
            this->stopbits = boost::asio::serial_port_base::stop_bits(boost::asio::serial_port_base::stop_bits::one);
            break;
        case STOPBITS_ONE_POINT_FIVE:
            this->stopbits = boost::asio::serial_port_base::stop_bits(boost::asio::serial_port_base::stop_bits::onepointfive);
            break;
        case STOPBITS_TWO:
            this->stopbits = boost::asio::serial_port_base::stop_bits(boost::asio::serial_port_base::stop_bits::two);
            break;
        default:
            throw(InvalidStopbitsException(stopbits));
            break;
    }
}

const stopbits_t Serial::getStopbits() const {
    switch(this->stopbits.value()) {
        case boost::asio::serial_port_base::stop_bits::one:
            return stopbits_t(STOPBITS_ONE);
        case boost::asio::serial_port_base::stop_bits::onepointfive:
            return stopbits_t(STOPBITS_ONE_POINT_FIVE);
        case boost::asio::serial_port_base::stop_bits::two:
            return stopbits_t(STOPBITS_TWO);
        default:
            throw(InvalidStopbitsException(this->stopbits.value()));
    }
}

void Serial::setFlowcontrol(flowcontrol_t flowcontrol) {
    switch(flowcontrol) {
        case FLOWCONTROL_NONE:
            this->flowcontrol = boost::asio::serial_port_base::flow_control(boost::asio::serial_port_base::flow_control::none);
            break;
        case FLOWCONTROL_SOFTWARE:
            this->flowcontrol = boost::asio::serial_port_base::flow_control(boost::asio::serial_port_base::flow_control::software);
            break;
        case FLOWCONTROL_HARDWARE:
            this->flowcontrol = boost::asio::serial_port_base::flow_control(boost::asio::serial_port_base::flow_control::hardware);
            break;
        default:
            throw(InvalidFlowcontrolException(flowcontrol));
            break;
    }
}

const flowcontrol_t Serial::getFlowcontrol() const {
    switch(this->flowcontrol.value()) {
        case boost::asio::serial_port_base::flow_control::none:
            return flowcontrol_t(FLOWCONTROL_NONE);
        case boost::asio::serial_port_base::flow_control::software:
            return flowcontrol_t(FLOWCONTROL_SOFTWARE);
        case boost::asio::serial_port_base::flow_control::hardware:
            return flowcontrol_t(FLOWCONTROL_HARDWARE);
        default:
            throw(InvalidFlowcontrolException(this->flowcontrol.value()));
    }
}
