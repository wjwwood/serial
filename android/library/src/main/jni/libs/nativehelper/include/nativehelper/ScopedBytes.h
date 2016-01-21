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

#ifndef SCOPED_BYTES_H_included
#define SCOPED_BYTES_H_included

#include "JNIHelp.h"

/**
 * ScopedBytesRO and ScopedBytesRW attempt to paper over the differences between byte[]s and
 * ByteBuffers. This in turn helps paper over the differences between non-direct ByteBuffers backed
 * by byte[]s, direct ByteBuffers backed by bytes[]s, and direct ByteBuffers not backed by byte[]s.
 * (On Android, this last group only contains MappedByteBuffers.)
 */
template<bool readOnly>
class ScopedBytes {
public:
    ScopedBytes(JNIEnv* env, jbyteArray object)
    : mEnv(env), mByteArray(object), mPtr(NULL)
    {
        if (mByteArray == NULL) {
            jniThrowNullPointerException(mEnv, NULL);
        } else {
            mPtr = mEnv->GetByteArrayElements(mByteArray, NULL);
            mLength = mEnv->GetArrayLength(mByteArray);
        }
    }

    ~ScopedBytes() {
        if (mByteArray != NULL) {
            mEnv->ReleaseByteArrayElements(mByteArray, mPtr, readOnly ? JNI_ABORT : 0);
        }
    }
    jsize length() {
        return mLength;
    }

private:
    JNIEnv* mEnv;
    jbyteArray mByteArray;

protected:
    jbyte* mPtr;
    jsize mLength;

private:
    // Disallow copy and assignment.
    ScopedBytes(const ScopedBytes&);
    void operator=(const ScopedBytes&);
};

class ScopedBytesRO : public ScopedBytes<true> {
public:
    ScopedBytesRO(JNIEnv* env, jbyteArray object) : ScopedBytes<true>(env, object) {}
    const jbyte* get() const {
        return mPtr;
    }
};

class ScopedBytesRW : public ScopedBytes<false> {
public:
    ScopedBytesRW(JNIEnv* env, jbyteArray object) : ScopedBytes<false>(env, object) {}
    jbyte* get() {
        return mPtr;
    }
};

#endif  // SCOPED_BYTES_H_included
