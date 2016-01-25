/*
 * Copyright (C) 2003, 2004, 2005, 2008, 2009, 2010 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE COMPUTER, INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE COMPUTER, INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef JNIUtility_h
#define JNIUtility_h

#include <jni.h>
#include <stdio.h>
#include <string>
#include <list>
#include <memory>

#include <nativehelper/ScopedLocalRef.h>

#include "log.h"

#define FIND_CLASS_RETURN_GLOBAL_REF	0x1
#define FIND_CLASS_USE_CLASS_LOADER		0x2

/**
 * Helper method for checking java exceptions
 * @return true if an exception occurred.
 */
bool checkException(JNIEnv* env);
std::string jstringToStdString(JNIEnv* env, jstring jstr);
jstring stdStringToJstring(JNIEnv* env, const std::string& str);
std::list<std::string> jstringArrayToStdStringList(jobjectArray jstringArray);
bool jbooleanTobool(jboolean jBoolean);

JNIEnv* getJNIEnv();
JavaVM* getJavaVM();
void setJavaVM(JavaVM*);
jclass findClass(const char* name, int flags = FIND_CLASS_USE_CLASS_LOADER);

inline std::string jstringToStdString(jstring jstr)
{
    return jstringToStdString(getJNIEnv(), jstr);
}
inline jstring stdStringToJstring(const std::string& str)
{
    return stdStringToJstring(getJNIEnv(), str);
}

jobjectArray createStringArray(JNIEnv* env, int size);

template <typename T> struct JNICaller;

template<> struct JNICaller<void> {
    static void callV(jobject obj, jmethodID mid, va_list args)
    {
        getJNIEnv()->CallVoidMethodV(obj, mid, args);
    }
    static void callStaticV(jclass cls, jmethodID mid, va_list args)
    {
        return getJNIEnv()->CallStaticVoidMethodV(cls, mid, args);
    }
    static void returnEmpty()
    {
    }
};

#define DEFINE_CALLER_TYPE(_jtype, _jname, _default) \
    template<> struct JNICaller<_jtype> { \
        static _jtype callV(jobject obj, jmethodID mid, va_list args) \
        { \
            return getJNIEnv()->Call##_jname##MethodV(obj, mid, args); \
        } \
        static _jtype get(jobject obj, jfieldID fid) \
        { \
            return getJNIEnv()->Get##_jname##Field(obj, fid); \
        } \
        static void set(jobject obj, jfieldID fid, _jtype value) \
        { \
            getJNIEnv()->Set##_jname##Field(obj, fid, value); \
        } \
        static _jtype callStaticV(jclass cls, jmethodID mid, va_list args) \
        { \
            return getJNIEnv()->CallStatic##_jname##MethodV(cls, mid, args); \
        } \
        static _jtype returnEmpty() \
        { \
            return _default; \
        } \
    };

    DEFINE_CALLER_TYPE(jobject, Object, 0)
    DEFINE_CALLER_TYPE(jboolean, Boolean, false)
    DEFINE_CALLER_TYPE(jbyte, Byte, 0)
    DEFINE_CALLER_TYPE(jchar, Char, 0)
    DEFINE_CALLER_TYPE(jshort, Short, 0)
    DEFINE_CALLER_TYPE(jint, Int, 0)
    DEFINE_CALLER_TYPE(jlong, Long, 0)
    DEFINE_CALLER_TYPE(jfloat, Float, 0)
    DEFINE_CALLER_TYPE(jdouble, Double, 0)


template<> struct JNICaller<std::string> {
    static std::string callA(jobject obj, jmethodID mid, jvalue* args)
    {
        jobject jstr = getJNIEnv()->CallObjectMethodA(obj, mid, args);
        std::string ret = jstringToStdString(getJNIEnv(), (jstring)jstr);
        getJNIEnv()->DeleteLocalRef(jstr);
        return ret;
    }
    static std::string callV(jobject obj, jmethodID mid, va_list args)
    {
        jobject jstr = getJNIEnv()->CallObjectMethodV(obj, mid, args);
        std::string ret = jstringToStdString(getJNIEnv(), (jstring)jstr);
        getJNIEnv()->DeleteLocalRef(jstr);
        return ret;
    }
    static std::string get(jobject obj, jfieldID fid)
    {
        jobject jstr = getJNIEnv()->GetObjectField(obj, fid);
        std::string ret = jstringToStdString(getJNIEnv(), (jstring)jstr);
        getJNIEnv()->DeleteLocalRef(jstr);
        return ret;
    }
    static void set(jobject obj, jfieldID fid, std::string value)
    {
        jobject jstr = stdStringToJstring(getJNIEnv(), value);
        getJNIEnv()->SetObjectField(obj, fid, jstr);
        getJNIEnv()->DeleteLocalRef(jstr);
    }
    static std::string callStaticV(jclass cls, jmethodID mid, va_list args)
    {
        jobject jstr = getJNIEnv()->CallStaticObjectMethodV(cls, mid, args);
        std::string ret = jstringToStdString(getJNIEnv(), (jstring)jstr);
        getJNIEnv()->DeleteLocalRef(jstr);
        return ret;
    }
    static std::string returnEmpty()
    {
        return std::string();
    }
};

template<typename T> T callJNIMethodIDV(jobject obj, jmethodID mid, va_list args)
{
    return JNICaller<T>::callV(obj, mid, args);
}

template<typename T>
static T callJNIMethodV(jobject obj, const char* name, const char* sig, va_list args)
{
    JavaVM* jvm = getJavaVM();
    JNIEnv* env = getJNIEnv();

    if (obj && jvm && env) {
        ScopedLocalRef<jclass> cls(env, env->GetObjectClass(obj));
        if (cls.get()) {
            jmethodID mid = env->GetMethodID(cls.get(), name, sig);
            if (mid) {
                // Avoids references to cls without popping the local frame.
                return JNICaller<T>::callV(obj, mid, args);
            } else {
                LOGE("Could not find method %s for %p", name, cls.get());
            }
            env->ExceptionDescribe();
        } else {
            env->ExceptionDescribe();
            LOGE("Could not find class for %p", obj);
        }
    }

    return JNICaller<T>::returnEmpty();
}

template<typename T>
T callJNIMethod(jobject obj, const char* methodName, const char* methodSignature, ...)
{
    va_list args;
    va_start(args, methodSignature);

    T result = callJNIMethodV<T>(obj, methodName, methodSignature, args);

    va_end(args);

    return result;
}

template<typename T>
T callJNIMethodID(jobject obj, jmethodID mid, ...)
{
    va_list args;
    va_start(args, mid);

    T result = callJNIMethodIDV<T>(obj, mid, args);

    va_end(args);

    return result;
}

template<typename T>
T callJNIStaticMethod(jclass cls, jmethodID mid, ...)
{
    va_list args;
    va_start(args, mid);

    T result = JNICaller<T>::callStaticV(cls, mid, args);

    va_end(args);

    return result;
}

template<typename T>
T callJNIStaticMethod(jclass cls, const char* methodName, const char* methodSignature, ...)
{
    JavaVM* jvm = getJavaVM();
    JNIEnv* env = getJNIEnv();
    va_list args;

    va_start(args, methodSignature);

    if (cls && jvm && env) {
        jmethodID mid = env->GetStaticMethodID(cls, methodName, methodSignature);
        if (mid)
            return JNICaller<T>::callStaticV(cls, mid, args);
        else {
            env->ExceptionDescribe();
            LOGE("Could not find method: %s for %p", methodName, cls);
        }
    }

    va_end(args);

    return JNICaller<T>::returnEmpty();
}

void callJNIVoidMethod(jobject obj, const char* methodName, const char* methodSignature, ...);
void callJNIVoidMethodID(jobject obj, jmethodID mid, ...);

void callJNIStaticVoidMethod(const char* className, const char* methodName, const char* methodSignature, ...);

template<typename T>
static void setFieldValue(jobject obj, const char* name, const char* sig, T value)
{
    JavaVM* jvm = getJavaVM();
    JNIEnv* env = getJNIEnv();

    if (obj && jvm && env) {
        ScopedLocalRef<jclass> cls(env, env->GetObjectClass(obj));
        if (cls.get()) {
            jfieldID fid = env->GetFieldID(cls.get(), name, sig);
            if (fid) {
                // Avoids references to cls without popping the local frame.
                return JNICaller<T>::set(obj, fid, value);
            } else {
                LOGE("Could not find field %s for %p", name, cls.get());
            }
            env->ExceptionDescribe();
        } else {
            env->ExceptionDescribe();
            LOGE("Could not find class for %p", name, obj);
        }
    }
}

template<typename T>
static T getFieldValue(jobject obj, const char* name, const char* sig)
{
    JavaVM* jvm = getJavaVM();
    JNIEnv* env = getJNIEnv();

    if (obj && jvm && env) {
        ScopedLocalRef<jclass> cls(env, env->GetObjectClass(obj));
        if (cls.get()) {
            jfieldID fid = env->GetFieldID(cls.get(), name, sig);
            if (fid) {
                // Avoids references to cls without popping the local frame.
                return JNICaller<T>::get(obj, fid);
            } else {
                LOGE("Could not find field %s", name);
            }
            env->ExceptionDescribe();
        } else {
            env->ExceptionDescribe();
            LOGE("Could not find class for %p", name, obj);
        }
    }
    return JNICaller<T>::returnEmpty();
}

jobject toJavaBoolean(bool value);
jobject toJavaLong(jlong value);
jobject toJavaInt(jint value);
jobject newJavaObject(const char* className);

#endif // JNIUtility_h
