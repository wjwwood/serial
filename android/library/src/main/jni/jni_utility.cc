/*
 * Copyright (C) 2003, 2004, 2005, 2007, 2008, 2009, 2010 Apple Inc. All rights reserved.
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

#include <jni_utility.h>

static JavaVM* jvm = 0;
static jobject gClassLoader;
static jmethodID gFindClassMethod;

static jclass gStringClass = 0;
static jmethodID gGetBytesMid = 0;
static jmethodID gNewStringMid = 0;

static jobject gTrue = 0;
static jobject gFalse = 0;
static jclass gLongClass = 0;
static jmethodID gLongValueOf = 0;
static jclass gIntClass = 0;
static jmethodID gIntValueOf = 0;

void setupGlobalClassLoader()
{
    JNIEnv* env = getJNIEnv();
    ScopedLocalRef<jclass> coreJniClass(env, env->FindClass("serial/Serial"));
    ScopedLocalRef<jclass> classClass(env, env->GetObjectClass(coreJniClass.get()));
    ScopedLocalRef<jclass> classLoaderClass(env, env->FindClass("java/lang/ClassLoader"));
    jmethodID getClassLoaderMethod = env->GetMethodID(classClass.get(), "getClassLoader",
            "()Ljava/lang/ClassLoader;");
    ScopedLocalRef<jobject> classLoader(env, env->CallObjectMethod(coreJniClass.get(), getClassLoaderMethod));
    gClassLoader = env->NewGlobalRef(classLoader.get());
    //LOGV("gClassLoader is %p", gClassLoader);
    gFindClassMethod = env->GetMethodID(classLoaderClass.get(), "findClass",
            "(Ljava/lang/String;)Ljava/lang/Class;");
    //LOGV("gFindClassMethod is %p", gFindClassMethod);

    ScopedLocalRef<jclass> stringClass(env, env->FindClass("java/lang/String"));
    gStringClass = (jclass)env->NewGlobalRef(stringClass.get());
    //LOGV("gStringClass is %p", gStringClass);
    gGetBytesMid = env->GetMethodID (gStringClass, "getBytes", "()[B");
    //LOGV("gGetBytesMid is %p", gGetBytesMid);
    gNewStringMid = env->GetMethodID(gStringClass, "<init>", "([B)V");
    //LOGV("gNewStringMid is %p", gNewStringMid);

    ScopedLocalRef<jclass> booleanCls(env, env->FindClass("java/lang/Boolean"));
    ScopedLocalRef<jobject> t(env, callJNIStaticMethod<jobject>(
            booleanCls.get(), "valueOf", "(Z)Ljava/lang/Boolean;", JNI_TRUE));
    gTrue = env->NewGlobalRef(t.get());
    //LOGV("gTrue is %p", gTrue);
    ScopedLocalRef<jobject> f(env, callJNIStaticMethod<jobject>(
                    booleanCls.get(), "valueOf", "(Z)Ljava/lang/Boolean;", JNI_FALSE));
    gFalse = env->NewGlobalRef(f.get());
    //LOGV("gFalse is %p", gFalse);

    ScopedLocalRef<jclass> longCls(env, env->FindClass("java/lang/Long"));
    gLongClass = (jclass)env->NewGlobalRef(longCls.get());
    gLongValueOf = env->GetStaticMethodID(gLongClass, "valueOf", "(J)Ljava/lang/Long;");

    ScopedLocalRef<jclass> intCls(env, env->FindClass("java/lang/Integer"));
    gIntClass = (jclass)env->NewGlobalRef(intCls.get());
    gIntValueOf = env->GetStaticMethodID(gIntClass, "valueOf", "(I)Ljava/lang/Integer;");
}

jclass findClass(const char* name, int flags /*= FIND_CLASS_USE_CLASS_LOADER*/)
{
    JNIEnv *env = getJNIEnv();
    jclass cls;
    if ((flags & FIND_CLASS_USE_CLASS_LOADER) == FIND_CLASS_USE_CLASS_LOADER) {
		ScopedLocalRef<jstring> jname(env, env->NewStringUTF(name));
		cls = static_cast<jclass>(env->CallObjectMethod(gClassLoader, gFindClassMethod, jname.get()));
    } else {
    	cls = env->FindClass(name);
    }
    checkException(env);
    
    if ((flags & FIND_CLASS_RETURN_GLOBAL_REF) == FIND_CLASS_RETURN_GLOBAL_REF) {
        jclass localClass = cls;
        cls = (jclass)env->NewGlobalRef(localClass);
        env->DeleteLocalRef(localClass);
    }
    return cls;
}

// Provide the ability for an outside component to specify the JavaVM to use
// If the jvm value is set, the getJavaVM function below will just return.
// In getJNIEnv(), if AttachCurrentThread is called to a VM that is already
// attached, the result is a no-op.
void setJavaVM(JavaVM* javaVM)
{
    jvm = javaVM;
    setupGlobalClassLoader();
}

JavaVM* getJavaVM()
{
    if (jvm)
        return jvm;
    LOGE("JavaVM is null");
    return NULL;
}

static void detachCurrentThread(void *data)
{
    getJavaVM()->DetachCurrentThread();
}

JNIEnv* getJNIEnv()
{
    union {
        JNIEnv* env;
        void* dummy;
    } u;
    jint jniError = 0;

    jniError = getJavaVM()->AttachCurrentThread(&u.env, 0);
    if (jniError == JNI_OK) {
        return u.env;
    }
    LOGE("AttachCurrentThread failed, returned %ld", static_cast<long>(jniError));
    return 0;
}

bool checkException(JNIEnv* env) {
    if (env->ExceptionCheck() != 0) {
        LOGE("*** Uncaught exception returned from Java call!\n");
        env->ExceptionDescribe();
        return true;
    }
    return false;
}

std::string jstringToStdString(JNIEnv* env, jstring jstr) {
    if (!jstr || !env)
        return std::string();

    std::string result;
    ScopedLocalRef<jbyteArray> barr(env,
            (jbyteArray) env->CallObjectMethod(jstr, gGetBytesMid));
    jsize alen = env->GetArrayLength(barr.get());
    jbyte * ba = env->GetByteArrayElements(barr.get(), JNI_FALSE);
    if (alen> 0) {
        char* rtn = (char *) malloc (alen + 1);
        memcpy(rtn, ba, alen);
        rtn [alen] = 0;
        result.assign(rtn);
    }
    env->ReleaseByteArrayElements(barr.get(), ba, 0);
    return result;
}

jstring stdStringToJstring(JNIEnv* env, const std::string& str) {
    //return env->NewStringUTF(str.c_str());
    const char* bytes = str.c_str();
    size_t size = str.size();
    ScopedLocalRef<jbyteArray> result(env, env->NewByteArray(size));
    env->SetByteArrayRegion(result.get(), 0, size, (jbyte*)bytes);
    jstring ret = (jstring) env->NewObject(gStringClass, gNewStringMid, result.get());
    return ret;
}

jobjectArray createStringArray(JNIEnv *env, int size) {
    return env->NewObjectArray(size, gStringClass, NULL);
}

void callJNIVoidMethod(jobject obj, const char* methodName, const char* methodSignature, ...)
{
    va_list args;
    va_start(args, methodSignature);

    callJNIMethodV<void>(obj, methodName, methodSignature, args);

    va_end(args);
}

void callJNIVoidMethodID(jobject obj, jmethodID mid, ...)
{
    va_list args;
    va_start(args, mid);

    callJNIMethodIDV<void>(obj, mid, args);

    va_end(args);
}

void callJNIStaticVoidMethod(const char* className, const char* methodName, const char* methodSignature, ...)
{
    va_list args;
    va_start(args, methodSignature);

    JNIEnv *env = getJNIEnv();
    ScopedLocalRef<jclass> cls(env, findClass(className));
    if (checkException(env)) {
        return;
    }
    if (cls.get() && env) {
        jmethodID mid = env->GetStaticMethodID(cls.get(), methodName, methodSignature);
        if (mid)
            JNICaller<void>::callStaticV(cls.get(), mid, args);
        else {
            env->ExceptionDescribe();
            LOGE("Could not find method: %s for %p", methodName, cls.get());
        }
    }
    va_end(args);
}

jobject toJavaBoolean(bool value)
{
    if (value) {
        return gTrue;
    } else {
        return gFalse;
    }
}

jobject toJavaLong(jlong value)
{
    return callJNIStaticMethod<jobject>(gLongClass, gLongValueOf, value);
}

jobject toJavaInt(jint value)
{
    return callJNIStaticMethod<jobject>(gIntClass, gIntValueOf, value);
}

jobject newJavaObject(const char* className)
{
    JNIEnv *env = getJNIEnv();
    ScopedLocalRef<jclass> cls(env, findClass(className));
    if (cls.get()) {
        jmethodID mid = env->GetMethodID(cls.get(), "<init>", "()V");
        if (mid) {
            return env->NewObject(cls.get(), mid);
        } else {
            LOGE("Could not find method %s", "<init>()V");
        }
        env->ExceptionDescribe();
    } else {
        env->ExceptionDescribe();
        LOGE("Could not find class %s", className);
    }
    return 0;
}

std::list<std::string> jstringArrayToStdStringList(jobjectArray jstringArray)
{
    std::list<std::string> list;
    JNIEnv *env = getJNIEnv();
    jsize size = env->GetArrayLength(jstringArray);
    for (int i = 0; i < size; i++) {
        ScopedLocalRef<jobject> item(env, env->GetObjectArrayElement(jstringArray, i));
        if (item.get()) {
            list.push_back(jstringToStdString((jstring)item.get()));
        }
    }
    return list;
}
