#!/bin/bash

ARGS=$(getopt -o "d" -l "debug" -n "build-jni.sh" -- "$@");

eval set -- "$ARGS";

JNI_ARGS="NDK_LIBS_OUT=../jniLibs"

while true; do
  case "$1" in
    -d|--debug)
      shift
      JNI_ARGS="$JNI_ARGS NDK_DEBUG=1"
      break;
      ;;
    --)
      shift
      ;;
  esac
done

# Remove '--'
shift

ndk-build $JNI_ARGS