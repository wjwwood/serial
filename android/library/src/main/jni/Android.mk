LOCAL_PATH := $(call my-dir)

$(call import-add-path,$(LOCAL_PATH)/libs)

include $(CLEAR_VARS)

LOCAL_MODULE := serial
LOCAL_C_INCLUDES := \
	$(LOCAL_PATH)/include \
	$(LOCAL_PATH)/libs
	
LOCAL_SRC_FILES := glob.c \
    serial.cc \
    serial_unix.cc \
    serial_jni.cc \
    list_ports_linux.cc \
    jni_utility.cc \
    jni_main.cc

LOCAL_STATIC_LIBRARIES += nativehelper
LOCAL_LDLIBS := -llog -lz

include $(BUILD_SHARED_LIBRARY)

$(call import-module,nativehelper)
