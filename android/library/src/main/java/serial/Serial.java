package serial;

import java.io.Closeable;
import java.io.IOException;
import java.nio.ByteBuffer;
import java.nio.CharBuffer;
import java.nio.charset.Charset;
import java.util.regex.Pattern;

/**
 * Serial Port.
 *
 * Created by chzhong on 1/20/16.
 */
public class Serial implements Closeable {

    /**
     * Baudrate 110 bits/s.
     */
    public static final int BAUDRATE_110 = 110;
    /**
     * Baudrate 300 bits/s.
     */
    public static final int BAUDRATE_300 = 300;
    /**
     * Baudrate 600 bits/s.
     */
    public static final int BAUDRATE_600 = 600;
    /**
     * Baudrate 1200 bits/s.
     */
    public static final int BAUDRATE_1200 = 1200;
    /**
     * Baudrate 2400 bits/s.
     */
    public static final int BAUDRATE_2400 = 2400;
    /**
     * Baudrate 4800 bits/s.
     */
    public static final int BAUDRATE_4800 = 4800;
    /**
     * Baudrate 9600 bits/s. This is the default.
     */
    public static final int BAUDRATE_9600 = 9600;
    /**
     * Baudrate 14400 bits/s.
     */
    public static final int BAUDRATE_14400 = 14400;
    /**
     * Baudrate 28800 bits/s.
     */
    public static final int BAUDRATE_28800 = 28800;
    /**
     * Baudrate 38400 bits/s.
     */
    public static final int BAUDRATE_38400 = 38400;
    /**
     * Baudrate 56000 bits/s.
     */
    public static final int BAUDRATE_56000 = 56000;
    /**
     * Baudrate 57600 bits/s.
     */
    public static final int BAUDRATE_57600 = 57600;
    /**
     * Baudrate 115200 bits/s.
     */
    public static final int BAUDRATE_115200 = 115200;
    /**
     * Baudrate 128000 bits/s.
     */
    public static final int BAUDRATE_128000 = 128000;
    /**
     * Baudrate 153600 bits/s.
     */
    public static final int BAUDRATE_153600 = 153600;
    /**
     * Baudrate 230400 bits/s.
     */
    public static final int BAUDRATE_230400 = 230400;
    /**
     * Baudrate 256000 bits/s.
     */
    public static final int BAUDRATE_256000 = 256000;
    /**
     * Baudrate 460800 bits/s.
     */
    public static final int BAUDRATE_460800 = 460800;
    /**
     * Baudrate 921600 bits/s.
     */
    public static final int BAUDRATE_921600 = 921600;

    /**
     * End-of-line: line-feed (\n). This is the default.
     */
    public static final String EOL_LF = "\n";
    /**
     * End-of-line: carriage-return (\n).
     */
    public static final String EOL_CR = "\r";
    /**
     * End-of-line: CR & LF (\r\n).
     */
    public static final String EOL_CRLF = "\r\n";

    /**
     * Default charset.
     */
    public static final Charset CHARSET_DEFAULT;

    /**
     * UTF-8 charset.
     */
    public static final Charset CHARSET_UTF8;

    /**
     * ISO-8859-1 charset.
     */
    public static final Charset CHARSET_ISO_8859_1;

    /**
     * US-ASCII charset.
     */
    public static final Charset CHARSET_US_ASCII;

    /**
     * UTF-16 charset.
     */
    public static final Charset CHARSET_UTF16;

    /**
     * UTF-16LE charset.
     */
    public static final Charset CHARSET_UTF16LE;
    /**
     * UTF-16BE charset.
     */
    public static final Charset CHARSET_UTF16BE;


    private static final Pattern FIELD_DELIM = Pattern.compile("\\t");

    static {
        CHARSET_DEFAULT = Charset.defaultCharset();
        CHARSET_UTF8 = Charset.forName("UTF-8");
        CHARSET_ISO_8859_1 = Charset.forName("ISO-8859-1");
        CHARSET_US_ASCII = Charset.forName("US-ASCII");
        CHARSET_UTF16 = Charset.forName("UTF-16");
        CHARSET_UTF16LE = Charset.forName("UTF-16LE");
        CHARSET_UTF16BE = Charset.forName("UTF-16BE");

        System.loadLibrary("serial");
    }
    /**
     * List available ports on this device.
     *
     * @return an array of available ports.
     */
    public static PortInfo[] listPorts() {
        PortInfo[] ports;
        String[] portDescs = native_listPorts();
        if (portDescs != null && portDescs.length > 0) {
            ports = new PortInfo[portDescs.length];
            int index = 0;
            for (String portDesc : portDescs) {
                String[] parts = FIELD_DELIM.split(portDesc);
                ports[index++] = new PortInfo(parts[0], parts[1], parts[2]);
            }
        } else {
            ports = null;
        }
        return ports;
    }

    /**
     * Builder for {@link Serial}.
     */
    public static class Builder {

        private String mPort;
        private int mBaudrate = BAUDRATE_9600;
        private Timeout mTimeout = Timeout.simpleTimeout(1000);
        private ByteSize mBytesize = ByteSize.EightBits;
        private Parity mParity = Parity.None;
        private Stopbits mStopbits = Stopbits.One;
        private FlowControl mFlowcontrol = FlowControl.None;

        /**
         * Initial a {@link serial.Serial.Builder} for serial port.
         *
         */
        public Builder() {
        }

        /**
         * Initial a {@link serial.Serial.Builder} for serial port.
         *
         * @param port the path of the serial port.
         */
        public Builder(String port) {
            this.mPort = port;
        }

        /**
         * Initial a {@link serial.Serial.Builder} for serial port.
         *
         * @param port the path of the serial port.
         * @param baudrate the baudrate to use.
         */
        public Builder(String port, int baudrate) {
            this.mPort = port;
            this.mBaudrate = baudrate;
        }

        /**
         * @see Serial#setPort(String)
         */
        public Builder setPort(String port) {
            mPort = port;
            return this;
        }

        /**
         * @see Serial#setBaudrate(int)
         */
        public Builder setBaudrate(int baudrate) {
            mBaudrate = baudrate;
            return this;
        }

        /**
         * @see Serial#setTimeout(Timeout)
         */
        public Builder setTimeout(Timeout timeout) {
            mTimeout = timeout;
            return this;
        }

        /**
         * @see Serial#setBytesize(ByteSize)
         */
        public Builder setBytesize(ByteSize bytesize) {
            mBytesize = bytesize;
            return this;
        }

        /**
         * @see Serial#setParity(Parity)
         */
        public Builder setParity(Parity parity) {
            mParity = parity;
            return this;
        }

        /**
         * @see Serial#setStopbits(Stopbits)
         */
        public Builder setStopbits(Stopbits stopbits) {
            mStopbits = stopbits;
            return this;
        }

        /**
         * @see Serial#setFlowcontrol(FlowControl)
         */
        public Builder setFlowcontrol(FlowControl flowcontrol) {
            mFlowcontrol = flowcontrol;
            return this;
        }

        /**
         * Create the serial object with give parameters.
         *
         * @return an instance of {@link Serial}.
         *
         * @throws SerialIOException I/O error.
         */
        public Serial create() throws SerialIOException {
            return new Serial(mPort, mBaudrate, mTimeout, mBytesize, mParity, mStopbits, mFlowcontrol);
        }
    }

    /**
     * Creates a Serial object and opens the port if a port is specified,
     * otherwise it remains closed until serial::Serial::open is called.
     *
     * @param port A string containing the address of the serial port,
     *        which would be something like 'COM1' on Windows and '/dev/ttyS0'
     *        on Linux.
     *
     * @param baudrate An unsigned 32-bit integer that represents the baudrate
     *
     * @param timeout A serial::Timeout struct that defines the timeout
     * conditions for the serial port. \see serial::Timeout
     *
     * @\param bytesize Size of each byte in the serial transmission of data,
     * default is eightbits, possible values are: fivebits, sixbits, sevenbits,
     * eightbits
     *
     * @param parity Method of parity, default is parity_none, possible values
     * are: parity_none, parity_odd, parity_even
     *
     * @param stopbits Number of stop bits used, default is stopbits_one,
     * possible values are: stopbits_one, stopbits_one_point_five, stopbits_two
     *
     * @param flowcontrol Type of flowcontrol used, default is
     * flowcontrol_none, possible values are: flowcontrol_none,
     * flowcontrol_software, flowcontrol_hardware
     *
     * @throws SerialException Generic serial error.
     * @throws SerialIOException I/O error.
     * @throws IllegalArgumentException Invalid arguments are given.
     */
    private Serial(String port,
                   int baudrate,
                   Timeout timeout,
                   ByteSize bytesize,
                   Parity parity,
                   Stopbits stopbits,
                   FlowControl flowcontrol) throws SerialIOException {
        this.mTimeout = timeout;
        this.mNativeSerial = native_create(port, baudrate,
                new int[]{
                        timeout.inter_byte_timeout,
                        timeout.read_timeout_constant,
                        timeout.read_timeout_multiplier,
                        timeout.write_timeout_constant,
                        timeout.read_timeout_multiplier
                },
                bytesize.bitLength, parity.ordinal(), stopbits.value, flowcontrol.ordinal()
        );
        if (mNativeSerial != 0)
            this.mOpened = native_isOpen(mNativeSerial);
    }

    private boolean mOpened;
    private long mNativeSerial;
    private Timeout mTimeout;

    @Override
    protected void finalize() throws Throwable {
        if (mNativeSerial != 0) {
            native_close(mNativeSerial);
            native_destory(mNativeSerial);
            mNativeSerial = 0;
            mOpened = false;
        }
        super.finalize();
    }

    private void checkValid() {
        if (0 == mNativeSerial) {
            /*
            ...
            serial.checkValid : 0
            serial.xxx         : 1
            <caller>           : 2
             */
            StackTraceElement frame = Log.getStackFrameAt(2);
            String msg = PortNotOpenedException.formatMessage("Native port is invalid, please create another instance.", frame);
            throw new IllegalStateException(msg);
        }
    }

    private void checkOpened() {
        if (!mOpened) {
            /*
            ...
            serial.checkOpened : 0
            serial.xxx         : 1
            <caller>           : 2
             */
            StackTraceElement frame = Log.getStackFrameAt(2);
            throw new PortNotOpenedException("Native port is not opened.", frame);
        }
    }

    /**
     * Determine whether this instance has a valid port.
     *
     * @return true if native port is valid, false otherwise.
     */
    public boolean isValid() {
        return mNativeSerial != 0;
    }

    /**
     * Opens the serial port as long as the port is set and the port isn't
     * already open.
     *
     * If the port is provided to the constructor then an explicit call to open
     * is not needed.
     *
     * @see #Serial(String, int, Timeout, ByteSize, Parity, Stopbits, FlowControl)
     *
     * @throws SerialException Generic serial error.
     * @throws SerialIOException I/O error.
     * @throws IllegalArgumentException Invalid arguments are given.
     */
    public void open() throws SerialIOException {
        if (mOpened)
            return;
        checkValid();
        native_open(mNativeSerial);
    }


    /** Gets the open status of the serial port.
     *
     * @return Returns true if the port is open, false otherwise.
     */
    public boolean isOpen () {
        checkValid();
        return native_isOpen(mNativeSerial);
    }

    /**
     * Closes the serial port.
     *
     * @throws IOException I/O error.
     */
    public void close () throws IOException {
        if (mNativeSerial == 0 || !mOpened)
            return;
        native_close(mNativeSerial);
        mOpened = false;
    }

    /**
     * Return the number of characters in the buffer.
     *
     * @throws SerialIOException I/O Error.
     */
    public int available () throws SerialIOException {
        checkOpened();
        return native_available(mNativeSerial);
    }

    /**
     * Block until there is serial data to read or read_timeout_constant
     * number of milliseconds have elapsed. The return value is true when
     * the function exits with the port in a readable state, false otherwise
     * (due to timeout or select interruption).
     *
     * @throws SerialIOException I/O Error.
     */
    public boolean waitReadable () throws SerialIOException {
        checkOpened();
        return native_waitReadable(mNativeSerial);
    }

    /**
     * Block for a period of time corresponding to the transmission time of
     * count characters at present serial settings. This may be used in con-
     * junction with waitReadable to read larger blocks of data from the
     * port.
     */
    public void waitByteTimes (int count) {
        checkOpened();
        native_waitByteTimes(mNativeSerial, count);
    }

    /**
     * Read a given amount of bytes from the serial port into a given buffer.
     *
     * The read function will return in one of three cases:
     *  * The number of requested bytes was read.
     *    * In this case the number of bytes requested will match the size_t
     *      returned by read.
     *  * A timeout occurred, in this case the number of bytes read will not
     *    match the amount requested, but no exception will be thrown.  One of
     *    two possible timeouts occurred:
     *    * The inter byte timeout expired, this means that number of
     *      milliseconds elapsed between receiving bytes from the serial port
     *      exceeded the inter byte timeout.
     *    * The total timeout expired, which is calculated by multiplying the
     *      read timeout multiplier by the number of requested bytes and then
     *      added to the read timeout constant.  If that total number of
     *      milliseconds elapses after the initial call to read a timeout will
     *      occur.
     *  * An exception occurred, in this case an actual exception will be thrown.
     *
     * @param buffer An array of at least the requested size.
     * @param offset the offset of the buffer to receive data.
     * @param size how many bytes to be read.
     *
     * @return A size_t representing the number of bytes read as a result of the
     *         call to read.
     *
     * @throws SerialIOException I/O Error.
     */
    public int read (byte[] buffer, int offset, int size) throws SerialIOException {
        checkOpened();
        return native_read(mNativeSerial, buffer, offset, size);
    }

    /**
     * Read all data available from the serial port.
     *
     * @return A buffer that contains all available data.
     *
     * @throws SerialIOException I/O Error.
     */
    public byte[] read () throws SerialIOException {
        checkOpened();
        int available = native_available(mNativeSerial);
        if (available < 0)
            return null;
        else if (available == 0)
            return new byte[0];
        byte[] buffer = new byte[available];
        int bytesRead = native_read(mNativeSerial, buffer, 0, buffer.length);
        if (bytesRead < buffer.length) {
            byte[] out = new byte[bytesRead];
            System.arraycopy(buffer, 0, out, 0, bytesRead);
            buffer = out;
        }
        return  buffer;
    }

    /** Read a given amount of bytes from the serial port into a give buffer.
     *
     * @param buffer A reference to a std::vector of uint8_t.
     * @param size A size_t defining how many bytes to be read.
     *
     * @return A size_t representing the number of bytes read as a result of the
     *         call to read.
     *
     * @throws SerialIOException I/O Error.
     */
    public int read (ByteBuffer buffer, int size /*= 1*/) throws SerialIOException {
        checkOpened();
        byte[] buf = new byte[size];
        int bytesRead = read(buf, 0, buf.length);
        buffer.put(buf, 0, bytesRead);
        return bytesRead;
    }

    /**
     * Read a given amount of bytes from the serial port into a give buffer.
     *
     * @param buffer A reference to a std::string.
     * @param size A size_t defining how many bytes to be read.
     * @param charset The charset of the data.
     *
     * @return A size_t representing the number of bytes read as a result of the
     *         call to read.
     *
     * @throws SerialIOException I/O Error.
     */
    public int read(StringBuilder buffer, int size /*= 1*/, Charset charset) throws SerialIOException {
        checkOpened();
        if (charset == null)
            charset = CHARSET_DEFAULT;
        byte[] buf = new byte[size];
        int bytesRead = read(buf, 0, buf.length);
        CharBuffer chars = charset.decode(ByteBuffer.wrap(buf, 0, bytesRead));
        buffer.append(chars.toString());
        return bytesRead;
    }

    /** Read a given amount of bytes from the serial port and return a string
     *  containing the data.
     *
     * @param size A size_t defining how many bytes to be read.
     *
     * @return A std::string containing the data read from the port.
     *
     * @throws SerialIOException I/O Error.
     *
     */
    public String read(int size, Charset charset) throws SerialIOException {
        checkOpened();
        if (charset == null)
            charset = CHARSET_DEFAULT;
        StringBuilder sb = new StringBuilder();
        read(sb, size, charset);
        return sb.toString();
    }

    /** Reads in a line or until a given delimiter has been processed.
     *
     * Reads from the serial port until a single line has been read.
     *
     * @param buffer A std::string reference used to store the data.
     * @param size A maximum length of a line, defaults to 65536 (2^16)
     * @param eol A string to match against for the EOL.
     *
     * @return A size_t representing the number of bytes read.
     *
     * @throws SerialIOException I/O Error.
     */
    public int readline (StringBuilder buffer, int size /*= 65536*/, String eol /*= "\n"*/) throws SerialIOException {
        checkOpened();
        if (eol == null)
            eol = EOL_LF;
        String line = native_readline(mNativeSerial, size, eol);
        buffer.append(line);
        return line.getBytes().length;
    }

    /** Reads in a line or until a given delimiter has been processed.
     *
     * Reads from the serial port until a single line has been read.
     *
     * @param size A maximum length of a line, defaults to 65536 (2^16)
     * @param eol A string to match against for the EOL.
     *
     * @return A std::string containing the line.
     *
     * @throws SerialIOException I/O Error.
     */
    public String readline (int size /*= 65536*/, String eol /*= "\n"*/) throws SerialIOException {
        checkOpened();
        if (eol == null)
            eol = EOL_LF;
        return native_readline(mNativeSerial, size, eol);
    }

    /** Reads in multiple lines until the serial port times out.
     *
     * This requires a timeout > 0 before it can be run. It will read until a
     * timeout occurs and return a list of strings.
     *
     * @param size A maximum length of combined lines, defaults to 65536 (2^16)
     *
     * @param eol A string to match against for the EOL.
     *
     * @return A array containing the lines.
     *
     * @throws SerialIOException I/O Error.
     */
    public String[] readlines (int size /*= 65536*/, String eol /*= "\n"*/) throws SerialIOException {
        checkOpened();
        if (eol == null)
            eol = EOL_LF;
        return native_readlines(mNativeSerial, size, eol);
    }

    /** Write a string to the serial port.
     *
     * @param data A const reference containing the data to be written
     * to the serial port.
     *
     * @param size A size_t that indicates how many bytes should be written from
     * the given data buffer.
     *
     * @return A size_t representing the number of bytes actually written to
     * the serial port.
     *
     * @throws SerialIOException I/O Error.
     */
    public int write (byte[] data, int size) throws SerialIOException {
        checkOpened();
        return native_write(mNativeSerial, data, size);
    }

    /** Write a string to the serial port.
     *
     * @param s A const reference containing the data to be written
     * to the serial port.
     *
     * @return A size_t representing the number of bytes actually written to
     * the serial port.
     *
     * @throws SerialIOException I/O Error.
     */
    public int write (String s) throws SerialIOException {
        checkOpened();
        byte[] data = s.getBytes();
        return native_write(mNativeSerial, data, data.length);
    }

    /** Sets the serial port identifier.
     *
     * @param port A const std::string reference containing the address of the
     * serial port, which would be something like 'COM1' on Windows and
     * '/dev/ttyS0' on Linux.
     *
     * @throws IllegalArgumentException
     */
    public void setPort (String port) {
        checkValid();
        native_setPort(mNativeSerial, port);
    }

    /** Gets the serial port identifier.
     *
     * @see #setPort(String)
     *
     */
    public String getPort () {
        checkValid();
        return native_getPort(mNativeSerial);
    }

    /** Sets the timeout for reads and writes using the Timeout struct.
     *
     * There are two timeout conditions described here:
     *  * The inter byte timeout:
     *    * The inter_byte_timeout component of serial::Timeout defines the
     *      maximum amount of time, in milliseconds, between receiving bytes on
     *      the serial port that can pass before a timeout occurs.  Setting this
     *      to zero will prevent inter byte timeouts from occurring.
     *  * Total time timeout:
     *    * The constant and multiplier component of this timeout condition,
     *      for both read and write, are defined in serial::Timeout.  This
     *      timeout occurs if the total time since the read or write call was
     *      made exceeds the specified time in milliseconds.
     *    * The limit is defined by multiplying the multiplier component by the
     *      number of requested bytes and adding that product to the constant
     *      component.  In this way if you want a read call, for example, to
     *      timeout after exactly one second regardless of the number of bytes
     *      you asked for then set the read_timeout_constant component of
     *      serial::Timeout to 1000 and the read_timeout_multiplier to zero.
     *      This timeout condition can be used in conjunction with the inter
     *      byte timeout condition with out any problems, timeout will simply
     *      occur when one of the two timeout conditions is met.  This allows
     *      users to have maximum control over the trade-off between
     *      responsiveness and efficiency.
     *
     * Read and write functions will return in one of three cases.  When the
     * reading or writing is complete, when a timeout occurs, or when an
     * exception occurs.
     *
     * @param timeout A serial::Timeout struct containing the inter byte
     * timeout, and the read and write timeout constants and multipliers.
     *
     * @see Timeout
     */
    public void setTimeout (Timeout timeout) {
        checkValid();
        if (null == timeout)
            timeout = new Timeout();
        this.mTimeout = timeout;
        native_setTimeout(mNativeSerial, new int[] {
                timeout.inter_byte_timeout,
                timeout.read_timeout_constant,
                timeout.read_timeout_multiplier,
                timeout.write_timeout_constant,
                timeout.read_timeout_multiplier
        });
    }

    /**
     * Sets the timeout for reads and writes.
     */
    public void setTimeout (int inter_byte_timeout, int read_timeout_constant,
                            int read_timeout_multiplier, int write_timeout_constant,
                            int write_timeout_multiplier)
    {
        Timeout timeout = new Timeout(inter_byte_timeout, read_timeout_constant,
                read_timeout_multiplier, write_timeout_constant,
                write_timeout_multiplier);
        setTimeout(timeout);
    }

    /** Gets the timeout for reads in seconds.
     *
     * @return A Timeout struct containing the inter_byte_timeout, and read
     * and write timeout constants and multipliers.
     *
     * @see #setTimeout(Timeout)
     */
    public Timeout getTimeout () {
        return mTimeout;
    }

    /** Sets the baudrate for the serial port.
     *
     * Possible baudrates depends on the system but some safe baudrates include:
     * 110, 300, 600, 1200, 2400, 4800, 9600 (default), 14400, 19200, 28800, 38400, 56000,
     * 57600, 115200
     * Some other baudrates that are supported by some comports:
     * 128000, 153600, 230400, 256000, 460800, 921600
     *
     * @param baudrate An integer that sets the baud rate for the serial port.
     *
     * @throws SerialIOException I/O Error.
     */
    public void setBaudrate (int baudrate) throws SerialIOException {
        checkValid();
        native_setBaudrate(mNativeSerial, baudrate);
    }

    /** Gets the baudrate for the serial port.
     *
     * @return An integer that sets the baud rate for the serial port.
     *
     * @see #setBaudrate(int)
     *
     */
    public int getBaudrate () {
        checkValid();
        return native_getBaudrate(mNativeSerial);
    }

    /**
     *  Sets the bytesize for the serial port.
     *
     * @param bytesize Size of each byte in the serial transmission of data,
     * default is eightbits, possible values are: fivebits, sixbits, sevenbits,
     * eightbits
     *
     * @throws SerialIOException I/O Error.
     */
    public void setBytesize (ByteSize bytesize) throws SerialIOException {
        checkValid();
        if (null == bytesize)
            bytesize = ByteSize.EightBits;
        native_setBytesize(mNativeSerial, bytesize.bitLength);
    }

    /**
     *  Gets the bytesize for the serial port.
     *
     * @see #setBytesize(ByteSize)
     *
     */
    public ByteSize getBytesize ()  {
        checkValid();
        return ByteSize.fromValue(native_getBytesize(mNativeSerial));
    }

    /**
     * Sets the parity for the serial port.
     *
     * @param parity Method of parity, default is parity_none, possible values
     * are: parity_none, parity_odd, parity_even
     * @throws SerialIOException I/O Error.
     *
     */
    public void setParity (Parity parity) throws SerialIOException {
        checkValid();
        if (null == parity)
            parity = Parity.None;
        native_setParity(mNativeSerial, parity.ordinal());
    }

    /** Gets the parity for the serial port.
     *
     * @see #setParity(Parity)
     *
     */
    public Parity getParity() {
        checkValid();
        return Parity.values()[native_getParity(mNativeSerial)];
    }

    /**
     *  Sets the stopbits for the serial port.
     *
     * @param stopbits Number of stop bits used, default is stopbits_one,
     * possible values are: stopbits_one, stopbits_one_point_five, stopbits_two
     * @throws SerialIOException I/O Error.
     */
    public void setStopbits (Stopbits stopbits) throws SerialIOException {
        checkValid();
        if (null == stopbits)
            stopbits = Stopbits.One;
        native_setStopbits(mNativeSerial, stopbits.value);
    }

    /**
     * Gets the stopbits for the serial port.
     *
     * @see #setStopbits(Stopbits)
     *
     */
    public Stopbits getStopbits () {
        checkValid();
        return Stopbits.fromValue(native_getStopbits(mNativeSerial));
    }

    /**
     * Sets the flow control for the serial port.
     *
     * @param flowcontrol Type of flowcontrol used, default is flowcontrol_none,
     * possible values are: flowcontrol_none, flowcontrol_software,
     * flowcontrol_hardware
     *
     * @throws SerialIOException I/O Error.
     */
    public void setFlowcontrol (FlowControl flowcontrol) throws SerialIOException {
        checkValid();
        if (null == flowcontrol)
            flowcontrol = FlowControl.None;
        native_setFlowcontrol(mNativeSerial, flowcontrol.ordinal());
    }

    /**
     *  Gets the flow control for the serial port.
     *
     * @see #setFlowcontrol(FlowControl)
     *
     */
    public FlowControl getFlowcontrol () {
        checkValid();
        return FlowControl.values()[native_getFlowcontrol(mNativeSerial)];
    }

    /** Flush the input and output buffers */
    public void flush () {
        checkOpened();
        native_flush(mNativeSerial);
    }

    /** Flush only the input buffer */
    public void flushInput () {
        checkOpened();
        native_flushInput(mNativeSerial);
    }

    /** Flush only the output buffer */
    public void flushOutput () {
        checkOpened();
        native_flushOutput(mNativeSerial);
    }

    /** Sends the RS-232 break signal.  See tcsendbreak(3). */
    public void sendBreak (int duration) {
        checkOpened();
        native_sendBreak(mNativeSerial, duration);
    }

    /** Set the break condition to a given level. */
    public void setBreak (boolean level) {
        checkOpened();
        native_setBreak(mNativeSerial, level);
    }

    /** Set the break condition to true level. */
    public void setBreak () {
        checkOpened();
        setBreak(true);
    }

    /** Set the RTS handshaking line to the given level.*/
    public void setRTS (boolean level) {
        checkOpened();
        native_setRTS(mNativeSerial, level);
    }

    /** Set the RTS handshaking line to the true level.*/
    public void setRTS () {
        checkOpened();
        setRTS(true);
    }

    /** Set the DTR handshaking line to the given level. */
    public void setDTR (boolean level) {
        checkOpened();
        native_setDTR(mNativeSerial, level);
    }

    /** Set the DTR handshaking line to the given level. */
    public void setDTR () {
        checkOpened();
        setDTR(true);
    }

    /**
     * Blocks until CTS, DSR, RI, CD changes or something interrupts it.
     *
     * Can throw an exception if an error occurs while waiting.
     * You can check the status of CTS, DSR, RI, and CD once this returns.
     * Uses TIOCMIWAIT via ioctl if available (mostly only on Linux) with a
     * resolution of less than +-1ms and as good as +-0.2ms.  Otherwise a
     * polling method is used which can give +-2ms.
     *
     * @return Returns true if one of the lines changed, false if something else
     * occurred.
     *
     * @throws SerialException
     */
    public boolean waitForChange () {
        checkOpened();
        return native_waitForChange(mNativeSerial);
    }

    /*! Returns the current status of the CTS line. */
    public boolean getCTS () {
        checkOpened();
        return native_getCTS(mNativeSerial);
    }

    /** Returns the current status of the DSR line. */
    public boolean getDSR () {
        checkOpened();
        return native_getDSR(mNativeSerial);
    }

    /** Returns the current status of the RI line. */
    public boolean getRI () {
        checkOpened();
        return native_getRI(mNativeSerial);
    }

    /** Returns the current status of the CD line. */
    public boolean getCD () {
        checkOpened();
        return native_getCD(mNativeSerial);
    }


    private static native String[] native_listPorts();

    private static native long native_create(String port, int baudrate, int[] ints, int bytesize, int parity, int stopbits, int flowcontrol) throws IllegalArgumentException, SerialException, SerialIOException;
    private static native void native_destory(long nativePtr);
    private static native void native_open(long nativePtr)  throws IllegalArgumentException, SerialException, SerialIOException;
    private static native boolean native_isOpen(long nativePtr);
    private static native void native_close(long nativePtr) throws SerialIOException;
    private static native int native_available(long nativePtr) throws SerialIOException;
    private static native boolean native_waitReadable(long nativePtr) throws SerialIOException;
    private static native void native_waitByteTimes(long nativePtr, int count);
    private static native int native_read(long nativePtr, byte[] buffer, int offset, int size) throws IllegalArgumentException, SerialException, SerialIOException;
    private static native String native_readline(long nativePtr, int size, String eol) throws IllegalArgumentException, SerialException, SerialIOException;
    private static native String[] native_readlines(long nativePtr, int size, String eol) throws IllegalArgumentException, SerialException, SerialIOException;
    private static native int native_write(long nativePtr, byte[] buffer,int size) throws IllegalArgumentException, SerialException, SerialIOException;

    private static native void native_setPort(long nativePtr, String port);
    private static native String native_getPort(long nativePtr);

    private static native void native_setBaudrate(long nativePtr, int baudrate) throws IllegalArgumentException, SerialException, SerialIOException;
    private static native int native_getBaudrate(long nativePtr);
    private static native void native_setTimeout(long nativePtr, int[] timeouts);
    private static native int[] native_getTimeout(long nativePtr);

    private static native void native_setBytesize(long nativePtr, int bytesize) throws IllegalArgumentException, SerialException, SerialIOException;
    private static native int native_getBytesize(long nativePtr);
    private static native void native_setParity(long nativePtr, int parity) throws IllegalArgumentException, SerialException, SerialIOException;
    private static native int native_getParity(long nativePtr);
    private static native void native_setStopbits(long nativePtr, int stopbits) throws IllegalArgumentException, SerialException, SerialIOException;
    private static native int native_getStopbits(long nativePtr);
    private static native void native_setFlowcontrol(long nativePtr, int flowcontrol) throws IllegalArgumentException, SerialException, SerialIOException;
    private static native int native_getFlowcontrol(long nativePtr);

    private static native void native_flush(long nativePtr);
    private static native void native_flushInput(long nativePtr);
    private static native void native_flushOutput(long nativePtr);

    private static native void native_sendBreak(long nativePtr, int duration) throws SerialException;
    private static native void native_setBreak(long nativePtr, boolean level) throws SerialException;
    private static native void native_setRTS(long nativePtr, boolean level) throws SerialException;
    private static native void native_setDTR(long nativePtr, boolean level) throws SerialException;

    private static native boolean native_waitForChange(long nativePtr) throws SerialException;

    private static native boolean native_getCTS(long nativePtr) throws SerialException;
    private static native boolean native_getDSR(long nativePtr) throws SerialException;
    private static native boolean native_getRI(long nativePtr) throws SerialException;
    private static native boolean native_getCD(long nativePtr) throws SerialException;

}
