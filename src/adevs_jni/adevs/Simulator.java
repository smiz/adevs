package adevs;
import java.util.Collection;
import java.util.ArrayList;

/**
 * This is a wrapper around the C++ Simulator class. It is used in exactly the
 * same way. The Java version of this simulator does not support parallel simulation
 * (yet). 
 */
public class Simulator
{
	/**
	 * Creates a simulator for an Atomic or Network model.
	 *
 	 * @param	model	The Network or Atomic model that you want to simulate
	 * @see	Network Atomic
	 */
	public Simulator(Devs model)
	{
		// These objects are needed by the native simulator
		shared_event = new adevs.Event();
		shared_coll = new ArrayList(); 
		// Now create the native simulator
		Cpp_SimID = createCppSimulator(model);
	}
	/**
	 * Get the time of the model's next event.
	 *
	 * @return	the time of the next internal event
	 */
	public double nextEventTime() { return nextEventTime(Cpp_SimID); }
	/**
	 * Compute the output and next state at the next event time. This invokes
	 * callbacks on the registered listeners.
	 *
	 * @see	EventListener
	 */
	public void execNextEvent() { execNextEvent(Cpp_SimID); }
 	/**
	 * Simulate until nextEventTime() is greater than tend. This invokes
	 * callbacks on registered listeners as the simulation progresses.
	 * Note that tend must be greater than or equal to the time of the
	 * last event.
	 *
	 * @param	tend	time to stop the simulation
	 * @see EventListener
	 */
	public void execUntil(double tend) { execUntil(tend,Cpp_SimID); }
	/**
	 * Compute the output at the time of the next internal event. This
	 * invokes the outputEvent method of registered listeners, but it
	 * does not change the state of the model or advance the simualtion
	 * clock.
	 *
	 * @see	EventListener
	 */
	public void computeNextOutput() { computeNextOutput(Cpp_SimID); }
	/**
	 * Inject input into the model at the specified time. The time t
	 * must be greater than the time of the last event and less than or
	 * equal to the time of the next internal event. This invokes callbacks
	 * on registered listeners.
	 *
	 * @param	input	The values to inject and the target models
	 * @param	t	Time to apply the input
	 * @see	Event
	 */
	public void computeNextState(Collection<Event> input, double t)
	{
		computeNextState(input,t,Cpp_SimID);
	}
	/**
	 * Register a listener to receive callbacks when output and changes in state occur.
	 *
	 * @param	l	The listener to register with the simulator
	 * @see	EventListener
	 */
	public void addEventListener(EventListener l)
	{
		addEventListener(l,Cpp_SimID);
	}
	/**
	 * Unregister a listener so that it will not receive callbacks from the simulator.
	 *
	 * @param	l	The listener to unregister
	 * @see	EventListener
	 */
	public void removeEventListener(EventListener l)
	{
		removeEventListener(l,Cpp_SimID);
	}
	/**
	 * Release the native resources used by this simulator.
	 * This must be called if you plan to reuse a model in a new instance
	 * of a Simulator. This Simulator object will not respond
	 * to method calls after its native resources are disposed of.
	 */
	public void dispose()
	{
		destroyCppSimulator(Cpp_SimID);
		Cpp_SimID = 0;
	}
	/**
	 * Finalizer deletes native resources if they have not already
	 * been taken care of by a call to dispose().
	 */
	protected void finalize()
	{
		dispose();
	}
	/// Identifies the C++ Simulator that belongs to this class
	private long Cpp_SimID; 
	/// This a collection for common use by the native model wrappers
	private Collection shared_coll;
	/// This is an event to be used in listener callbacks
	private adevs.Event shared_event;
	/// Create a simulator for a model. Returns the SimID.
	private native long createCppSimulator(Devs model);
	/// Get the absolute time of the model's next event
	private native double nextEventTime(long Cpp_SimID);
	/// Compute the output and next state at the next event time
	private native void execNextEvent(long Cpp_SimID);
 	/// Execute until nextEventTime() > tend. 
	private native void execUntil(double tend, long Cpp_SimID);
	/// Compute the output at the time of the next event
	private native void computeNextOutput(long Cpp_SimID);
	/// Inject input into the model at the specified time
	private native void computeNextState(Collection<Event> input, double t, long Cpp_SimID);
	/// Register a listener to receive callbacks when output and changes in state occur.
	private native void addEventListener(EventListener l, long Cpp_SimID);
	/// Unregister an EventListener
	private native void removeEventListener(EventListener l, long Cpp_SimID);
	/// Destroy the native simulator
	private native void destroyCppSimulator(long Cpp_SimID);

	/// Load the native code
	static
	{
		System.loadLibrary("java_adevs");
	}
}
