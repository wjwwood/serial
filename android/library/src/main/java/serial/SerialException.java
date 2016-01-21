package serial;

/**
 * Created by chzhong on 1/20/16.
 */
public class SerialException extends RuntimeException {
    /**
     * Constructs a new {@code SerialException} with the current stack trace
     * and the specified detail message.
     *
     * @param detailMessage the detail message for this exception.
     */
    public SerialException(String detailMessage) {
        super(detailMessage);
    }

    /**
     * Constructs a new {@code PortNotOpenedException} with the current stack trace,
     * the specified detail message and the specified cause.
     *
     * @param detailMessage the detail message for this exception.
     * @param throwable
     */
    public SerialException(String detailMessage, Throwable throwable) {
        super(detailMessage, throwable);
    }
}
