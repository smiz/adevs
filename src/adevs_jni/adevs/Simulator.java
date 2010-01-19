package adevs;
import java.util.Collection;
import java.util.ArrayList;

/**
 * This is a wrapper around the adevs Simulator class. It is used in exactly the
 * same way. The java version of this simulator does not support parallel simulation
 * (yet).
 */
public class Simulator
{
	/// Create a simulator for an Atomic model
	public Simulator(Devs model)
	{
		// These objects are needed by the native simulator
		shared_event = new adevs.Event();
		shared_coll = new ArrayList(); 
		// Now create the native simulator
		Cpp_SimID = createCppSimulator(model);
	}
	/// Get the absolute time of the model's next event
	public double nextEventTime() { return nextEventTime(Cpp_SimID); }
	/// Compute the output and next state at the next event time
	public void execNextEvent() { execNextEvent(Cpp_SimID); }
 	/// Execute until nextEventTime() > tend. 
	public void execUntil(double tend) { execUntil(tend,Cpp_SimID); }
	/// Compute the output at the time of the next event
	public void computeNextOutput() { computeNextOutput(Cpp_SimID); }
	/// Inject input into the model at the specified time
	public void computeNextState(Collection<Event> input, double t)
	{
		computeNextState(input,t,Cpp_SimID);
	}
	/// Register a listener to receive callbacks when output and changes in state occur.
	public void addEventListener(EventListener l)
	{
		addEventListener(l,Cpp_SimID);
	}
	/// Unregister an EventListener
	public void removeEventListener(EventListener l)
	{
		removeEventListener(l,Cpp_SimID);
	}
	/**
	 * Explicitly release native resources. This must be called if you plan to reuse
	 * a model in a new instance of a Simulator. The Simulator can not be used
	 * after this method is called. Subsequent method calls will have not effect.
	 */
	public void dispose()
	{
		destroyCppSimulator(Cpp_SimID);
		Cpp_SimID = 0;
	}
	/// Finalizer deletes native resources
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
