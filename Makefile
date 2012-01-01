ifdef ROS_ROOT
include $(shell rospack find mk)/cmake.mk
else
include serial.mk
endif
