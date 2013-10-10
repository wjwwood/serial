/*!
 * \file serial/impl/unit-timespec.h
 * \author  Mike Purvis <mpurvis@clearpathrobotics.com>
 * \version 0.1
 *
 * \section LICENSE
 *
 * The MIT License
 *
 * Copyright (c) 2013 William Woodall
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 * \section DESCRIPTION
 *
 * Provides helper and operator functions for concisely and reliably handling
 * timespec instances on unix platforms.
 */

#ifndef SERIAL_IMPL_UNIX_TIMESPEC_H
#define SERIAL_IMPL_UNIX_TIMESPEC_H

#include <time.h>

/*! Smooth over platform variances in getting an accurate timespec
 * representing the present moment. */ 
static inline struct timespec
timespec_now ()
{
  struct timespec ts;
# ifdef __MACH__ // OS X does not have clock_gettime, use clock_get_time
  clock_serv_t cclock;
  mach_timespec_t mts;
  host_get_clock_service(mach_host_self(), CALENDAR_CLOCK, &cclock);
  clock_get_time(cclock, &mts);
  mach_port_deallocate(mach_task_self(), cclock);
  ts.tv_sec = mts.tv_sec;
  ts.tv_nsec = mts.tv_nsec;
# else
  clock_gettime(CLOCK_REALTIME, &ts);
# endif
  return ts;
}

/*! Simple function to normalize the tv_nsec field to [0..1e9), carrying
 * the remainder into the tv_sec field. This will not protect against the
 * possibility of an overflow in the nsec field--proceed with caution. */
static inline void
normalize(struct timespec* ts) {
  while (ts->tv_nsec < 0) {
    ts->tv_nsec += 1e9;
    ts->tv_sec -= 1;
  }
  while (ts->tv_nsec >= 1e9) {
    ts->tv_nsec -= 1e9;
    ts->tv_sec += 1;
  }
}

/*! Return a timespec which is the sum of two other timespecs. This
 * operator only makes logical sense when one or both of the arguments
 * represents a duration. */
static inline timespec
operator+ (const struct timespec &a, const struct timespec &b) {
  struct timespec result = { a.tv_sec + b.tv_sec,
                             a.tv_nsec + b.tv_nsec };
  normalize(&result);
  return result;
}

/*! Return a timespec which is the difference of two other timespecs.
 * This operator only makes logical sense when one or both of the arguments
 * represents a duration. */
static inline timespec
operator- (const struct timespec &a, const struct timespec &b) {
  struct timespec result = { a.tv_sec - b.tv_sec,
                             a.tv_nsec - b.tv_nsec };
  normalize(&result);
  return result;
}

/*! Return a timespec which is a multiplication of a timespec and a positive
 * integer. No overflow protection-- not suitable for multiplications with 
 * large carries, eg a <1s timespec multiplied by a large enough integer 
 * that the result is muliple seconds. Only makes sense when the timespec
 * argument is a duration. */
static inline timespec
operator* (const struct timespec &ts, const size_t mul) {
  struct timespec result = { ts.tv_sec * mul,
                             ts.tv_nsec * mul };
  normalize(&result);
  return result;
}

/*! Return whichever of two timespec durations represents the shortest or most
 * negative period. */
static inline struct timespec
min (const struct timespec &a, const struct timespec &b) {
  if (a.tv_sec < b.tv_sec
     || (a.tv_sec == b.tv_sec && a.tv_nsec < b.tv_nsec)) {
    return a;
  } else {
    return b;
  }
}

/*! Return a timespec duration set from a provided number of milliseconds. */
static struct timespec
timespec_from_millis(const size_t millis) {
  struct timespec result = { 0, millis * 1000000 };
  normalize(&result);
  return result;
}

#endif
