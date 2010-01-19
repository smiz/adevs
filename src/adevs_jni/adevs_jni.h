#ifndef _adevs_jni_h
#define _adevs_jni_h
#include "adevs.h"
#include "jni.h"
#include <set>
#include <list>
#include <vector>

/*
 * This is the Java JNI interface for the adevs simulation engine.
 */

namespace adevs {

typedef jobject java_io;
class JavaEventListenerManager;

/**
 * This Simulator implements the Java adevs.Simulator class. It adds to the Simulator 
 * class a list of Java object peers that are in use by the simulation. This is used
 * for garbage collection when the Simulator is destroyed.
 */
struct JavaSimulator
{
	// The set of models in use by the simulator. Membership in the set is
	// handled by the model's themselves in their constructors and destructors.
	std::set<Devs<java_io>*> models;
	// List of global references created by the JavaNetwork objects while routing events
	std::vector<java_io> routed_events;
	// Flag indicating that a structure change has occurred
	bool structure_change_occurred;
	// The simulator itself
	adevs::Simulator<java_io>* sim;
	// Manager for Java EventListeners
	JavaEventListenerManager* event_lst_mgr;
	// The java runtime environment
	JNIEnv* jenv;
	// A bag of events for handling input to the simulator
	adevs::Bag<Event<java_io> > input_bag;
	// The java.Simulator class
	jobject jsimulator_class;
	// The java.Simulator collection for use by all of the objects in the models set
	jobject jsimulator_shared_collection;
	// Method and class IDs for a java collection
	jobject jcollection_class;
	// These methods must be supported by jcollection_obj
	jmethodID	jcollection_add, // boolean add(Object)
			jcollection_clear, // clear()
			jcollection_iterator; // Iterator iterator 
	// Method, class, and field IDs for the adevs.Devs class
	jobject jdevs_class;
	jmethodID jdevs_model_transition; // the model transition method
	jfieldID jdevs_cpp_peer; // model's cpp peer
	jfieldID jdevs_is_atomic; // flag indicating if the model is atomic
	// Method, class, and field IDs for the adevs.Atomic class
	jmethodID	jatomic_dint, // internal transition function
			jatomic_dext, // external transition function
			jatomic_dcon, // confluent transition function
			jatomic_ta, // time advance
			jatomic_out; // output function
	jobject jatomic_class;
	// Method, class, and field IDs for the Iterator class
	jobject jiterator_class;
	jmethodID jiterator_next, jiterator_hasNext;
	// Method, class, and field IDs for the adevs.Event class
	jobject jevent_class;
	jfieldID jevent_model, jevent_value;
	jobject simulator_shared_jevent; // This is the event to be used in Listener callbacks
	// Method, class and field IDs for the adevs.EventListener class
	jobject jevent_listener_class;
	jmethodID jevent_listener_out, jevent_listener_delta;
	// Method, class, and field IDs for the adevs.Network class
	jobject jnetwork_class;
	jmethodID jnetwork_get_components, jnetwork_route;
};

/**
 * A single listener manages all of the Java EventListener interfaces that are
 * registed with the simulator.
 */
class JavaEventListenerManager:
	public EventListener<java_io>
{
	public:
		JavaEventListenerManager(JavaSimulator& owner);
		void outputEvent(Event<java_io> x, double t);
		void stateChange(Atomic<java_io>* model, double t);
		void addJavaListener(jobject local_obj_ref);
		void removeJavaListener(jobject local_obj_ref);
		~JavaEventListenerManager();
	private:
		JavaSimulator& owner;
		std::list<jobject> global_refs;
};

/**
 * Base class for model wrappers.
 */
class JavaDevs
{
	public:
		JavaDevs(jobject jmodel_loc_ref, JavaSimulator& owner);
		virtual ~JavaDevs();
		jobject getJavaObjRef() { return jmodel_global_ref; }
		void clear_peer();
	protected:
		JavaSimulator& owner;
		void set_peer(void* obj_addr);
	private:
		jobject jmodel_global_ref;
};

/**
 * Wrapper around a Jave atomic model.
 */
class JavaAtomic:
	public Atomic<java_io>,
	public JavaDevs
{
	public:
		JavaAtomic(jobject jmodel_loc_ref, JavaSimulator& owner);
		void delta_int();
		void delta_ext(double e, const Bag<java_io>& xb);
		void delta_conf(const Bag<java_io>& xb);
		double ta();
		void output_func(Bag<java_io>& yb);
		void gc_output(Bag<java_io>& yb);
		bool model_transition();
		~JavaAtomic();
};

/**
 * Wrapper around a Java network model.
 */
class JavaNetwork:
	public Network<java_io>,
	public JavaDevs
{
	public:
		JavaNetwork(jobject jmodel_loc_ref, JavaSimulator& owner);
		void getComponents(Set<Devs<java_io>*>& c);
		void route(const java_io& x, Devs<java_io>* model, Bag<Event<java_io> >& r);
		bool model_transition();
		~JavaNetwork();
};

} // end of namespace

#endif
