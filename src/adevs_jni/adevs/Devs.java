package adevs;

/**
 * This is the base class for all DEVS models.
 * @see Network
 * @see Atomic
 */
public abstract class Devs
{
	/**
	 * This is the Dynamic DEVS model transition function. For Atomic models,
	 * this method is called after each change of the model's state. If the
	 * method returns true, the model_transition method of the model's parent
	 * (a Network) is called. For a Network model, this method may be used
	 * to add and remove components the the model's set of components. Only models
	 * added and removed by the model_transition function are guaranteed to
	 * be properly handled by the Simulator. Method returns false by default.
 	 * @return Returns true if a change in model structure should occur that
	 * involves this component model and its children.
	 */
	public boolean model_transition() { return false; }
	/// This is set by the cpp Simulator to keep track of the C++ peer
	private long cpp_peer;
	/// Used by the C++ simulator to typecheck models
	private boolean is_atomic;
	/**
	 * Get the identification of the native peer.
	 * @return The address of the C++ object that is the native 
	 * reflection of this object.
	 */
	public final long getNativePeer() { return cpp_peer; }
	/**
	 * Constructor sets the flag for the model's type
	 * @param is_atomic True if this is an atomic model, false otherwise.
	 */
	protected Devs(boolean is_atomic)
	{
		this.is_atomic = is_atomic;
		cpp_peer = 0;
	}
}
