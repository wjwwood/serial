# # ash_gti's dumb downed makefile so I can more easily test stuff
# CXX=clang++
# CXXFLAGS=-g -I./include -ferror-limit=5 -O3 -Wall -Weffc++ -pedantic -pedantic-errors -Wextra  -Wall -Waggregate-return -Wcast-align -Wcast-qual  -Wchar-subscripts  -Wcomment -Wconversion -Wdisabled-optimization -Wfloat-equal  -Wformat  -Wformat=2 -Wformat-nonliteral -Wformat-security  -Wformat-y2k -Wimplicit  -Wimport  -Winit-self  -Winline -Winvalid-pch   -Wlong-long -Wmissing-braces -Wmissing-field-initializers -Wmissing-format-attribute   -Wmissing-include-dirs -Wmissing-noreturn -Wpacked -Wparentheses  -Wpointer-arith -Wredundant-decls -Wreturn-type -Wsequence-point  -Wshadow -Wsign-compare  -Wstack-protector -Wstrict-aliasing -Wstrict-aliasing=2 -Wswitch  -Wswitch-default -Wswitch-enum -Wtrigraphs  -Wuninitialized -Wunknown-pragmas  -Wunreachable-code -Wunused -Wunused-function  -Wunused-label  -Wunused-parameter -Wunused-value  -Wunused-variable  -Wvariadic-macros -Wvolatile-register-var  -Wwrite-strings
# 
# test: tests/serial_tests.o src/serial.o src/impl/unix.o
# 	$(CXX) -o test tests/serial_tests.o src/serial.o src/impl/unix.o
ifdef ROS_ROOT
include $(shell rospack find mk)/cmake.mk
else
include serial.makefile
endif
