package serial;

import java.io.IOException;

/**
 * Created by chzhong on 1/20/16.
 */
public class SerialIOException extends IOException {

    /**
     * Constructs a new {@code SerialIOException} with its stack trace and detail
     * message filled in.
     *
     * @param detailMessage the detail message for this exception.
     */
    public SerialIOException(String detailMessage) {
        super(detailMessage);
    }

    /**
     * Constructs a new instance of this class with detail message and cause
     * filled in.
     *
     * @param message The detail message for the exception.
     * @param cause   The detail cause for the exception.
     * @since 1.6
     */
    public SerialIOException(String message, Throwable cause) {
        super(message, cause);
    }
}
