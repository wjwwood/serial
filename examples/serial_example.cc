#include <string>
#include <iostream>
#include <cstdio>

// OS Specific sleep
#ifdef __WIN32__
#include <windows.h>
#else
#include <unistd.h>
#endif

#include "serial/serial.h"

using std::string;
using std::exception;
using std::cout;
using std::cerr;
using std::endl;

void my_sleep(unsigned long milliseconds) {
#ifdef __WIN32__
      Sleep(milliseconds); // 100 ms
#else
      usleep(milliseconds*1000); // 100 ms
#endif
}

int run(int argc, char **argv)
{
    if(argc < 3) {
      cerr << "Usage: test_serial <serial port address> ";
      cerr << "<baudrate> [test string]" << endl;
      return 0;
    }
    // Argument 1 is the serial port
    string port(argv[1]);

    // Argument 2 is the baudrate
    unsigned long baud = 0;
    sscanf(argv[2], "%lu", &baud);

    // port, baudrate, timeout in milliseconds
    serial::Serial serial(port, baud, 1000);

    cout << "Is the serial port open?";
    if(serial.isOpen())
        cout << " Yes." << endl;
    else
        cout << " No." << endl;

    int count = 0;
    string test_string;
    if (argc == 4) {
      test_string = argv[3];
    } else {
      test_string = "Testing.";
    }
    while (true) {
      size_t bytes_wrote = serial.write(test_string);

      string result = serial.read(test_string.length());

      cout << "Iteration: " << count << ", Bytes written: ";
      cout << bytes_wrote << ", Bytes read: ";
      cout << result.length() << ", String read: " << result << endl;

      count += 1;
      my_sleep(10);
    }
    
    return 0;
}

int main(int argc, char **argv) {
  try {
    return run(argc, argv);
  } catch (exception &e) {
    cerr << "Unhandled Exception: " << e.what() << endl;
  }
}
