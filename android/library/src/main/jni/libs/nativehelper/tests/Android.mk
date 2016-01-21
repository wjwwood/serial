# Build the unit tests.
LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

# Target unit test.

include $(CLEAR_VARS)
LOCAL_MODULE := JniInvocation_test
LOCAL_CLANG := true
LOCAL_SRC_FILES := JniInvocation_test.cpp
LOCAL_SHARED_LIBRARIES := \
    libnativehelper

include external/libcxx/libcxx.mk

LOCAL_MULTILIB := both
LOCAL_MODULE_STEM_32 := $(LOCAL_MODULE)32
LOCAL_MODULE_STEM_64 := $(LOCAL_MODULE)64
include $(BUILD_NATIVE_TEST)

# Host unit test.

include $(CLEAR_VARS)
LOCAL_MODULE := JniInvocation_test
LOCAL_CLANG := true
LOCAL_SRC_FILES := JniInvocation_test.cpp
LOCAL_SHARED_LIBRARIES := \
    libnativehelper

include external/libcxx/libcxx.mk

LOCAL_MULTILIB := both
LOCAL_MODULE_STEM_32 := $(LOCAL_MODULE)32
LOCAL_MODULE_STEM_64 := $(LOCAL_MODULE)64
include $(BUILD_HOST_NATIVE_TEST)
