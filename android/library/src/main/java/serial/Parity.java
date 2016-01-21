package serial;

/**
 * Enumeration defines the possible parity types for the serial port.
 */
public enum Parity {
    /**
     * Use none parity type.
     *
     * This is the default.
     */
    None,
    /**
     * Use odd parity types
     */
    Odd,
    /**
     * Use even parity type
     */
    Even,
    /**
     * Use mark parity type.
     *
     */
    Mark,
    /**
     * Use space parity type.
     *
     */
    Space;
}
