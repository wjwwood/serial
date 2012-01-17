# Serial Communication Library

## Documentation

Coming Soon!

## Dependencies

* CMake, for the build system: http://www.cmake.org/
* Boost, for threading: http://www.boost.org/
* (Optional) ROS

## Stand Alone Installation

Get the source:

    git clone git://github.com/wjwwood/serial.git
    cd serial

Compile the code:

    make

Or run cmake youself:

    mkdir build && cd build
    cmake ..
    make

Install the code (UNIX):

    make
    sudo make install

Uninstall the code (UNIX):

    make
    sudo make uninstall

Build the documentation:

    Comming Soon!

## Using within ROS workspace

Setup workspace (skip if you already have one):

    mkdir some_ros_workspace
    rosws init some_ros_workspace
    cd some_ros_workspace
    source setup.bash
    
Add the rosinstall entry for this stack:
    
    echo "- git: {local-name: serial, uri: 'https://github.com/wjwwood/serial.git', version: 'master'}" >> .rosinstall
    
Rerun rosinstall (note this will change your $ROS_PACKAGE_PATH):
    
    rosinstall .
    source setup.bash

Build the unary stack:

    rosmake serial --rosdep-install

Run the example:

    rosrun serial serial_example

## License

The BSD License

Copyright (c) 2011 William Woodall

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
