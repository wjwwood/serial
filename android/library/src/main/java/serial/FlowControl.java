package serial;

/**
 * Enumeration defines the possible flowcontrol types for the serial port.
 */
public enum FlowControl {
    /**
     * Use none flow control.
     *
     * This is the default.
     */
    None,
    /**
     * Use software flow control.
     */
    Software,
    /**
     * Use hardware flow control.
     */
    Hardware;
}
