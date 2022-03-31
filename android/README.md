# Serial Communication Library for android (JNI API)

This is the Android support for Cross-platform, Serial Port library (JNI API).

### Dependencies

To build the code:
* NDK r10 (http://developer.android.com/intl/ndk/downloads/index.html)

The following linux headers are imported AS-IS to fix build breaks:

/usr/include/linux/serial.h
/usr/include/linux/tty_flags.h
/usr/include/sys/signal.h
/usr/include/sysexits.h

### Build

    cd android
    ndk-build

### About Java API

If you need to use Java API in android, have a look at https://github.com/chzhong/serial-android.

### License and Author

serial:
See README.md at the root directory.

Linux headers:
See file header for detail.