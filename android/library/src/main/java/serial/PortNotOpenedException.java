package serial;

import java.util.Locale;

/**
 * Created by chzhong on 1/20/16.
 */
public class PortNotOpenedException extends IllegalStateException {

    static final String formatMessage(String message, StackTraceElement frame) {
        return String.format(Locale.getDefault(), "%s. (%s.%s @ %s:%d)",
                message, frame.getClassName(), frame.getMethodName(),
                frame.getFileName(), frame.getLineNumber());
    }

    /**
     * Constructs a new {@code PortNotOpenedException} with the current stack trace
     * and the specified detail message.
     *
     * @param detailMessage the detail message for this exception.
     * @param frame the frame of the wrong call.
     */
    PortNotOpenedException(String detailMessage, StackTraceElement frame) {
        super(formatMessage(detailMessage, frame));
    }

}
