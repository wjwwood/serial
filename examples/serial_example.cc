#include <string>
#include <iostream>
#include <stdio.h>
#include <time.h>

#ifdef __MACH__
#include <mach/clock.h>
#include <mach/mach.h>
#endif

#include <boost/thread.hpp>

#include "serial/serial.h"

using std::string;
using std::cout;
using std::endl;

int run(int argc, char **argv)
{
    if(argc < 3) {
        std::cerr << "Usage: test_serial <serial port address> <baudrate>" << std::endl;
        return 0;
    }
    std::string port(argv[1]);
    unsigned long baud = 0;

    sscanf(argv[2], "%lu", &baud);

    // port, baudrate, timeout in milliseconds
    serial::Serial serial(port, baud, 10000);
    
    std::cout << "Is the serial port open?";
    if(serial.isOpen())
        std::cout << " Yes." << std::endl;
    else
        std::cout << " No." << std::endl;
    
    int count = 0;
    while (count >= 0) {
      struct timespec start, end;
      double diff;

      #ifdef __MACH__ // OS X does not have clock_gettime, use clock_get_time
      clock_serv_t cclock;
      mach_timespec_t mts;
      host_get_clock_service(mach_host_self(), CALENDAR_CLOCK, &cclock);
      clock_get_time(cclock, &mts);
      mach_port_deallocate(mach_task_self(), cclock);
      start.tv_sec = mts.tv_sec;
      start.tv_nsec = mts.tv_nsec;

      #else
      clock_gettime(CLOCK_REALTIME, &start);
      #endif

      size_t bytes_wrote = serial.write("Testing.\n");

      #ifdef __MACH__ // OS X does not have clock_gettime, use clock_get_time
      host_get_clock_service(mach_host_self(), CALENDAR_CLOCK, &cclock);
      clock_get_time(cclock, &mts);
      mach_port_deallocate(mach_task_self(), cclock);
      end.tv_sec = mts.tv_sec;
      end.tv_nsec = mts.tv_nsec;

      #else
      clock_gettime(CLOCK_REALTIME, &end);
      #endif

      end.tv_sec -= start.tv_sec;
      end.tv_nsec -= start.tv_nsec;
      printf("write: %05lu.%09lu\n", end.tv_sec, end.tv_nsec);

      #ifdef __MACH__ // OS X does not have clock_gettime, use clock_get_time
      host_get_clock_service(mach_host_self(), CALENDAR_CLOCK, &cclock);
      clock_get_time(cclock, &mts);
      mach_port_deallocate(mach_task_self(), cclock);
      start.tv_sec = mts.tv_sec;
      start.tv_nsec = mts.tv_nsec;

      #else
      clock_gettime(CLOCK_REALTIME, &start);
      #endif

      std::string result = serial.readline();
      #ifdef __MACH__ // OS X does not have clock_gettime, use clock_get_time
      host_get_clock_service(mach_host_self(), CALENDAR_CLOCK, &cclock);
      clock_get_time(cclock, &mts);
      mach_port_deallocate(mach_task_self(), cclock);
      end.tv_sec = mts.tv_sec;
      end.tv_nsec = mts.tv_nsec;

      #else
      clock_gettime(CLOCK_REALTIME, &end);
      #endif


      end.tv_sec -= start.tv_sec;
      end.tv_nsec -= start.tv_nsec;
      printf("read: %05lu.%09lu\n", end.tv_sec, end.tv_nsec);
      
      if (result == string("Testing.\n")) {
      }
      else {
        std::cout << ">" << count << ">" << bytes_wrote << ">";
        std::cout << result.length() << "<" << result << std::endl;
        cout << "No" << endl;
      }
        
      count += 1;
      boost::this_thread::sleep(boost::posix_time::milliseconds(100));
    }
    
    return 0;
}

int main(int argc, char **argv) {
  // try {
    return run(argc, argv);
  // } catch (std::exception &e) {
  //   std::cerr << "Unhandled Exception: " << e.what() << std::endl;
  // }
}
