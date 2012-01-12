CXX=clang++
CXXFLAGS=-g -I./include

test: tests/serial_tests.o src/serial.o src/impl/unix.o
	$(CXX) -o test tests/serial_tests.o src/serial.o src/impl/unix.o

# ifdef ROS_ROOT
# include $(shell rospack find mk)/cmake.mk
# else
# include serial.makefile
# endif
