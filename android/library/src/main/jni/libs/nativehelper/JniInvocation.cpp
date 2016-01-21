/*
 * Copyright (C) 2013 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "JniInvocation.h"

#include <dlfcn.h>
#include <stdlib.h>
#include <string.h>

#include <cstddef>

#define LOG_TAG "JniInvocation"
#include "cutils/log.h"

#ifdef HAVE_ANDROID_OS
#include "cutils/properties.h"
#endif

JniInvocation* JniInvocation::jni_invocation_ = NULL;

JniInvocation::JniInvocation() :
    handle_(NULL),
    JNI_GetDefaultJavaVMInitArgs_(NULL),
    JNI_CreateJavaVM_(NULL),
    JNI_GetCreatedJavaVMs_(NULL) {

  LOG_ALWAYS_FATAL_IF(jni_invocation_ != NULL, "JniInvocation instance already initialized");
  jni_invocation_ = this;
}

JniInvocation::~JniInvocation() {
  jni_invocation_ = NULL;
  if (handle_ != NULL) {
    dlclose(handle_);
  }
}

#ifdef HAVE_ANDROID_OS
static const char* kLibrarySystemProperty = "persist.sys.dalvik.vm.lib.2";
static const char* kDebuggableSystemProperty = "ro.debuggable";
static const char* kDebuggableFallback = "0";  // Not debuggable.
#endif
static const char* kLibraryFallback = "libart.so";

const char* JniInvocation::GetLibrary(const char* library) {
#ifdef HAVE_ANDROID_OS
  char default_library[PROPERTY_VALUE_MAX];

  char debuggable[PROPERTY_VALUE_MAX];
  property_get(kDebuggableSystemProperty, debuggable, kDebuggableFallback);

  if (strcmp(debuggable, "1") != 0) {
    // Not a debuggable build.
    // Do not allow arbitrary library. Ignore the library parameter. This
    // will also ignore the default library, but initialize to empty string
    // for cleanliness.
    library = kLibraryFallback;
    default_library[0] = 0;
  } else {
    // Debuggable build.
    // Accept the library parameter. For the case it is NULL, load the default
    // library from the system property.
    property_get(kLibrarySystemProperty, default_library, kLibraryFallback);
  }
#else
  const char* default_library = kLibraryFallback;
#endif
  if (library == NULL) {
    library = default_library;
  }

  return library;
}

bool JniInvocation::Init(const char* library) {
  library = GetLibrary(library);

  handle_ = dlopen(library, RTLD_NOW);
  if (handle_ == NULL) {
    if (strcmp(library, kLibraryFallback) == 0) {
      // Nothing else to try.
      ALOGE("Failed to dlopen %s: %s", library, dlerror());
      return false;
    }
    // Note that this is enough to get something like the zygote
    // running, we can't property_set here to fix this for the future
    // because we are root and not the system user. See
    // RuntimeInit.commonInit for where we fix up the property to
    // avoid future fallbacks. http://b/11463182
    ALOGW("Falling back from %s to %s after dlopen error: %s",
          library, kLibraryFallback, dlerror());
    library = kLibraryFallback;
    handle_ = dlopen(library, RTLD_NOW);
    if (handle_ == NULL) {
      ALOGE("Failed to dlopen %s: %s", library, dlerror());
      return false;
    }
  }
  if (!FindSymbol(reinterpret_cast<void**>(&JNI_GetDefaultJavaVMInitArgs_),
                  "JNI_GetDefaultJavaVMInitArgs")) {
    return false;
  }
  if (!FindSymbol(reinterpret_cast<void**>(&JNI_CreateJavaVM_),
                  "JNI_CreateJavaVM")) {
    return false;
  }
  if (!FindSymbol(reinterpret_cast<void**>(&JNI_GetCreatedJavaVMs_),
                  "JNI_GetCreatedJavaVMs")) {
    return false;
  }
  return true;
}

jint JniInvocation::JNI_GetDefaultJavaVMInitArgs(void* vmargs) {
  return JNI_GetDefaultJavaVMInitArgs_(vmargs);
}

jint JniInvocation::JNI_CreateJavaVM(JavaVM** p_vm, JNIEnv** p_env, void* vm_args) {
  return JNI_CreateJavaVM_(p_vm, p_env, vm_args);
}

jint JniInvocation::JNI_GetCreatedJavaVMs(JavaVM** vms, jsize size, jsize* vm_count) {
  return JNI_GetCreatedJavaVMs_(vms, size, vm_count);
}

bool JniInvocation::FindSymbol(void** pointer, const char* symbol) {
  *pointer = dlsym(handle_, symbol);
  if (*pointer == NULL) {
    ALOGE("Failed to find symbol %s: %s\n", symbol, dlerror());
    dlclose(handle_);
    handle_ = NULL;
    return false;
  }
  return true;
}

JniInvocation& JniInvocation::GetJniInvocation() {
  LOG_ALWAYS_FATAL_IF(jni_invocation_ == NULL,
                      "Failed to create JniInvocation instance before using JNI invocation API");
  return *jni_invocation_;
}

extern "C" jint JNI_GetDefaultJavaVMInitArgs(void* vm_args) {
  return JniInvocation::GetJniInvocation().JNI_GetDefaultJavaVMInitArgs(vm_args);
}

extern "C" jint JNI_CreateJavaVM(JavaVM** p_vm, JNIEnv** p_env, void* vm_args) {
  return JniInvocation::GetJniInvocation().JNI_CreateJavaVM(p_vm, p_env, vm_args);
}

extern "C" jint JNI_GetCreatedJavaVMs(JavaVM** vms, jsize size, jsize* vm_count) {
  return JniInvocation::GetJniInvocation().JNI_GetCreatedJavaVMs(vms, size, vm_count);
}
