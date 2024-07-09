#include "adevs_jni.h"
using namespace adevs;
using namespace std;

JavaNetwork::JavaNetwork(jobject jmodel_loc_ref, JavaSimulator &owner)
    : Network<java_io>(), JavaDevs(jmodel_loc_ref, owner) {
    set_peer(this);
    // Add us to the membership list
    owner.models.insert(this);
}

void JavaNetwork::getComponents(Set<Devs<java_io>*> &c) {
    // Call the getComponents function using the shared collection
    CALL_AND_THROW(owner.jenv->CallVoidMethod(
                       getJavaObjRef(), owner.jnetwork_get_components,
                       owner.jsimulator_shared_collection),
                   owner.jenv)
    // Copy the components into c
    owner.jenv->PushLocalFrame(16);
    jobject iter = owner.jenv->NewGlobalRef(owner.jenv->CallObjectMethod(
        owner.jsimulator_shared_collection, owner.jcollection_iterator));
    assert(iter != NULL);
    while (owner.jenv->CallBooleanMethod(iter, owner.jiterator_hasNext) ==
           JNI_TRUE) {
        owner.jenv->PushLocalFrame(16);
        // Create a global reference to the output value and put it into the output bag.
        // The global references are deleted in the gc_output method
        jobject java_model =
            owner.jenv->CallObjectMethod(iter, owner.jiterator_next);
        // Make sure this is a legitimate object
        if (java_model == NULL) {
            return;
        }
        // If it is, put it into the set of components
        Devs<java_io>* cpp_peer = (Devs<java_io>*)(owner.jenv->GetLongField(
            java_model, owner.jdevs_cpp_peer));
        if (cpp_peer == NULL) {
            if (owner.jenv->GetBooleanField(
                    java_model, owner.jdevs_is_atomic) == JNI_TRUE) {
                cpp_peer = new JavaAtomic(java_model, owner);
            } else {
                cpp_peer = new JavaNetwork(java_model, owner);
            }
        }
        assert(cpp_peer != NULL);
        cpp_peer->setParent(this);
        c.insert(cpp_peer);
        owner.jenv->PopLocalFrame(NULL);
    }
    // Clear the collection
    owner.jenv->CallVoidMethod(owner.jsimulator_shared_collection,
                               owner.jcollection_clear);
    owner.jenv->DeleteGlobalRef(iter);
    owner.jenv->PopLocalFrame(NULL);
}

void JavaNetwork::route(java_io const &x, Devs<java_io>* model,
                        Bag<Event<java_io>> &r) {
    JavaDevs* cpp_peer = dynamic_cast<JavaDevs*>(model);
    // Call the java implementation of the route method
    CALL_AND_THROW(
        owner.jenv->CallVoidMethod(getJavaObjRef(), owner.jnetwork_route, x,
                                   cpp_peer->getJavaObjRef(),
                                   owner.jsimulator_shared_collection),
        owner.jenv)
    // Copy the values into yb
    owner.jenv->PushLocalFrame(16);
    jobject iter = owner.jenv->NewGlobalRef(owner.jenv->CallObjectMethod(
        owner.jsimulator_shared_collection, owner.jcollection_iterator));
    assert(iter != NULL);
    while (owner.jenv->CallBooleanMethod(iter, owner.jiterator_hasNext) ==
           JNI_TRUE) {
        owner.jenv->PushLocalFrame(16);
        jobject the_event =
            owner.jenv->CallObjectMethod(iter, owner.jiterator_next);
        jobject event_value = owner.jenv->NewGlobalRef(
            owner.jenv->GetObjectField(the_event, owner.jevent_value));
        assert(event_value != NULL);
        jobject event_target =
            owner.jenv->GetObjectField(the_event, owner.jevent_model);
        assert(event_target != NULL);
        jlong cpp_peer_id =
            owner.jenv->GetLongField(event_target, owner.jdevs_cpp_peer);
        r.insert(Event<java_io>((Devs<java_io>*)cpp_peer_id, event_value));
        owner.routed_events.push_back(event_value);
        owner.jenv->PopLocalFrame(NULL);
    }
    owner.jenv->CallVoidMethod(owner.jsimulator_shared_collection,
                               owner.jcollection_clear);
    owner.jenv->DeleteGlobalRef(iter);
    owner.jenv->PopLocalFrame(NULL);
}

bool JavaNetwork::model_transition() {CALL_AND_THROW(
    return (owner.jenv -> CallBooleanMethod(getJavaObjRef(),
                                            owner.jdevs_model_transition) ==
            JNI_TRUE),
           owner.jenv)}

JavaNetwork::~JavaNetwork() {
    owner.models.erase(this);
}
