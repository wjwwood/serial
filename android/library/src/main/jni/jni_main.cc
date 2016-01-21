#include <jni.h>
#include <stdlib.h>

#include "jni_utility.h"
#include "log.h"

using namespace std;

#ifdef __cplusplus
extern "C" {
#endif

struct RegistrationMethod {
    const char* name;
    int (*func)(JNIEnv*);
};

extern int registerSerial(JNIEnv* env);

static RegistrationMethod gRegMethods[] = {
    { "Serial", registerSerial },
};

JNIEXPORT jint JNI_OnLoad(JavaVM* vm, void* reserved)
{
    LOGV("JNI_OnLoad");
    JNIEnv* env = NULL;
    jint result = -1;

    if (vm->GetEnv((void**) &env, JNI_VERSION_1_4) != JNI_OK) {
        LOGE("GetEnv failed!");
        return result;
    }
    if (!env)
        LOGE("Could not retrieve the env!");

    // Save the JavaVM pointer for use globally.
    setJavaVM(vm);

    const RegistrationMethod* method = gRegMethods;
    const RegistrationMethod* end = method + sizeof(gRegMethods) / sizeof(RegistrationMethod);
    while (method != end) {
        if (method->func(env) < 0) {
            LOGE("%s registration failed!", method->name);
            return result;
        }
        //LOGV("%s registration done.", method->name);
        method++;
    }

    // Initialize rand() function. The rand() function is used in
    // FileSystemAndroid to create a random temporary filename.
    srand(time(NULL));
    LOGV("JNI_OnLoad done.");
    return JNI_VERSION_1_4;
}

#ifdef __cplusplus
}
#endif
