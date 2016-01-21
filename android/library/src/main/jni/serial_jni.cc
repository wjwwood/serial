#include <nativehelper/JNIHelp.h>
#include "jni_utility.h"
#include <serial/serial.h>

using namespace std;
using namespace serial;

#define MAKE_TIMEOUT(env,in,out) jint* _array = (env)->GetIntArrayElements((in), NULL); \
    if (_array[0] == -1) {\
        _array[0] = Timeout::max();\
    }\
    Timeout out(_array[0], _array[1], _array[2], _array[3], _array[4]);\
    env->ReleaseIntArrayElements((in), _array, JNI_ABORT)


#define _BEGIN_TRY                              try {
#define _CATCH(cpp_ex)                          } catch (cpp_ex& _ex) {
#define _CATCH_AND_THROW(env, cpp_ex, java_ex)  } catch (cpp_ex& _ex) { \
    LOGE("%s", _ex.what());\
    (env)->ThrowNew(java_ex, _ex.what());
#define _END_TRY                                }
    
static jclass gSerialExceptionClass = 0;
static jclass gSerialIOExceptionClass = 0;
static jclass gIllegalArgumentException = 0;

static jobjectArray native_listPorts(JNIEnv *env, jobject)
{
    std::vector<PortInfo> ports = list_ports();
    jobjectArray portDescs = createStringArray(env, ports.size());
    int i = 0;
    for(std::vector<PortInfo>::iterator it = ports.begin(); it < ports.end(); ++it, ++i) {
        std::string desc;
        desc += it->port;
        desc.push_back('\t');
        desc += it->description;
        desc.push_back('\t');
        desc += it->hardware_id;
        env->SetObjectArrayElement(portDescs, i, stdStringToJstring(desc));
    }

    return portDescs;
}

static jlong native_create(JNIEnv *env, jobject, jstring jport, jint baudrate, jintArray jtimeout, jint bytesize, jint parity, jint stopbits, jint flowcontrol) 
{
    std::string port = jstringToStdString(jport);
    MAKE_TIMEOUT(env, jtimeout, timeout);
    
    Serial * com = NULL;
    _BEGIN_TRY
        Serial * com = new Serial(port, (uint32_t)baudrate, timeout, 
            bytesize_t(bytesize), parity_t(parity), stopbits_t(stopbits), flowcontrol_t(flowcontrol));    
        LOGD("Native serial port object %p.", com);
        return (jlong)com;
    _CATCH_AND_THROW(env, invalid_argument, gIllegalArgumentException)
    _CATCH_AND_THROW(env, IOException, gSerialIOExceptionClass)
    _CATCH_AND_THROW(env, SerialException, gSerialExceptionClass)
    _END_TRY
    return (jlong)com;   
}

static void native_destory(JNIEnv *env, jobject, jlong ptr)
{
    Serial * com = (Serial *)ptr;
    if (com)
        delete com;
}

static void native_open(JNIEnv *env, jobject, jlong ptr)
{
    Serial * com = (Serial *)ptr;
    _BEGIN_TRY
        com->open();
    _CATCH_AND_THROW(env, SerialException, gSerialExceptionClass)
    _CATCH_AND_THROW(env, IOException, gSerialIOExceptionClass)
    _CATCH_AND_THROW(env, invalid_argument, gIllegalArgumentException)
    _END_TRY
}

static jboolean native_isOpen(JNIEnv *env, jobject, jlong ptr)
{
    Serial * com = (Serial *)ptr;
    return com->isOpen() ? JNI_TRUE : JNI_FALSE;
}

static void native_close(JNIEnv *env, jobject, jlong ptr)
{
    Serial * com = (Serial *)ptr;
    _BEGIN_TRY
        com->close();
    _CATCH_AND_THROW(env, IOException, gSerialIOExceptionClass)
    _END_TRY
}

static jint native_available(JNIEnv *env, jobject, jlong ptr)
{
    Serial * com = (Serial *)ptr;
    _BEGIN_TRY
        return (jint)com->available();
    _CATCH_AND_THROW(env, IOException, gSerialIOExceptionClass)
    _END_TRY
    return -1;
}

static jboolean native_waitReadable(JNIEnv *env, jobject, jlong ptr)
{
    Serial * com = (Serial *)ptr;
    _BEGIN_TRY
        return com->waitReadable() ? JNI_TRUE : JNI_FALSE;
    _CATCH_AND_THROW(env, IOException, gSerialIOExceptionClass)
    _END_TRY
    return JNI_FALSE;
}

static void native_waitByteTimes(JNIEnv *env, jobject, jlong ptr, jint count)
{
    Serial * com = (Serial *)ptr;
    com->waitByteTimes(count);    
}

static jint native_read(JNIEnv *env, jobject, jlong ptr, jbyteArray jbuffer, jint offset, jint size)
{
    LOGI("native_read(0x%08llx,%p,%d,%d)", ptr, jbuffer, offset, size);
    Serial * com = (Serial *)ptr;
    jbyte* jarray = env->GetByteArrayElements(jbuffer, NULL);
    if (jarray) {
        uint8_t * buffer = (uint8_t *)(jarray + offset);
        LOGI("buffer = %p, size=%d", buffer, size);
        _BEGIN_TRY
            int bytesRead = com->read(buffer, (size_t)size);
            LOGI("bytes read = %d", bytesRead);
            env->ReleaseByteArrayElements(jbuffer, jarray, 0);
            return (jint)bytesRead;
        _CATCH_AND_THROW(env, invalid_argument, gIllegalArgumentException)
            env->ReleaseByteArrayElements(jbuffer, jarray, JNI_ABORT);
        _CATCH_AND_THROW(env, IOException, gSerialIOExceptionClass)
            env->ReleaseByteArrayElements(jbuffer, jarray, JNI_ABORT);
        _CATCH_AND_THROW(env, SerialException, gSerialExceptionClass)
            env->ReleaseByteArrayElements(jbuffer, jarray, JNI_ABORT);
        _END_TRY
        return -1;
    }
    return -1; // Failed
}

static jstring native_readline(JNIEnv *env, jobject, jlong ptr, jint size, jstring jeol)
{
    Serial * com = (Serial *)ptr;
    std::string eol = jstringToStdString(jeol);
    _BEGIN_TRY
        std::string line = com->readline(size, eol);
        return stdStringToJstring(line);
    _CATCH_AND_THROW(env, SerialException, gSerialExceptionClass)
    _CATCH_AND_THROW(env, IOException, gSerialIOExceptionClass)
    _END_TRY
    return NULL;
}

static jobjectArray native_readlines(JNIEnv *env, jobject, jlong ptr, jint size, jstring jeol)
{
    Serial * com = (Serial *)ptr;
    std::string eol = jstringToStdString(jeol);
    jobjectArray jlines = NULL;
    _BEGIN_TRY
        vector<string> lines = com->readlines(size, eol);
        jlines = createStringArray(env, lines.size());
        int i = 0;
        for(vector<string>::iterator it = lines.begin(); it < lines.end(); ++it, ++i) {
            env->SetObjectArrayElement(jlines, i, stdStringToJstring(*it));
        }
    _CATCH_AND_THROW(env, SerialException, gSerialExceptionClass)
    _CATCH_AND_THROW(env, IOException, gSerialIOExceptionClass)
    _END_TRY
    return jlines;
}

static jint native_write(JNIEnv *env, jobject, jlong ptr, jbyteArray jdata, jint size)
{
    LOGI("native_write(0x%08llx,%p,%d)", ptr, jdata, size);
    Serial * com = (Serial *)ptr;
    jbyte* jarray = env->GetByteArrayElements(jdata, NULL);
    if (jarray) {
        uint8_t * data = (uint8_t *)jarray;
        LOGI("data = %p, size=%d", data, size);
        _BEGIN_TRY
            int bytesWritten = com->write(data, (size_t)size);
            LOGI("bytes written = %d", bytesWritten);
            env->ReleaseByteArrayElements(jdata, jarray, JNI_ABORT);
            return (jint)bytesWritten;
        _CATCH_AND_THROW(env, invalid_argument, gIllegalArgumentException)
            env->ReleaseByteArrayElements(jdata, jarray, JNI_ABORT);
        _CATCH_AND_THROW(env, IOException, gSerialIOExceptionClass)
            env->ReleaseByteArrayElements(jdata, jarray, JNI_ABORT);
        _CATCH_AND_THROW(env, SerialException, gSerialExceptionClass)
            env->ReleaseByteArrayElements(jdata, jarray, JNI_ABORT);
        _END_TRY
    }
    return -1;
}

static void native_setPort(JNIEnv *env, jobject, jlong ptr, jstring jport)
{
    Serial * com = (Serial *)ptr;
    std::string port = jstringToStdString(jport);
    com->setPort(port);
}

static jstring native_getPort(JNIEnv *env, jobject, jlong ptr)
{
    Serial * com = (Serial *)ptr;
    std::string port = com->getPort();
    return stdStringToJstring(port);
}

static void native_setBaudrate(JNIEnv *env, jobject, jlong ptr, jint baudrate)
{
    Serial * com = (Serial *)ptr;
    _BEGIN_TRY
        com->setBaudrate((uint32_t)baudrate);
    _CATCH_AND_THROW(env, invalid_argument, gIllegalArgumentException)
    _CATCH_AND_THROW(env, IOException, gSerialIOExceptionClass)
    _CATCH_AND_THROW(env, SerialException, gSerialExceptionClass)
    _END_TRY
}

static jint native_getBaudrate(JNIEnv *env, jobject, jlong ptr)
{
    Serial * com = (Serial *)ptr;
    return (jint)com->getBaudrate();
}

static void native_setTimeout(JNIEnv *env, jobject, jlong ptr, jintArray jtimeout)
{
    Serial * com = (Serial *)ptr;
    MAKE_TIMEOUT(env, jtimeout, timeout);
    com->setTimeout(timeout);
}

static void native_setBytesize(JNIEnv *env, jobject, jlong ptr, jint bytesize)
{
    Serial * com = (Serial *)ptr;
    _BEGIN_TRY
        com->setBytesize(bytesize_t(bytesize));
    _CATCH_AND_THROW(env, invalid_argument, gIllegalArgumentException)
    _CATCH_AND_THROW(env, IOException, gSerialIOExceptionClass)
    _CATCH_AND_THROW(env, SerialException, gSerialExceptionClass)
    _END_TRY
}

static jint native_getBytesize(JNIEnv *env, jobject, jlong ptr)
{
    Serial * com = (Serial *)ptr;
    return (jint)com->getBytesize();
}

static void native_setParity(JNIEnv *env, jobject, jlong ptr, jint parity)
{
    Serial * com = (Serial *)ptr;
    _BEGIN_TRY
        com->setParity(parity_t(parity));
    _CATCH_AND_THROW(env, invalid_argument, gIllegalArgumentException)
    _CATCH_AND_THROW(env, IOException, gSerialIOExceptionClass)
    _CATCH_AND_THROW(env, SerialException, gSerialExceptionClass)
    _END_TRY
}

static jint native_getParity(JNIEnv *env, jobject, jlong ptr)
{
    Serial * com = (Serial *)ptr;
    return (jint)com->getParity();
}

static void native_setStopbits(JNIEnv *env, jobject, jlong ptr, jint stopbits)
{
    Serial * com = (Serial *)ptr;
    _BEGIN_TRY
        com->setStopbits(stopbits_t(stopbits));
    _CATCH_AND_THROW(env, SerialException, gSerialExceptionClass)
    _CATCH_AND_THROW(env, IOException, gSerialIOExceptionClass)
    _END_TRY
}

static jint native_getStopbits(JNIEnv *env, jobject, jlong ptr)
{
    Serial * com = (Serial *)ptr;
    return (jint)com->getStopbits();
}

static void native_setFlowcontrol(JNIEnv *env, jobject, jlong ptr, jint flowcontrol)
{
    Serial * com = (Serial *)ptr;
    _BEGIN_TRY
        com->setFlowcontrol(flowcontrol_t(flowcontrol));
    _CATCH_AND_THROW(env, invalid_argument, gIllegalArgumentException)
    _CATCH_AND_THROW(env, IOException, gSerialIOExceptionClass)
    _CATCH_AND_THROW(env, SerialException, gSerialExceptionClass)
    _END_TRY
}

static jint native_getFlowcontrol(JNIEnv *env, jobject, jlong ptr)
{
    Serial * com = (Serial *)ptr;
    return (jint)com->getFlowcontrol();
}

static void native_flush(JNIEnv *env, jobject, jlong ptr)
{
    Serial * com = (Serial *)ptr;
    com->flush();
}

static void native_flushInput(JNIEnv *env, jobject, jlong ptr)
{
    Serial * com = (Serial *)ptr;
    com->flushInput();
}

static void native_flushOutput(JNIEnv *env, jobject, jlong ptr)
{
    Serial * com = (Serial *)ptr;
    com->flushOutput();
}

static void native_sendBreak(JNIEnv *env, jobject, jlong ptr, jint duration)
{
    Serial * com = (Serial *)ptr;
    com->sendBreak((int)duration);
}

static void native_setBreak(JNIEnv *env, jobject, jlong ptr, jboolean level)
{
    Serial * com = (Serial *)ptr;
    _BEGIN_TRY
        com->setBreak((bool)level);
    _CATCH_AND_THROW(env, IOException, gSerialIOExceptionClass)
    _CATCH_AND_THROW(env, SerialException, gSerialExceptionClass)
    _END_TRY
}

static void native_setRTS(JNIEnv *env, jobject, jlong ptr, jboolean level)
{
    Serial * com = (Serial *)ptr;
    _BEGIN_TRY
        com->setRTS((bool)level);
    _CATCH_AND_THROW(env, IOException, gSerialIOExceptionClass)
    _CATCH_AND_THROW(env, SerialException, gSerialExceptionClass)
    _END_TRY
}

static void native_setDTR(JNIEnv *env, jobject, jlong ptr, jboolean level)
{
    Serial * com = (Serial *)ptr;
    _BEGIN_TRY
        com->setDTR((bool)level);
    _CATCH_AND_THROW(env, IOException, gSerialIOExceptionClass)
    _CATCH_AND_THROW(env, SerialException, gSerialExceptionClass)
    _END_TRY
}

static jboolean native_waitForChange(JNIEnv *env, jobject, jlong ptr)
{
    Serial * com = (Serial *)ptr;
    _BEGIN_TRY
        return com->waitForChange() ? JNI_TRUE : JNI_FALSE;
    _CATCH_AND_THROW(env, SerialException, gSerialExceptionClass)
    _END_TRY
    return JNI_FALSE;
}

static jboolean native_getCTS(JNIEnv *env, jobject, jlong ptr)
{
    Serial * com = (Serial *)ptr;
    _BEGIN_TRY
        return com->getCTS() ? JNI_TRUE : JNI_FALSE;
    _CATCH_AND_THROW(env, SerialException, gSerialExceptionClass)
    _END_TRY
    return JNI_FALSE;
}

static jboolean native_getDSR(JNIEnv *env, jobject, jlong ptr)
{
    Serial * com = (Serial *)ptr;
    _BEGIN_TRY
        return com->getDSR() ? JNI_TRUE : JNI_FALSE;
    _CATCH_AND_THROW(env, SerialException, gSerialExceptionClass)
    _END_TRY
    return JNI_FALSE;
}

static jboolean native_getRI(JNIEnv *env, jobject, jlong ptr)
{
    Serial * com = (Serial *)ptr;
    _BEGIN_TRY
        return com->getRI() ? JNI_TRUE : JNI_FALSE;
    _CATCH_AND_THROW(env, SerialException, gSerialExceptionClass)
    _END_TRY
    return JNI_FALSE;
}

static jboolean native_getCD(JNIEnv *env, jobject, jlong ptr)
{
    Serial * com = (Serial *)ptr;
    _BEGIN_TRY
        return com->getCD() ? JNI_TRUE : JNI_FALSE;
    _CATCH_AND_THROW(env, SerialException, gSerialExceptionClass)
    _END_TRY
    return JNI_FALSE;
}


#ifdef __cplusplus
extern "C" {
#endif

static JNINativeMethod gSerialMethods[] = {
    { "native_listPorts", "()[Ljava/lang/String;", (void*) native_listPorts },
    { "native_create", "(Ljava/lang/String;I[IIIII)J", (void*) native_create },
    { "native_destory", "(J)V", (void*) native_destory },
    { "native_open", "(J)V", (void*) native_open },
    { "native_isOpen", "(J)Z", (void*) native_isOpen },
    { "native_close", "(J)V", (void*) native_close },
    { "native_available", "(J)I", (void*) native_available },
    { "native_waitReadable", "(J)Z", (void*) native_waitReadable },
    { "native_waitByteTimes", "(JI)V", (void*) native_waitByteTimes },
    { "native_read", "(J[BII)I", (void*) native_read },
    { "native_readline", "(JILjava/lang/String;)Ljava/lang/String;", (void*) native_readline },
    { "native_readlines", "(JILjava/lang/String;)[Ljava/lang/String;", (void*) native_readlines },
    { "native_write", "(J[BI)I", (void*) native_write },
    { "native_setPort", "(JLjava/lang/String;)V", (void*) native_setPort },
    { "native_getPort", "(J)Ljava/lang/String;", (void*) native_getPort },
    { "native_setBaudrate", "(JI)V", (void*) native_setBaudrate },
    { "native_getBaudrate", "(J)I", (void*) native_getBaudrate },
    { "native_setTimeout", "(J[I)V", (void*) native_setTimeout },
    { "native_setBytesize", "(JI)V", (void*) native_setBytesize },
    { "native_getBytesize", "(J)I", (void*) native_getBytesize },
    { "native_setParity", "(JI)V", (void*) native_setParity },
    { "native_getParity", "(J)I", (void*) native_getParity },
    { "native_setStopbits", "(JI)V", (void*) native_setStopbits },
    { "native_getStopbits", "(J)I", (void*) native_getStopbits },
    { "native_setFlowcontrol", "(JI)V", (void*) native_setFlowcontrol },
    { "native_getFlowcontrol", "(J)I", (void*) native_getFlowcontrol },
    { "native_flush", "(J)V", (void*) native_flush },
    { "native_flushInput", "(J)V", (void*) native_flushInput },
    { "native_flushOutput", "(J)V", (void*) native_flushOutput },
    { "native_sendBreak", "(JI)V", (void*) native_sendBreak },
    { "native_setBreak", "(JZ)V", (void*) native_setBreak },
    { "native_setRTS", "(JZ)V", (void*) native_setRTS },
    { "native_setDTR", "(JZ)V", (void*) native_setDTR },
    { "native_waitForChange", "(J)Z", (void*) native_waitForChange },
    { "native_getCTS", "(J)Z", (void*) native_getCTS },
    { "native_getDSR", "(J)Z", (void*) native_getDSR },
    { "native_getRI", "(J)Z", (void*) native_getRI },
    { "native_getCD", "(J)Z", (void*) native_getCD },
};

int registerSerial(JNIEnv* env)
{
    gSerialExceptionClass = findClass("serial/SerialException", FIND_CLASS_RETURN_GLOBAL_REF);
    gSerialIOExceptionClass = findClass("serial/SerialIOException", FIND_CLASS_RETURN_GLOBAL_REF);
    gIllegalArgumentException = findClass("java/lang/IllegalArgumentException", FIND_CLASS_RETURN_GLOBAL_REF);
    return jniRegisterNativeMethods(env, "serial/Serial", gSerialMethods, NELEM(gSerialMethods));
}
#ifdef __cplusplus
}
#endif
