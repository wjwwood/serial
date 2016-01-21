package serial;

import android.text.TextUtils;

import java.io.PrintWriter;
import java.io.StringWriter;
import java.lang.reflect.Method;
import java.util.Locale;

import serial.android.BuildConfig;

/**
 * Log replacement for {@link android.util.Log}.
 *
 * @author chzhong
 */
public final class Log {

    /**
     *
     */
    private static final String MESSAGE_TEMPLATE = "[%s]%s";

    private static Object sSyncObject = new Object();

    /**
     * Priority constant for enable all loggings.
     */
    public static final int ALL = -1;

    /**
     * Priority constant for {@link #println(int, String, String)} or
     * {@link #setFilterLevel(int)} methods; use Log.v.
     */
    public static final int VERBOSE = 2;

    /**
     * Priority constant for the {@link #println(int, String, String)} or
     * {@link #setFilterLevel(int)} method; use Log.d.
     */
    public static final int DEBUG = 3;

    /**
     * Priority constant for the {@link #println(int, String, String)} or
     * {@link #setFilterLevel(int)} method; use Log.i.
     */
    public static final int INFO = 4;

    /**
     * Priority constant for the {@link #println(int, String, String)} or
     * {@link #setFilterLevel(int)} method; use Log.w.
     */
    public static final int WARN = 5;

    /**
     * Priority constant for the {@link #println(int, String, String)} or
     * {@link #setFilterLevel(int)} method; use Log.e.
     */
    public static final int ERROR = 6;

    /**
     * Priority constant for the {@link #println(int, String, String)} or
     * {@link #setFilterLevel(int)} method.
     */
    public static final int ASSERT = 7;

    /**
     * Priority constant for disable all loggings.
     */
    public static final int NONE = Integer.MAX_VALUE;

    /**
     * Filter level of logs. Only levels greater or equals this level will be
     * output to LogCat.
     */
    private static int sFilterLevel = BuildConfig.DEBUG ? ALL : WARN;

    private static String sApplicationTag;

    /**
     * Set the default tag for this application.
     *
     * @param tag The tag of the application.
     */
    public static void setApplicationTag(String tag) {
        sApplicationTag = tag;
    }

    /**
     * Gets the default tag of the application.
     *
     * @return The default tag of the application.
     */
    public static String getApplicationTag() {
        return sApplicationTag;
    }

    /**
     * Sets the filter level of logs. Only levels greater or equals this level
     * will be output to LogCat.
     *
     * @param level The filter level.
     */
    public static void setFilterLevel(int level) {
        synchronized (sSyncObject) {
            sFilterLevel = level;
        }
    }

    /**
     * Gets the filter level of logs. Only levels greater or equals this level
     * will be output to LogCat.
     *
     * @return Current filter level.
     */
    public static int getFilterLevel() {
        return sFilterLevel;
    }

    private Log() {
    }

    /**
     * Send a {@link #VERBOSE} log message.
     *
     * @param msg The message you would like logged.
     */
    public static int v(String msg) {
        return println(VERBOSE, null, msg);
    }

    /**
     * Send a {@link #VERBOSE} log message.
     *
     * @param tag Used to identify the source of a log message. It usually
     *            identifies the class or activity where the log call occurs.
     * @param msg The message you would like logged.
     */
    public static int v(String tag, String msg) {
        return println(VERBOSE, tag, msg);
    }

    /**
     * Send a {@link #VERBOSE} log message.
     *
     * @param tag Used to identify the source of a log message. It usually
     *            identifies the class or activity where the log call occurs.
     * @param format The format of the message you would like logged.
     * @param args The arguments used to format the message.
     */
    public static int v(String tag, String format, Object... args) {
        if (VERBOSE < sFilterLevel)
            return 0;
        String msg = formatString(format, args);
        return println(VERBOSE, tag, msg);
    }

    /**
     * Send a {@link #VERBOSE} log message and log the exception.
     *
     * @param tag Used to identify the source of a log message. It usually
     *            identifies the class or activity where the log call occurs.
     * @param msg The message you would like logged.
     * @param tr An exception to log
     */
    public static int v(String tag, String msg, Throwable tr) {
        return println(VERBOSE, tag, msg + '\n' + getStackTraceString(tr));
    }

    /**
     * Send a {@link #DEBUG} log message.
     *
     * @param msg The message you would like logged.
     */
    public static int d(String msg) {
        return println(DEBUG, null, msg);
    }

    /**
     * Send a {@link #DEBUG} log message.
     *
     * @param tag Used to identify the source of a log message. It usually
     *            identifies the class or activity where the log call occurs.
     * @param msg The message you would like logged.
     */
    public static int d(String tag, String msg) {
        return println(DEBUG, tag, msg);
    }

    /**
     * Send a {@link #DEBUG} log message.
     *
     * @param tag Used to identify the source of a log message. It usually
     *            identifies the class or activity where the log call occurs.
     * @param format The format of the message you would like logged.
     * @param args The arguments used to format the message.
     */
    public static int d(String tag, String format, Object... args) {
        if (DEBUG < sFilterLevel)
            return 0;
        String msg = formatString(format, args);
        return println(DEBUG, tag, msg);
    }

    /**
     * Send a {@link #DEBUG} log message and log the exception.
     *
     * @param tag Used to identify the source of a log message. It usually
     *            identifies the class or activity where the log call occurs.
     * @param msg The message you would like logged.
     * @param tr An exception to log
     */
    public static int d(String tag, String msg, Throwable tr) {
        return println(DEBUG, tag, msg + '\n' + getStackTraceString(tr));
    }

    /**
     * Send an {@link #INFO} log message.
     *
     * @param msg The message you would like logged.
     */
    public static int i(String msg) {
        return println(INFO, null, msg);
    }

    /**
     * Send an {@link #INFO} log message.
     *
     * @param tag Used to identify the source of a log message. It usually
     *            identifies the class or activity where the log call occurs.
     * @param msg The message you would like logged.
     */
    public static int i(String tag, String msg) {
        return println(INFO, tag, msg);
    }

    /**
     * Send an {@link #INFO} log message.
     *
     * @param tag Used to identify the source of a log message. It usually
     *            identifies the class or activity where the log call occurs.
     * @param format The format of the message you would like logged.
     * @param args The arguments used to format the message.
     */
    public static int i(String tag, String format, Object... args) {
        if (INFO < sFilterLevel)
            return 0;
        String msg = formatString(format, args);
        return println(INFO, tag, msg);
    }

    /**
     * Send a {@link #INFO} log message and log the exception.
     *
     * @param tag Used to identify the source of a log message. It usually
     *            identifies the class or activity where the log call occurs.
     * @param msg The message you would like logged.
     * @param tr An exception to log
     */
    public static int i(String tag, String msg, Throwable tr) {
        return println(INFO, tag, msg + '\n' + getStackTraceString(tr));
    }

    /**
     * Send a {@link #WARN} log message.
     *
     * @param msg The message you would like logged.
     */
    public static int w(String msg) {
        return println(WARN, null, msg);
    }

    /**
     * Send a {@link #WARN} log message.
     *
     * @param tag Used to identify the source of a log message. It usually
     *            identifies the class or activity where the log call occurs.
     * @param msg The message you would like logged.
     */
    public static int w(String tag, String msg) {
        return println(WARN, tag, msg);
    }

    /**
     * Send a {@link #WARN} log message.
     *
     * @param tag Used to identify the source of a log message. It usually
     *            identifies the class or activity where the log call occurs.
     * @param format The format of the message you would like logged.
     * @param args The arguments used to format the message.
     */
    public static int w(String tag, String format, Object... args) {
        if (WARN < sFilterLevel)
            return 0;
        String msg = formatString(format, args);
        return println(WARN, tag, msg);
    }

    /**
     * Send a {@link #WARN} log message and log the exception.
     *
     * @param tag Used to identify the source of a log message. It usually
     *            identifies the class or activity where the log call occurs.
     * @param msg The message you would like logged.
     * @param tr An exception to log
     */
    public static int w(String tag, String msg, Throwable tr) {
        return println(WARN, tag, msg + '\n' + getStackTraceString(tr));
    }

    /**
     * Checks to see whether or not a log for the specified tag is loggable at
     * the specified level. The default level of any tag is set to INFO. This
     * means that any level above and including INFO will be logged. Before you
     * make any calls to a logging method you should check to see if your tag
     * should be logged. You can change the default level by setting a system
     * property: 'setprop log.tag.&lt;YOUR_LOG_TAG> &lt;LEVEL>' Where level is
     * either VERBOSE, DEBUG, INFO, WARN, ERROR, ASSERT, or SUPPRESS. SUPRESS
     * will turn off all logging for your tag. You can also create a local.prop
     * file that with the following in it:
     * 'log.tag.&lt;YOUR_LOG_TAG>=&lt;LEVEL>' and place that in
     * /data/local.prop.
     *
     * @param tag The tag to check.
     * @param level The level to check.
     * @return Whether or not that this is allowed to be logged.
     * @throws IllegalArgumentException is thrown if the tag.length() > 23.
     */
    public static boolean isLoggable(String tag, int level) {
        return android.util.Log.isLoggable(tag, level);
    }

    /*
     * Send a {@link #WARN} log message and log the exception.
     * @param tag Used to identify the source of a log message. It usually
     * identifies the class or activity where the log call occurs.
     * @param tr An exception to log
     */
    public static int w(String tag, Throwable tr) {
        return println(WARN, tag, getStackTraceString(tr));
    }

    public static int w(Throwable tr) {
        return println(WARN, null, getStackTraceString(tr));
    }

    /**
     * Send an {@link #ERROR} log message.
     *
     * @param tag Used to identify the source of a log message. It usually
     *            identifies the class or activity where the log call occurs.
     * @param format The format of the message you would like logged.
     * @param args The arguments used to format the message.
     */
    public static int e(String tag, String format, Object... args) {
        if (ERROR < sFilterLevel)
            return 0;
        String msg = formatString(format, args);
        return println(ERROR, tag, msg);
    }

    private static String formatString(String format, Object... args) {
        try {
            return String.format(Locale.US, format, args);
        } catch (Exception e) {
            StringBuilder builder = new StringBuilder();
            builder.append(format);
            for (Object arg : args) {
                builder.append(arg.toString());
            }
            return builder.toString();
        }
    }

    /**
     * Send an {@link #ERROR} log message.
     *
     * @param msg The message you would like logged.
     */
    public static int e(String msg) {
        return println(ERROR, null, msg);
    }

    /**
     * Send an {@link #ERROR} log message.
     *
     * @param tr The throwable you would like logged.
     */
    public static int e(Throwable tr) {
        return println(ERROR, null, getStackTraceString(tr));
    }

    /**
     * Send an {@link #ERROR} log message.
     *
     * @param tag Used to identify the source of a log message. It usually
     *            identifies the class or activity where the log call occurs.
     * @param msg The message you would like logged.
     */
    public static int e(String tag, String msg) {
        return println(ERROR, tag, msg);
    }

    /**
     * Send a {@link #ERROR} log message and log the exception.
     *
     * @param msg The message you would like logged.
     * @param tr An exception to log
     */
    public static int e(String msg, Throwable tr) {
        return println(ERROR, null, msg + '\n' + getStackTraceString(tr));
    }

    /**
     * Send a {@link #ERROR} log message and log the exception.
     *
     * @param tag Used to identify the source of a log message. It usually
     *            identifies the class or activity where the log call occurs.
     * @param msg The message you would like logged.
     * @param tr An exception to log
     */
    public static int e(String tag, String msg, Throwable tr) {
        return println(ERROR, tag, msg + '\n' + getStackTraceString(tr));
    }

    /**
     * Handy function to get a loggable stack trace from a Throwable
     *
     * @param tr An exception to log
     */
    public static String getStackTraceString(Throwable tr) {
        if (tr == null) {
            return "";
        }
        StringWriter sw = new StringWriter();
        PrintWriter pw = new PrintWriter(sw);
        tr.printStackTrace(pw);
        return sw.toString();
    }

    /**
     * Low-level logging call.
     *
     * @param priority The priority/type of this log message
     * @param tag Used to identify the source of a log message. It usually
     *            identifies the class or activity where the log call occurs.
     * @param msg The message you would like logged.
     * @return The number of bytes written.
     */
    public static int println(int priority, String tag, String msg) {
        msg = (msg == null) ? "" : msg;

        int result = 0;
        if (priority >= sFilterLevel) {
            if (TextUtils.isEmpty(sApplicationTag) || sApplicationTag.equals(tag)) {
                result = android.util.Log.println(priority, tag, msg);
            } else if (TextUtils.isEmpty(tag) && !TextUtils.isEmpty(sApplicationTag)) {
                result = android.util.Log.println(priority, sApplicationTag, msg);
            } else {
                String message = formatString(MESSAGE_TEMPLATE, tag, msg);
                result = android.util.Log.println(priority, sApplicationTag, message);
            }
        }
        return result;
    }

    public static void myAssert(boolean condition, String message) {
        if (sFilterLevel == ALL && !condition) {
            throw new AssertionError(message);
        }
    }

    /**
     * Return the {@link StackTraceElement} for current method's caller.
     *
     * <p>For example:</p>
     * <pre>
     * void foo() {
     *     bar();
     * }
     *
     * void bar() {
     *     StackTraceElement caller = getCallerFrame();
     *
     *     // Will print "foo".
     *     System.println(caller.getMethodName());
     * }
     * </pre>
     *
     * <p> Might be {@code null}. If you call this from static void main(String[]).
     *
     * @return the stack frame of current method's caller.
     */
    public static StackTraceElement getCallerFrame() {
        /*
         * traces = {
         * 	nativeGetStackTrace,
         * 	Thread.getStackTrace,
         * 	getStackFrameAt(2),
         * 	getCurrentFrame,        ... [3]
         * 	<current>,              ... [3 + 1]
         * 	<caller>,               ... [3 + 2]
         * 	...
         * }
         */
        return getStackFrameAt(2);
    }

    /**
     * Return the {@link StackTraceElement} for current method.
     *
     * <p>For example:</p>
     * <pre>
     * void foo() {
     *     bar();
     * }
     *
     * void bar() {
     *     StackTraceElement current = getCurrentFrame();
     *
     *     // Will print "bar".
     *     System.println(current.getMethodName());
     * }
     * </pre>
     *
     * @return the stack frame of current method's caller.
     */
    public static StackTraceElement getCurrentFrame() {
        /*
         * traces = {
         * 	nativeGetStackTrace,
         * 	Thread.getStackTrace,
         * 	getStackFrameAt(1),
         * 	getCurrentFrame,        ... [3]
         * 	<current>,              ... [3 + 1]
         * 	...
         * }
         */
        return getStackFrameAt(1);
    }

    /**
     * Return specified {@link StackTraceElement} on the call stack, relative to current method.
     *
     * <p>For example:</p>
     * <pre>
     * void main() {
     *     stub();
     * }
     *
     * void stub() {
     *     foo();
     * }
     * void foo() {
     *     bar();
     * }
     *
     * void bar() {
     *     // Will print "bar"
     *     System.println(getStackFrameAt(0).getMethodName());
     *     // Will print "foo"
     *     System.println(getStackFrameAt(1).getMethodName());
     *     // Will print "stub"
     *     System.println(getStackFrameAt(2).getMethodName());
     *     // Will print "main"
     *     System.println(getStackFrameAt(3).getMethodName());
     *     // Will print "null"
     *     System.println(getStackFrameAt(4));
     *     System.println(getStackFrameAt(-1));
     * }
     * </pre>
     *
     * @param offset the offset of the call stack from current method. 0 means current methods, 1 means caller, etc.
     * @return the stack frame of current method's caller.
     */
    public static StackTraceElement getStackFrameAt(int offset) {
        if (offset < 0) {
            return null;
        }
        /*
         * traces = {
         * 	nativeGetStackTrace,
         * 	Thread.getStackTrace,
         * 	getStackFrameAt(offset),
         * 	<current>,              ... [3 + 0], offset 0
         * 	<caller>,               ... [3 + 1], offset 1
         * 	<super caller>,         ... [3 + 2], offset 2
         * 	<grand caller>,        ... [3 + 3], offset 3
         * 	...
         * }
         */
        StackTraceElement[] traces = Thread.currentThread().getStackTrace();
        if (traces.length < 4 + offset) {
            // The stack at offset is underflow...
            return null;
        }
        return traces[3 + offset];
    }
}
