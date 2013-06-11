^^^^^^^^^^^^^^^^^^^^^^^^^^^^
Changelog for package serial
^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Forthcoming
-----------
* Potential timing fix for read and write.
  Fixes #27
* Update list of exceptions thrown from constructor
* alloca.h does not exist on OpenBSD either

1.1.3 (2013-01-09)
------------------
* install headers

1.1.2 (2012-12-14)
------------------
* Fix buildtool depends

1.1.1 (2012-12-03)
------------------
* Small CMakeLists.txt touch ups.
* Removed rt linking on OS X. Fixes #24.

1.1.0 (2012-10-24)
------------------
* Added a link option specifically for Linux
* Releasing version 1.1.0 of serial
  Converted the build system to caktin
* Updating the README.md
* Adding catkin build files and a new Makefile.
* Removing old build files.
* Fixed linking on Linux and OS X.
* Updated the tests for the new Serial API.
* Added the `rt` library to the `serial_LIBRARIES` variable when on Linux.
* Adding explicit keyword to Timeout constructor to prevent accidental conversion from int types.
* Merge pull request #21 from davidhodo/master
  add option to not use clang so i can use this in bitbake
* should have been if NOT defined
* add option to not use Clang, even if it is installed
* Adding void return type for Arduino example in examples/serial_example.cc.  Fixes #20
* Adding baudrates: 1000000, 11520000, 2000000, 2500000, 3000000, 3500000, and 4000000 to the unix.cc implementation. This is an attempting to work around issue #18, needs testing on a non-USB serial adapter that can handle higher baudrates.
* Clean up of the build system.  Added rt and util in certain places so compiling the tests on Linux succeed, which should fix #19.  Also removed over verbose warnings on Linux when build stand-alone.
* Silly little bug in the custom baud rate code.  Should fix #18.
* Fixing a bug in the unix write code which may have prevented it from writing in some cases, and updating some of the comments and error messages in the write function that still read like the read function
* Fixing compile and link problems in Linux and ROS
* Fixes to flowcontrol that solve a wierd bug on Linux and fix an oversight on Windows.
* Fixing warning in Linux
* Fixing a compile problem in Linux that got introduced in the Windows testing.
* Adding link to the rosinstall file
* Updating docs and adding a rosinstall file
* Merge commit 'e9999c9c7c801b8c60569f1b48792af8c050eac4'
* Updating documentation
* Update master
* Merge branch 'new_api'
* Merging boostless into master
* Changes after testing on Windows.  Everything seems to work, but it could use some more vetting.
* Fixing some warnings from compiling on Linux
* Fixed some issues with the copy constructors
* A small change to quiet warnings in linux
* Disabling more copy constructors
* Disabling copy constructors on the expections
* Sorted out the stdint stuff by including a header from the v8 project.
* Still working on std types
* Merge branch 'new_api' of github.com:wjwwood/serial into new_api
  Conflicts:
  .gitignore
* Removing a vestigial file
* Adding initial windows support.
* Syntax fix
* Adjustments to the read timeouts
* Updating the unix implementation to support the more sophisticated timeout system.
* Adding a convenience function for generating simple Timeouts based purely on the total time until a timeout occurs with no regard to inter byte timeout or timeout multipliers.
* Fixes #16.
* Changes to the documentation generation mechanism
* More updates to the documentation and new api after comments from john and michael.
* Updating the API to fix some inconsistencies
* fixed library name - hardcoded not based on project name
* fixing bug in the Findserial.cmake file, reported by David Hodo.
* Fixed some warnings from clang
* Changing the throw to add file and line number
* Missing a linked library on linux
* Merge branch 'boostless' of github.com:wjwwood/serial into boostless
* Adding my basic tests
* Merge branch 'boostless' of github.com:wjwwood/serial into boostless
* Removing a Clang specific compiler warning.
* Removing a Clang specific compiler warning.
* Enabling warnings! They catch things, like Pokemon
  And bad casts and bad constructors and and … Ya, Warnings good!
* Fixing bad C++ constructors.
  I am not sure if this is related to the random crashes on Linux, but the wrong C++ std::string constructors were being called because of bad use of c-style array dereferencing. The correct C++ std::string constructors are being called now.
* Configuring CMake to link against thread libraries
  This is used to link against pthread and friends that are used for threading and mutex's.
* Tracking down segfaults on Linux
* Fixing a reading buffer bug and expanding the example
* fixing some stuff in unix.cc to match changes in enums
* wtf: 'Users/william/devel/atrv_ws/serial/include/serial/serial.h:82: error: stray ‘\303’ in program', fixed...
* Merge branch 'boostless' of https://github.com/wjwwood/serial into boostless
  Conflicts:
  include/serial/serial.h
* Some style changes
* Read/write seem to be working on linux, need to test on OS X.
* Removed serial listener, reworking the example, completely removed boost.  Builds on my laptop with boost uninstalled.
* Removing serial listener, next make sure I can compile without boost completely.
* Testing my changes to make sure they are consistant.
* Testing the new buffer
* Merge branch 'stringbuffer' into boostless
* Adding some linux specific fixes. fixes #13
* Merge branch 'boostless' into stringbuffer
* Some small changes to fix things discovered while testing serial listener.
* Change the buffer to a generic C++ std::string
* fixing empty token bug, and adding some debugging stuff.
* Serial listener tests complete and working
* Reverting the deletion of the baud rate validation.
* Got some serial listener tests working
* Merge branch 'boostless' of github.com:wjwwood/serial into boostless
* Validate bauds and add the ability to set custom bauds.
* Still working on the tests
* Merge branch 'boostless' of github.com:wjwwood/serial into boostless
* Merge branch 'boostless' of https://github.com/wjwwood/serial into boostless
* Found an error in my buffering code and added a timeout
  to write
* Updating tests while on the mac
* Updating tests while testing on linux
* Updating the error handling.
* Merge branch 'boostless' of github.com:wjwwood/serial into boostless
* Updating the example to allow variable bauds
* Fixing message in serial_example.
* working on tests and stuff
* Merge pull request #10 from natem345/patch-1
  Minor readme additions
* Updated readme: got rosinstall error without space between - and git, and added warning about environment var change
* Trying to do a first pass to make this thread safe… not sure…
* Fixing timeouts, there were not functioning correctly.
* Fixing another issue with the default handler setter.
* Fixing serial manifest.xml for ROS and replacing an accidental missing function
* Fixing exception handler setting function
* Merge branch 'boostless' of https://github.com/wjwwood/serial into boostless
  Conflicts:
  include/serial/serial_listener.h
* Removing dead code.
* Merge branch 'boostless' of github.com:wjwwood/serial into boostless
  Conflicts:
  include/serial/serial_listener.h
  src/impl/unix.cc
* Adding in an internal buffer to Serial, this is independent of the SerialImpl and buffers reads to help performance.
  Also correcting my styles to match the style guide and adding in some documentation.
* Changed the stopListening function, now it no longer removes filters, just stops listening and cleans the data buffer of partial messages.
* Fixing compile errors with serial listener.
* Fixing Findserial.cmake and updating serial_listener.h to remove unused functions.
* Fixing a warning.
* Merge branch 'boostless' of github.com:wjwwood/serial into boostless
* Correcting some bad logic and making my test run forever to try to judge some basic performance characteristics.
* Updating ROS make system.
* Changed how the memory is allocated in the read operation and changed how the timeouts function. If the timeout is -1, then it will block until it finishes the reads.
* Adding doxygen support.
* Fixed all of the warnings from serial_listener
* Cleanup of code base
* Removing vestigial files.
* Fixing a number of warnings in the serial library. The SerialListener still needs some love.
* Enabling warnings, to make sure things are good.
* Correcting the behavior of Serial::setPort and anything that modifies stuff related to baud/parity/etc.
* Merge branch 'boostless' of github.com:wjwwood/serial into boostless
  Conflicts:
  include/serial/impl/unix.h
  src/impl/unix.cc
  src/serial.cc
* Fixed some issues I found while testing my code, also implemented the drain, flush, set/send Break, get {CTS, RI, CD, DSR}, set {RTS, DTR}
* Quieting tests for now
* Fixed some memory problems on destruction.  Serial listener maybe working, serial's read doesn't seem to return anything or block at all.
* Everything builds, but haven't tested it on a serial device.
* Merge branch 'boostless' of github.com:wjwwood/serial into boostless
  Conflicts:
  include/serial/impl/unix.h
  include/serial/serial.h
  serial.cmake
  src/impl/unix.cc
  src/serial.cc
* Serial Listener changes compile against the example reference, time to merge with John.
* Still working on Serial listener refactor, not working, fixing to make big changes.
* Removing some unenessacary repetition
* Implemented write, readline, readlines and available
* Adding in my unix implementation. reconfigurePort and read should probably work, but I haven't tested them yet. I am going to work on write later.
* Updating manifest.xml to have proper exports.
* Working on pimpl implementation of serial.
* working on new boostless serial with a pimpl setup
* Finished serial_listener.  But serial is seriously inefficient, I need to investigate this before moving on.
* Fixing to remove once type functions and re-implement them
* Cleaning up binary inclusion.
* Merge branch 'master' of github.com:wjwwood/serial into serial_listener
* Adding a cmake option to prevent ROS building.
* Still working on SerialListener addition.
* Adding files for serial_listener.
* Fixed an installation error.
* Fixed ROS building.
* Update README.md
* Adding ROS stuff to the read me
* Update README.md
* Adding a readme
* Adding support to build as a ROS unary stack or as a standalone library.
* Added a policy to suppress a warning in Windows.
* Fixing a bug/patch submitted by d.suvorov90@gmail.com for linking on Windows.
* Revamped the build system a bit to make it more friendly to embeding in other projects.
* Removed const's before the return type on the methods, this isn't correct and produced warnings in linux.
* Made readline smarter and less complicated.
* Adding a read_until option where you can insert a deliminator.
* More fixes related to non-blocking read.
* Fixed nonblocking read problem.
* Added the set and get port functions, probably should have been in there from the start...
* Fixed the Findserial.cmake install path override...
* Added the ability to override the install path for the Findserial.cmake file.
* Added a Findserial.cmake file to be instaleld to help with finding the library.  Also, added an uninstall target.
* Added isOpen, and credited John Harrison as an author as well.
* Misspelled parity....
* PARTY_* related fixes. Compiles and works on OS X and Windows, needs to be tested on Linux.
* Fixes for windows.
* Last merge undid some of the changes for windows compatability.
* Merge branch 'master' of github.com:wjwwood/serial
  Conflicts:
  include/serial.h
  src/serial.cpp
* Look ma, no pointers! (directly controlled, also now there are no memory leaks)
* Merge branch 'master' of github.com:wjwwood/serial
* Fixed some potential memory leaks.  Also, added a possible fix for the PARTIY_NONE debackle.
* Fixed some potential memory leaks.  Also, added a possible fix for the PARTIY_NONE debackle.
* More windows changes.
* Changes to fix WINDOWSasdklfjal;sjkdfa;lsd
* Added some boost related stuff to cmake to help with windows building.
* Disabled the copy constructor and made the return types all const.
* Cleaned up the library, added a namespace.  Also implemented some suggestions of john's.
* Cleaned up test program a bit.
* Read has been informally tested to work.
* Fixed an issue with setting cmake install prefix externally.
* Added install targets.
* Completely removed ROS dependency, builds with cmake and has a auto cmake && make Makefile.  To build just type make.
* moved files out or stack/package setup.
* Fixed the issue with reading on osx.  writing still needs to be tested, then I am going to remove the ROS dependency.
* Cleaning up cout's
* Serial read is working, but the timeout is a little buggy...  write hasn't been tested and I am going to try this out on linux.
* Simple sync read working.
* Initial commit.
