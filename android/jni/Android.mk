LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := serial
LOCAL_C_INCLUDES := \
    $(LOCAL_PATH)/include\
    $(LOCAL_PATH)/../../include
	
LOCAL_SRC_FILES := glob.c \
    ../../src/serial.cc \
    ../../src/impl/unix.cc \
    ../../src/impl/list_ports/list_ports_linux.cc

include $(BUILD_SHARED_LIBRARY)
