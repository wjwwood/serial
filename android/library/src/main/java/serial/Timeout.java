package serial;

/**
 * Structure for setting the timeout of the serial port, times are
 * in milliseconds.
 *
 * In order to disable the interbyte timeout, set it to {@link #MAX}.
 */
public final class Timeout {

    /**
     * Value that represents interbyte timeout should be disabled.
     *
     * This is an unsigned value.
     */
    public static final int MAX = -1;

    /**
     * Convenience function to generate Timeout structs using a
     * single absolute timeout.
     *
     * @param timeout A long that defines the time in milliseconds until a
     * timeout occurs after a call to read or write is made.
     * @return Timeout struct that represents this simple timeout provided.
     */
    public static final Timeout simpleTimeout(int timeout) {
        return new Timeout(MAX, timeout, 0, timeout, 0);
    }

    /**
     * Number of milliseconds between bytes received to timeout on.
     */
    int inter_byte_timeout;
    /**
     *  A constant number of milliseconds to wait after calling read.
     */
    int read_timeout_constant;
    /**
     * A multiplier against the number of requested bytes to wait after
     *  calling read.
     */
    int read_timeout_multiplier;
    /**
     * A constant number of milliseconds to wait after calling write.
     */
    int write_timeout_constant;
    /**
     * A multiplier against the number of requested bytes to wait after
     *  calling write.
     */
    int write_timeout_multiplier;

    public Timeout() {
        this(0, 0, 0, 0, 0);
    }

    /**
     * Convenience function to generate Timeout structs using a
     * single absolute timeout.
     *
     * @param timeout A long that defines the time in milliseconds until a
     * timeout occurs after a call to read or write is made.
     * @return Timeout struct that represents this simple timeout provided.
     */
    public Timeout(int timeout) {
        this(MAX, timeout, 0, timeout, 0);
    }

    public Timeout(int inter_byte_timeout, int read_timeout_constant, int read_timeout_multiplier, int write_timeout_constant, int write_timeout_multiplier) {
        this.inter_byte_timeout = inter_byte_timeout;
        this.read_timeout_constant = read_timeout_constant;
        this.read_timeout_multiplier = read_timeout_multiplier;
        this.write_timeout_constant = write_timeout_constant;
        this.write_timeout_multiplier = write_timeout_multiplier;
    }

}
