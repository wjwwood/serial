#include "serial.h"

/** Completion Conditions **/

class transfer_at_least_ignore_invalid_argument {
public:
    typedef bool result_type;
    
    explicit transfer_at_least_ignore_invalid_argument(std::size_t minimum) : minimum_(minimum) {}
    
    template <typename Error>
    bool operator()(const Error& err, std::size_t bytes_transferred) {
        std::cout << "Here2" << std::endl;
        if(err) {// There is an Error
            if(err == boost::asio::error::operation_aborted) {
                std::cout << "Here1" << std::endl;
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

Serial::Serial() {
    this->init();
}

Serial::Serial(std::string port,
               int baudrate,
               double timeout,
               int bytesize,
               int parity,
               int stopbits,
               int flowcontrol) 
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
    this->work = new boost::asio::io_service::work(this->io_service);
    this->serial_port = NULL;
    this->timeout_timer = new boost::asio::deadline_timer(this->io_service);
    
    // Serial Port settings
    this->port = "";
    this->setBaudrate(DEFAULT_BAUDRATE);
    this->setTimeoutMilliseconds(DEFAULT_TIMEOUT);
    
    // Private variables
    this->bytes_read = 0;
    this->bytes_to_read = 0;
    this->reading = false;
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
        this->serial_port = new boost::asio::serial_port(this->io_service, this->port);
        
        this->serial_port->set_option(*this->baudrate);
        this->serial_port->set_option(*this->flowcontrol);
        this->serial_port->set_option(*this->parity);
        this->serial_port->set_option(*this->stopbits);
        this->serial_port->set_option(*this->bytesize);
    } catch(std::exception &e) {
        throw(SerialPortFailedToOpenException(e.what()));
        this->serial_port = NULL;
    }
}

void Serial::close() {
    // Cancel the current timeout timer and async reads
    this->timeout_timer->cancel();
    this->serial_port->cancel();
    this->serial_port->close();
}

int Serial::read(char* buffer, int size) {
    this->reading = true;
    boost::asio::async_read(*(this->serial_port), boost::asio::buffer(buffer, size), transfer_at_least_ignore_invalid_argument(size),
                            boost::bind(&Serial::read_complete, this,
                            boost::asio::placeholders::error,
                            boost::asio::placeholders::bytes_transferred));
    timeout_timer->expires_from_now(*this->timeout);
    timeout_timer->async_wait(boost::bind(&Serial::timeout_callback, this,
                             boost::asio::placeholders::error));
    
    while(this->reading)
        this->io_service.run_one();
    
    this->bytes_to_read = size;
    
    return this->bytes_read;
}

std::string Serial::read(int size) {
    char serial_buffer[size];
    int bytes_read_ = this->read(serial_buffer, size);
    return std::string(serial_buffer, (std::size_t)bytes_read_);
}

void Serial::read_complete(const boost::system::error_code& error, std::size_t bytes_transferred) {
    // std::cout << "Here2" << std::endl;
    if(!error || error != boost::asio::error::operation_aborted) { // If there was no error OR the error wasn't operation aborted (canceled), Cancel the timer
        this->timeout_timer->cancel();  // will cause timeout_callback to fire with an error
    }
    
    this->bytes_read = bytes_transferred;
    
    this->reading = false;
}

void Serial::timeout_callback(const boost::system::error_code& error) {
    if (!error) {
        // The timeout wasn't canceled, so cancel the async read
        std::cout << "Here3" << std::endl;
        this->serial_port->cancel();
    }
}

int Serial::write(char data[], int length) {
    return boost::asio::write(*this->serial_port, boost::asio::buffer(data, length), boost::asio::transfer_all());
}

int Serial::write(std::string data) {
    char * cstr;
    cstr = new char[data.size()+1];
    std::strcpy(cstr, data.c_str());
    return this->write(cstr, data.length());
}

void Serial::setRTS(bool level) {
    this->serial_port->set_option(RTSControl(level));
}

void Serial::setDTR(bool level) {
    this->serial_port->set_option(DTRControl(level));
}

bool Serial::getCTS() {
    throw(boost::asio::error::operation_not_supported);
    return false;
}

bool Serial::getDSR() {
    throw(boost::asio::error::operation_not_supported);
    return false;
}

void Serial::setTimeoutMilliseconds(long timeout) {
    if(timeout > 0.0) {
        this->timeout = new boost::posix_time::milliseconds(timeout);
    } else {
        this->timeout = NULL;
    }
}

long Serial::getTimeoutMilliseconds() {
    return this->timeout->total_milliseconds();
}

void Serial::setBaudrate(int baudrate) {
    this->baudrate = new boost::asio::serial_port_base::baud_rate(baudrate);
}

int Serial::getBaudrate() {
    return this->baudrate->value();
}

void Serial::setBytesize(int bytesize) {
    switch(bytesize) {
        case FIVEBITS:
            this->bytesize = new boost::asio::serial_port_base::character_size(5);
            break;
        case SIXBITS:
            this->bytesize = new boost::asio::serial_port_base::character_size(6);
            break;
        case SEVENBITS:
            this->bytesize = new boost::asio::serial_port_base::character_size(7);
            break;
        case EIGHTBITS:
            this->bytesize = new boost::asio::serial_port_base::character_size(8);
            break;
        default:
            throw(InvalidBytesizeException(bytesize));
            break;
    }
}

int Serial::getBytesize() {
    return this->bytesize->value();
}

void Serial::setParity(int parity) {
    switch(parity) {
        case PARITY_NONE:
            this->parity = new boost::asio::serial_port_base::parity(boost::asio::serial_port_base::parity::none);
            break;
        case PARITY_ODD:
            this->parity = new boost::asio::serial_port_base::parity(boost::asio::serial_port_base::parity::odd);
            break;
        case PARITY_EVEN:
            this->parity = new boost::asio::serial_port_base::parity(boost::asio::serial_port_base::parity::even);
            break;
        default:
            throw(InvalidParityException(parity));
            break;
    }
}

int Serial::getParity() {
    switch(this->parity->value()) {
        case boost::asio::serial_port_base::parity::none:
            return PARITY_NONE;
        case boost::asio::serial_port_base::parity::odd:
            return PARITY_ODD;
        case boost::asio::serial_port_base::parity::even:
            return PARITY_EVEN;
    }
    return -1;
}

void Serial::setStopbits(int stopbits) {
    switch(stopbits) {
        case STOPBITS_ONE:
            this->stopbits = new boost::asio::serial_port_base::stop_bits(boost::asio::serial_port_base::stop_bits::one);
            break;
        case STOPBITS_ONE_POINT_FIVE:
            this->stopbits = new boost::asio::serial_port_base::stop_bits(boost::asio::serial_port_base::stop_bits::onepointfive);
            break;
        case STOPBITS_TWO:
            this->stopbits = new boost::asio::serial_port_base::stop_bits(boost::asio::serial_port_base::stop_bits::two);
            break;
        default:
            throw(InvalidStopbitsException(stopbits));
            break;
    }
}

int Serial::getStopbits() {
    switch(this->parity->value()) {
        case boost::asio::serial_port_base::stop_bits::one:
            return STOPBITS_ONE;
        case boost::asio::serial_port_base::stop_bits::onepointfive:
            return STOPBITS_ONE_POINT_FIVE;
        case boost::asio::serial_port_base::stop_bits::two:
            return STOPBITS_TWO;
    }
    return -1;
}

void Serial::setFlowcontrol(int flowcontrol) {
    switch(flowcontrol) {
        case FLOWCONTROL_NONE:
            this->flowcontrol = new boost::asio::serial_port_base::flow_control(boost::asio::serial_port_base::flow_control::none);
            break;
        case FLOWCONTROL_SOFTWARE:
            this->flowcontrol = new boost::asio::serial_port_base::flow_control(boost::asio::serial_port_base::flow_control::software);
            break;
        case FLOWCONTROL_HARDWARE:
            this->flowcontrol = new boost::asio::serial_port_base::flow_control(boost::asio::serial_port_base::flow_control::hardware);
            break;
        default:
            throw(InvalidFlowcontrolException(flowcontrol));
            break;
    }
}

int Serial::getFlowcontrol() {
    switch(this->parity->value()) {
        case boost::asio::serial_port_base::flow_control::none:
            return FLOWCONTROL_NONE;
        case boost::asio::serial_port_base::flow_control::software:
            return FLOWCONTROL_SOFTWARE;
        case boost::asio::serial_port_base::flow_control::hardware:
            return FLOWCONTROL_HARDWARE;
    }
    return -1;
}