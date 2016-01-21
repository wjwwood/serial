/*
 * Copyright (C) 2010 The Android Open Source Project
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

#define LOG_TAG "JniConstants"

#include "ALog-priv.h"
#include "JniConstants.h"
#include "ScopedLocalRef.h"

#include <stdlib.h>

jclass JniConstants::bidiRunClass;
jclass JniConstants::bigDecimalClass;
jclass JniConstants::booleanClass;
jclass JniConstants::byteArrayClass;
jclass JniConstants::byteClass;
jclass JniConstants::calendarClass;
jclass JniConstants::characterClass;
jclass JniConstants::charsetICUClass;
jclass JniConstants::constructorClass;
jclass JniConstants::deflaterClass;
jclass JniConstants::doubleClass;
jclass JniConstants::errnoExceptionClass;
jclass JniConstants::fieldClass;
jclass JniConstants::fieldPositionIteratorClass;
jclass JniConstants::fileDescriptorClass;
jclass JniConstants::floatClass;
jclass JniConstants::gaiExceptionClass;
jclass JniConstants::inet6AddressClass;
jclass JniConstants::inetAddressClass;
jclass JniConstants::inetSocketAddressClass;
jclass JniConstants::inetUnixAddressClass;
jclass JniConstants::inflaterClass;
jclass JniConstants::inputStreamClass;
jclass JniConstants::integerClass;
jclass JniConstants::localeDataClass;
jclass JniConstants::longClass;
jclass JniConstants::methodClass;
jclass JniConstants::mutableIntClass;
jclass JniConstants::mutableLongClass;
jclass JniConstants::objectClass;
jclass JniConstants::objectArrayClass;
jclass JniConstants::outputStreamClass;
jclass JniConstants::parsePositionClass;
jclass JniConstants::patternSyntaxExceptionClass;
jclass JniConstants::realToStringClass;
jclass JniConstants::referenceClass;
jclass JniConstants::shortClass;
jclass JniConstants::socketClass;
jclass JniConstants::socketImplClass;
jclass JniConstants::stringClass;
jclass JniConstants::structAddrinfoClass;
jclass JniConstants::structFlockClass;
jclass JniConstants::structGroupReqClass;
jclass JniConstants::structGroupSourceReqClass;
jclass JniConstants::structLingerClass;
jclass JniConstants::structPasswdClass;
jclass JniConstants::structPollfdClass;
jclass JniConstants::structStatClass;
jclass JniConstants::structStatVfsClass;
jclass JniConstants::structTimevalClass;
jclass JniConstants::structUcredClass;
jclass JniConstants::structUtsnameClass;

static jclass findClass(JNIEnv* env, const char* name) {
    ScopedLocalRef<jclass> localClass(env, env->FindClass(name));
    jclass result = reinterpret_cast<jclass>(env->NewGlobalRef(localClass.get()));
    if (result == NULL) {
        ALOGE("failed to find class '%s'", name);
        abort();
    }
    return result;
}

void JniConstants::init(JNIEnv* env) {
    bidiRunClass = findClass(env, "java/text/Bidi$Run");
    bigDecimalClass = findClass(env, "java/math/BigDecimal");
    booleanClass = findClass(env, "java/lang/Boolean");
    byteClass = findClass(env, "java/lang/Byte");
    byteArrayClass = findClass(env, "[B");
    calendarClass = findClass(env, "java/util/Calendar");
    characterClass = findClass(env, "java/lang/Character");
    charsetICUClass = findClass(env, "java/nio/charset/CharsetICU");
    constructorClass = findClass(env, "java/lang/reflect/Constructor");
    floatClass = findClass(env, "java/lang/Float");
    deflaterClass = findClass(env, "java/util/zip/Deflater");
    doubleClass = findClass(env, "java/lang/Double");
    errnoExceptionClass = findClass(env, "android/system/ErrnoException");
    fieldClass = findClass(env, "java/lang/reflect/Field");
    fieldPositionIteratorClass = findClass(env, "libcore/icu/NativeDecimalFormat$FieldPositionIterator");
    fileDescriptorClass = findClass(env, "java/io/FileDescriptor");
    gaiExceptionClass = findClass(env, "android/system/GaiException");
    inet6AddressClass = findClass(env, "java/net/Inet6Address");
    inetAddressClass = findClass(env, "java/net/InetAddress");
    inetSocketAddressClass = findClass(env, "java/net/InetSocketAddress");
    inetUnixAddressClass = findClass(env, "java/net/InetUnixAddress");
    inflaterClass = findClass(env, "java/util/zip/Inflater");
    inputStreamClass = findClass(env, "java/io/InputStream");
    integerClass = findClass(env, "java/lang/Integer");
    localeDataClass = findClass(env, "libcore/icu/LocaleData");
    longClass = findClass(env, "java/lang/Long");
    methodClass = findClass(env, "java/lang/reflect/Method");
    mutableIntClass = findClass(env, "android/util/MutableInt");
    mutableLongClass = findClass(env, "android/util/MutableLong");
    objectClass = findClass(env, "java/lang/Object");
    objectArrayClass = findClass(env, "[Ljava/lang/Object;");
    outputStreamClass = findClass(env, "java/io/OutputStream");
    parsePositionClass = findClass(env, "java/text/ParsePosition");
    patternSyntaxExceptionClass = findClass(env, "java/util/regex/PatternSyntaxException");
    realToStringClass = findClass(env, "java/lang/RealToString");
    referenceClass = findClass(env, "java/lang/ref/Reference");
    shortClass = findClass(env, "java/lang/Short");
    socketClass = findClass(env, "java/net/Socket");
    socketImplClass = findClass(env, "java/net/SocketImpl");
    stringClass = findClass(env, "java/lang/String");
    structAddrinfoClass = findClass(env, "android/system/StructAddrinfo");
    structFlockClass = findClass(env, "android/system/StructFlock");
    structGroupReqClass = findClass(env, "android/system/StructGroupReq");
    structGroupSourceReqClass = findClass(env, "android/system/StructGroupSourceReq");
    structLingerClass = findClass(env, "android/system/StructLinger");
    structPasswdClass = findClass(env, "android/system/StructPasswd");
    structPollfdClass = findClass(env, "android/system/StructPollfd");
    structStatClass = findClass(env, "android/system/StructStat");
    structStatVfsClass = findClass(env, "android/system/StructStatVfs");
    structTimevalClass = findClass(env, "android/system/StructTimeval");
    structUcredClass = findClass(env, "android/system/StructUcred");
    structUtsnameClass = findClass(env, "android/system/StructUtsname");
}
