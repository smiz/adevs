#include "adevs_jni/adevs_jni.h"
#include "adevs_jni/adevs_Simulator.h"
#include <list>
using namespace adevs;
using namespace std;

/**
 * Throw a Java exception in response to an adevs exception.
 */
static void throwit(adevs::exception exp, JNIEnv* jenv)
{
	jclass newExcCls = jenv->FindClass("adevs/SimulationException");
	if (newExcCls == NULL)
	{
		/* Unable to find the exception class, give up. */
		return;
	}
	jenv->ThrowNew(newExcCls,exp.what());
}
	
/**
 * Free references left dangling when the networks route their events.
 * The routed_events list is filled with objects in the JavaNetwork's
 * route method.
 */
static void clean_up_dangling_io_references(JavaSimulator* sim)
{
	for (vector<java_io>::iterator iter = sim->routed_events.begin();
			iter != sim->routed_events.end(); iter++)
		sim->jenv->DeleteGlobalRef(*iter);
	sim->routed_events.clear();
}

/**
 * Find and remove model peers that were orphaned by a structure
 * change.
 */
static void clean_up_orphaned_models(JavaSimulator* sim)
{
	if (!sim->structure_change_occurred) return;
	list<Devs<java_io>*> orphans;
	set<Devs<java_io>*>::iterator iter = sim->models.begin();
	// Make a list of the orphans
	for (; iter != sim->models.end(); iter++)
	{
		Devs<java_io>* parent = (*iter)->getParent();
		// Is not the root and has no parent
		if (parent != NULL && sim->models.find(parent) == sim->models.end())
			orphans.push_back(*iter);
	}
	// Delete the orphans
	while (!orphans.empty())
	{
		delete orphans.front();
		orphans.pop_front();
	}
	sim->structure_change_occurred = false;
}

/*
 * Class:     adevs_Simulator
 * Method:    createCppSimulator
 * Signature: (Ladevs/Devs;)J
 */
JNIEXPORT jlong JNICALL Java_adevs_Simulator_createCppSimulator
  (JNIEnv *env, jobject caller, jobject model)
{
	jclass clazz;
	JavaSimulator* sim = new JavaSimulator;
	sim->structure_change_occurred = false;
	// Save the reference to the environment
	sim->jenv = env;
	// Setup all of the method, class, and field IDs that we will need
	env->PushLocalFrame(16);
	// adevs.Devs
	clazz = env->FindClass("adevs/Devs");
	sim->jdevs_class = env->NewGlobalRef(clazz);
	sim->jdevs_model_transition = env->GetMethodID(clazz,"model_transition","()Z");
	sim->jdevs_cpp_peer = env->GetFieldID(clazz,"cpp_peer","J");
	sim->jdevs_is_atomic = env->GetFieldID(clazz,"is_atomic","Z");
	env->DeleteLocalRef(clazz);
	// adevs.Atomic 
	clazz = env->FindClass("adevs/Atomic");
	sim->jatomic_class = env->NewGlobalRef(clazz);
	sim->jatomic_dint = env->GetMethodID(clazz,"delta_int","()V");
	sim->jatomic_dext = env->GetMethodID(clazz,"delta_ext","(DLjava/util/Collection;)V");
	sim->jatomic_dcon = env->GetMethodID(clazz,"delta_conf","(Ljava/util/Collection;)V");
	sim->jatomic_ta = env->GetMethodID(clazz,"ta","()D");
	sim->jatomic_out = env->GetMethodID(clazz,"output_func","(Ljava/util/Collection;)V");
	env->DeleteLocalRef(clazz);
	// java.util.Collection
	clazz = env->FindClass("java/util/Collection");
	sim->jcollection_class = env->NewGlobalRef(clazz);
	sim->jcollection_add = env->GetMethodID(clazz,"add","(Ljava/lang/Object;)Z");
	sim->jcollection_clear = env->GetMethodID(clazz,"clear","()V");
	sim->jcollection_iterator = env->GetMethodID(clazz,"iterator","()Ljava/util/Iterator;");
	env->DeleteLocalRef(clazz);
	// java.util.Iterator
	clazz = env->FindClass("java/util/Iterator");
	sim->jiterator_class = env->NewGlobalRef(clazz);
	sim->jiterator_next = env->GetMethodID(clazz,"next","()Ljava/lang/Object;");
	sim->jiterator_hasNext = env->GetMethodID(clazz,"hasNext","()Z");
	env->DeleteLocalRef(clazz);
	// adevs.Simulator
	clazz = env->FindClass("adevs/Simulator");
	sim->jsimulator_class = env->NewGlobalRef(clazz);
	jfieldID shared_jcollection = env->GetFieldID(clazz,"shared_coll","Ljava/util/Collection;");
	jfieldID shared_jevent = env->GetFieldID(clazz,"shared_event","Ladevs/Event;");
	env->DeleteLocalRef(clazz);
	// adevs.Event
	clazz = env->FindClass("adevs/Event");
	sim->jevent_class = env->NewGlobalRef(clazz);
	sim->jevent_model = env->GetFieldID(clazz,"model","Ladevs/Devs;");
	sim->jevent_value = env->GetFieldID(clazz,"value","Ljava/lang/Object;");
	env->DeleteLocalRef(clazz);
	// adevs.EventListener 
	clazz = env->FindClass("adevs/EventListener");
	sim->jevent_listener_class = env->NewGlobalRef(clazz);
	sim->jevent_listener_out = env->GetMethodID(clazz,"outputEvent","(Ladevs/Event;D)V");
	sim->jevent_listener_delta = env->GetMethodID(clazz,"stateChange","(Ladevs/Atomic;D)V");
	env->DeleteLocalRef(clazz);
	// adevs.Network
	clazz = env->FindClass("adevs/Network");
	sim->jnetwork_class = env->NewGlobalRef(clazz);
	sim->jnetwork_route = env->GetMethodID(clazz,"route","(Ljava/lang/Object;Ladevs/Devs;Ljava/util/Collection;)V");
	sim->jnetwork_get_components = env->GetMethodID(clazz,"getComponents","(Ljava/util/Collection;)V");
	env->DeleteLocalRef(clazz);
	// Done!
	env->PopLocalFrame(NULL);
	// No event listeners to start
	sim->event_lst_mgr = NULL;
	// Create a reference to our shared collection
	sim->jsimulator_shared_collection =
		env->NewGlobalRef(env->GetObjectField(caller,shared_jcollection));
	// Create a reference to our shared event
	sim->simulator_shared_jevent =
		env->NewGlobalRef(env->GetObjectField(caller,shared_jevent));
	// Create a simulator for the model
	try 
	{
		if (env->GetBooleanField(model,sim->jdevs_is_atomic) == JNI_TRUE)
			sim->sim = new Simulator<java_io>(new JavaAtomic(model,*sim));
		else
			sim->sim = new Simulator<java_io>(new JavaNetwork(model,*sim));
	}
	catch(jthrowable exp)
	{
		sim->sim = NULL;
		env->ExceptionClear();
		env->Throw(exp);
	}
	catch(adevs::exception& exp)
	{
		throwit(exp,env);
	}
	// Return a reference to it
	return (jlong)sim;
}

/*
 * Class:     adevs_Simulator
 * Method:    nextEventTime
 * Signature: (J)D
 */
JNIEXPORT jdouble JNICALL Java_adevs_Simulator_nextEventTime
  (JNIEnv *env, jobject, jlong peer_id)
{
	if (peer_id == 0) return DBL_MAX;
	JavaSimulator* sim = (JavaSimulator*)peer_id;
	sim->jenv = env;
	return (jdouble)sim->sim->nextEventTime();
}

/*
 * Class:     adevs_Simulator
 * Method:    execNextEvent
 * Signature: (J)V
 */
JNIEXPORT void JNICALL Java_adevs_Simulator_execNextEvent
  (JNIEnv *env, jobject, jlong peer_id)
{
	if (peer_id == 0) return;
	JavaSimulator* sim = (JavaSimulator*)peer_id;
	sim->jenv = env;
	try
	{
		sim->sim->execNextEvent();
	}
	catch(jthrowable exp)
	{
		env->ExceptionClear();
		env->Throw(exp);
	}
	catch(adevs::exception& exp)
	{
		throwit(exp,env);
	}
	clean_up_dangling_io_references(sim);
	clean_up_orphaned_models(sim);
}

/*
 * Class:     adevs_Simulator
 * Method:    execUntil
 * Signature: (DJ)V
 */
JNIEXPORT void JNICALL Java_adevs_Simulator_execUntil
  (JNIEnv *env, jobject, jdouble tend, jlong peer_id)
{
	if (peer_id == 0) return;
	JavaSimulator* sim = (JavaSimulator*)peer_id;
	sim->jenv = env;
	try
	{
		sim->sim->execUntil((double)tend);
	}
	catch(jthrowable exp)
	{
		env->ExceptionClear();
		env->Throw(exp);
	}
	catch(adevs::exception& exp)
	{
		throwit(exp,env);
	}
}

/*
 * Class:     adevs_Simulator
 * Method:    computeNextOutput
 * Signature: (J)V
 */
JNIEXPORT void JNICALL Java_adevs_Simulator_computeNextOutput
  (JNIEnv *env, jobject, jlong peer_id)
{
	if (peer_id == 0) return;
	JavaSimulator* sim = (JavaSimulator*)peer_id;
	sim->jenv = env;
	try 
	{
		sim->sim->computeNextOutput();
	}
	catch(jthrowable exp)
	{
		env->ExceptionClear();
		env->Throw(exp);
	}
	catch(adevs::exception& exp)
	{
		throwit(exp,env);
	}
}

/*
 * Class:     adevs_Simulator
 * Method:    computeNextState
 * Signature: (Ljava/util/Collection;DJ)V
 */
JNIEXPORT void JNICALL Java_adevs_Simulator_computeNextState
  (JNIEnv *env, jobject caller, jobject collection, jdouble t, jlong peer_id)
{
	if (peer_id == 0) return;
	JavaSimulator* sim = (JavaSimulator*)peer_id;
	sim->jenv = env;
	// Iterate through the java Collection and copy the items to the input bag
	jobject iter = sim->jenv->NewGlobalRef(sim->jenv->CallObjectMethod(collection,sim->jcollection_iterator));
	while (sim->jenv->CallBooleanMethod(iter,sim->jiterator_hasNext) == JNI_TRUE)
	{
		sim->jenv->PushLocalFrame(16);
		jobject event = sim->jenv->CallObjectMethod(iter,sim->jiterator_next);
		// Get the cpp peer of the model
		jobject java_model = sim->jenv->GetObjectField(event,sim->jevent_model);
		Devs<java_io>* adevs_model =
			(Devs<java_io>*)(sim->jenv->GetLongField(java_model,sim->jdevs_cpp_peer));
		// Get the cpp value
		jobject java_value = sim->jenv->GetObjectField(event,sim->jevent_value);
		// Create an adevs event and put it into the input bag
		sim->input_bag.insert(Event<java_io>(adevs_model,sim->jenv->NewGlobalRef(java_value)));
		sim->jenv->PopLocalFrame(NULL);
	}
	sim->jenv->DeleteGlobalRef(iter);
	// Apply the input to the simulator
	try
	{
		sim->sim->computeNextState(sim->input_bag,(double)t);
	}
	catch(jthrowable exp)
	{
		env->ExceptionClear();
		env->Throw(exp);
	}
	catch(adevs::exception& exp)
	{
		throwit(exp,env);
	}
	// Clear the bag for the next call
	Bag<Event<java_io> >::iterator giter = sim->input_bag.begin();
	for (; giter != sim->input_bag.end(); giter++)
		sim->jenv->DeleteGlobalRef((*giter).value);
	sim->input_bag.clear();
	clean_up_dangling_io_references(sim);
	clean_up_orphaned_models(sim);
}

/*
 * Class:     adevs_Simulator
 * Method:    addEventListener
 * Signature: (Ladevs/EventListener;J)V
 */
JNIEXPORT void JNICALL Java_adevs_Simulator_addEventListener
  (JNIEnv *env, jobject, jobject event_listener, jlong peer_id)
{
	if (peer_id == 0) return;
	JavaSimulator* sim = (JavaSimulator*)peer_id;
	sim->jenv = env;
	if (sim->event_lst_mgr == NULL)
	{
		sim->event_lst_mgr = new JavaEventListenerManager(*sim);
		sim->sim->addEventListener(sim->event_lst_mgr);
	}
	sim->event_lst_mgr->addJavaListener(event_listener);
}

/*
 * Class:     adevs_Simulator
 * Method:    removeEventListener
 * Signature: (Ladevs/EventListener;J)V
 */
JNIEXPORT void JNICALL Java_adevs_Simulator_removeEventListener
  (JNIEnv *env, jobject, jobject event_listener, jlong peer_id)
{
	if (peer_id == 0) return;
	JavaSimulator* sim = (JavaSimulator*)peer_id;
	sim->jenv = env;
	if (sim->event_lst_mgr != NULL)
		sim->event_lst_mgr->removeJavaListener(event_listener);
}

/*
 * Class:     adevs_Simulator
 * Method:    destroyCppSimulator
 * Signature: (J)V
 */
JNIEXPORT void JNICALL Java_adevs_Simulator_destroyCppSimulator
  (JNIEnv *env, jobject, jlong peer_id)
{
	if (peer_id == 0) return;
	JavaSimulator* sim = (JavaSimulator*)peer_id;
	sim->jenv = env;
	// Delete the simulator
	delete sim->sim;
	// Delete all of its model peers
	while (!sim->models.empty())
	{
		JavaDevs* to_delete = dynamic_cast<JavaDevs*>(*(sim->models.begin()));
		to_delete->clear_peer();
		// Models remove themselves as they are deleted
		delete (*(sim->models.begin()));
	}
	// Delete the listener manager
	if (sim->event_lst_mgr != NULL)
		delete sim->event_lst_mgr;
	// Cleanup any references left behind by a call to computeOutput()
	clean_up_dangling_io_references(sim);
	// Delete our now useless global references
	env->DeleteGlobalRef(sim->jsimulator_shared_collection);
	env->DeleteGlobalRef(sim->jdevs_class);
	env->DeleteGlobalRef(sim->jatomic_class);
	env->DeleteGlobalRef(sim->jcollection_class);
	env->DeleteGlobalRef(sim->jiterator_class);
	env->DeleteGlobalRef(sim->jsimulator_class);
	env->DeleteGlobalRef(sim->jevent_class);
	env->DeleteGlobalRef(sim->jevent_listener_class);
	env->DeleteGlobalRef(sim->jnetwork_class);
	env->DeleteGlobalRef(sim->simulator_shared_jevent); 
	delete sim;
}

