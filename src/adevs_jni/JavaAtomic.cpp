#include "adevs/jni.h"
using namespace adevs;
using namespace std;

JavaAtomic::JavaAtomic(jobject jmodel_loc_ref, JavaSimulator &owner)
    : Atomic<java_io>(), JavaDevs(jmodel_loc_ref, owner) {
    set_peer(this);
    // Add us to the membership list
    owner.models.insert(this);
}

void JavaAtomic::delta_int() {
    CALL_AND_THROW(
        owner.jenv->CallVoidMethod(getJavaObjRef(), owner.jatomic_dint),
        owner.jenv)
}

void JavaAtomic::delta_ext(double e, Bag<java_io> const &xb) {
    // Get the shared collection
    Bag<java_io>::const_iterator iter = xb.begin();
    // Put the events into the shared collection.
    for (; iter != xb.end(); iter++) {
        owner.jenv->CallVoidMethod(owner.jsimulator_shared_collection,
                                   owner.jcollection_add, *iter);
    }
    // Execute the events
    CALL_AND_THROW(owner.jenv->CallVoidMethod(
                       getJavaObjRef(), owner.jatomic_dext, (jdouble)e,
                       owner.jsimulator_shared_collection),
                   owner.jenv)
    // Clear the collection
    owner.jenv->CallVoidMethod(owner.jsimulator_shared_collection,
                               owner.jcollection_clear);
}

void JavaAtomic::delta_conf(Bag<java_io> const &xb) {
    // Get the shared collection
    Bag<java_io>::const_iterator iter = xb.begin();
    // Put the events into the shared collection
    for (; iter != xb.end(); iter++) {
        owner.jenv->CallVoidMethod(owner.jsimulator_shared_collection,
                                   owner.jcollection_add, *iter);
    }
    // Execute the events
    CALL_AND_THROW(
        owner.jenv->CallVoidMethod(getJavaObjRef(), owner.jatomic_dcon,
                                   owner.jsimulator_shared_collection),
        owner.jenv)
    // Clear the collection
    owner.jenv->CallVoidMethod(owner.jsimulator_shared_collection,
                               owner.jcollection_clear);
}

void JavaAtomic::output_func(Bag<java_io> &yb) {
    // Call the output function using the shared collection
    CALL_AND_THROW(
        owner.jenv->CallVoidMethod(getJavaObjRef(), owner.jatomic_out,
                                   owner.jsimulator_shared_collection),
        owner.jenv)
    // Copy the values into yb
    owner.jenv->PushLocalFrame(16);
    jobject iter = owner.jenv->NewGlobalRef(owner.jenv->CallObjectMethod(
        owner.jsimulator_shared_collection, owner.jcollection_iterator));
    while (owner.jenv->CallBooleanMethod(iter, owner.jiterator_hasNext) ==
           JNI_TRUE) {
        // Create a global reference to the output value and put it into the output bag.
        // The global references are deleted in the gc_output method
        owner.jenv->PushLocalFrame(16);
        yb.insert(owner.jenv->NewGlobalRef(
            owner.jenv->CallObjectMethod(iter, owner.jiterator_next)));
        owner.jenv->PopLocalFrame(NULL);
    }
    owner.jenv->DeleteGlobalRef(iter);
    // Clear the collection
    owner.jenv->CallVoidMethod(owner.jsimulator_shared_collection,
                               owner.jcollection_clear);
    owner.jenv->PopLocalFrame(NULL);
}

double JavaAtomic::ta() {
    CALL_AND_THROW(return (double)(owner.jenv->CallDoubleMethod(
                              getJavaObjRef(), owner.jatomic_ta)),
                          owner.jenv)
}

void JavaAtomic::gc_output(Bag<java_io> &yb) {
    // Delete the global references to our output
    Bag<java_io>::const_iterator iter = yb.begin();
    for (; iter != yb.end(); iter++) {
        owner.jenv->DeleteGlobalRef(*iter);
    }
}

bool JavaAtomic::model_transition() {
    // Atomic models initiate all structure changes, and so it is sufficient
    // to set the structure change flag here
    bool result = false;
    CALL_AND_THROW(result = (owner.jenv->CallBooleanMethod(
                                 getJavaObjRef(),
                                 owner.jdevs_model_transition) == JNI_TRUE),
                   owner.jenv);
    if (!owner.structure_change_occurred) {
        owner.structure_change_occurred = result;
    }
    return result;
}

JavaAtomic::~JavaAtomic() {
    owner.models.erase(this);
}
