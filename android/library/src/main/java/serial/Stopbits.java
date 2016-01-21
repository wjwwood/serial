package serial;

/**
 * Enumeration defines the possible stopbit types for the serial port.
 */
public enum Stopbits {
    /**
     * Use 1 as stop bit.
     *
     * This is the default.
     */
    One(1),
    /**
     * Use 2 as stop bit.
     */
    Two(2),
    /**
     * Use 1.5 as stopbit.
     */
    OnePointFive(3);

    /**
     * Internal value of the enumeration.
     */
    public final int value;

    Stopbits(int value) {
        this.value = value;
    }

    static Stopbits fromValue(int value) {
        switch (value) {
            case 1:
                return One;
            case 2:
                return Two;
            case 3:
                return OnePointFive;
            default:
                throw new IllegalArgumentException("Invalid value.");
        }
    }
}
