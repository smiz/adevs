#include "adevs_jni.h"
using namespace adevs;
using namespace std;

JavaEventListenerManager::JavaEventListenerManager(JavaSimulator &owner)
    : EventListener<java_io>(), owner(owner) {}

void JavaEventListenerManager::outputEvent(Event<java_io> x, double t) {
    JavaDevs* cpp_model = dynamic_cast<JavaDevs*>(x.model);
    assert(cpp_model != NULL);
    // Make the callback for all registered listeners
    list<jobject>::iterator iter = global_refs.begin();
    for (; iter != global_refs.end(); iter++) {
        assert(owner.simulator_shared_jevent != NULL);
        owner.jenv->SetObjectField(owner.simulator_shared_jevent,
                                   owner.jevent_model,
                                   cpp_model->getJavaObjRef());
        owner.jenv->SetObjectField(owner.simulator_shared_jevent,
                                   owner.jevent_value, x.value);
        CALL_AND_THROW(owner.jenv->CallVoidMethod(
                           *iter, owner.jevent_listener_out,
                           owner.simulator_shared_jevent, (jdouble)t),
                       owner.jenv)
    }
}

void JavaEventListenerManager::stateChange(Atomic<java_io>* model, double t) {
    JavaDevs* cpp_model = dynamic_cast<JavaDevs*>(model);
    assert(cpp_model != NULL);
    // Make the callback for all registered listeners
    list<jobject>::iterator iter = global_refs.begin();
    for (; iter != global_refs.end(); iter++) {
        assert(owner.simulator_shared_jevent != NULL);
        CALL_AND_THROW(
            owner.jenv->CallVoidMethod(*iter, owner.jevent_listener_delta,
                                       cpp_model->getJavaObjRef(), (jdouble)t),
            owner.jenv)
    }
}

void JavaEventListenerManager::addJavaListener(jobject local_obj_ref) {
    global_refs.push_back(owner.jenv->NewGlobalRef(local_obj_ref));
}

void JavaEventListenerManager::removeJavaListener(jobject local_obj_ref) {
    // Look for it in the list
    list<jobject>::iterator iter = global_refs.begin();
    for (; iter != global_refs.end(); iter++) {
        if (owner.jenv->IsSameObject(*iter, local_obj_ref)) {
            owner.jenv->DeleteGlobalRef(*iter);
            global_refs.erase(iter);
            break;
        }
    }
}

JavaEventListenerManager::~JavaEventListenerManager() {
    while (!global_refs.empty()) {
        owner.jenv->DeleteGlobalRef(global_refs.front());
        global_refs.pop_front();
    }
}
