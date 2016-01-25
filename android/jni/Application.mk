# NDK_TOOLCHAIN_VERSION := clang

ifeq ($(NDK_DEBUG), 1)
APP_OPTIM := debug
else
APP_OPTIM := release
endif

APP_PLATFORM := android-16
APP_ABI := armeabi-v7a x86
APP_STL := gnustl_static
#APP_CFLAGS := -std=gnu11
APP_CPPFLAGS := -fexceptions