# Serial Communication Library

## Note about Repository Changes

If you had the serial library checked previously and now those settings seem broken, it is because I recently reorganized the serial repository.  I wanted these checkouts to break so that you were forced to update.

If you previously were using the `master` branch, that version is now under the git tag `v0.1`.

If you were previously using the `boostless` branch, that version is now tagged as `v0.2` or `boostless`.

If you were previously using the `new_api` branch, that has become the new `master` branch and is also tagged `v1.0.1`.

## Documentation

http://wjwwood.github.com/serial/docs/v1.0.1/index.html

## Dependencies

* CMake, for the build system: http://www.cmake.org/
* (Optional) ROS: http://ros.org/wiki

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
    cd some_ros_workspace
    rosws init . /opt/ros/fuerte # Replace the path here
    source setup.bash

Add the serial unary stack to your ROS workspace:
    
    rosws merge https://raw.github.com/wjwwood/serial/80c0/serial.rosinstall
    
Rerun rosinstall to fetch it:
    
    rosinstall .
    source setup.bash

Build the unary stack:

    rosmake serial --rosdep-install

Run the example:

    rosrun serial serial_example

Use it in your ROS pkg by adding this line to your `manifest.xml`:

    <depend package="serial" />

## License

The BSD License

Copyright (c) 2012 William Woodall

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
