package serial;

/**
 * Enumeration defines the possible bytesizes for the serial port.
 */
public enum ByteSize {
    /**
     * Use 5 bits in a byte.
     */
    FiveBits(5),
    /**
     * Use 6 bits in a byte.
     */
    SixBits(6),
    /**
     * Use 7 bits in a byte.
     */
    SevenBits(7),
    /**
     * Use 8 bits in a byte.
     *
     * This is the default.
     */
    EightBits(8);

    /**
     * Number of bits in a single byte.
     */
    public final int bitLength;

    ByteSize(int bitLength) {
        this.bitLength = bitLength;
    }

    static ByteSize fromValue(int value) {
        switch (value) {
            case 5:
                return FiveBits;
            case 6:
                return SixBits;
            case 7:
                return SevenBits;
            case 8:
                return EightBits;
            default:
                throw new IllegalArgumentException("Invalid value.");
        }
    }

}
