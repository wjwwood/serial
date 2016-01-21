package serial;

/**
 * Created by chzhong on 1/20/16.
 */
public final class PortInfo {

    /**
     * Address of the serial port (this can be passed to the constructor of Serial).
     */
    public final String port;

    /**
     * Human readable description of serial device if available.
     */
    public final String description;

    /**
     * Hardware ID (e.g. VID:PID of USB serial devices) or "n/a" if not available.
     */
    public final String hardwareId;

    PortInfo(String port, String description, String hardwareId) {
        this.port = port;
        this.description = description;
        this.hardwareId = hardwareId;
    }

    @Override
    public String toString() {
        return String.format("%s (%s)", description, port);
    }
}
