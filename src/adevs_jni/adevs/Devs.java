package adevs;

/**
 * This is the base class for all DEVS models.
 */
public abstract class Devs
{
	/**
	 * This is the Dynamic DEVS model transition function.
	 * It returns false by default.
	 */
	public boolean model_transition() { return false; }
	// Constructor sets the flag for the model's type
	protected Devs(boolean is_atomic)
	{
		this.is_atomic = is_atomic;
		cpp_peer = 0;
	}
	// This is set by the cpp Simulator to indicate that
	// The model's C++ peer
	private long cpp_peer;
	// Used by the C++ simulator to typecheck models
	private boolean is_atomic;
	// Get the identification of the native peer. 
	public final long getNativePeer() { return cpp_peer; }
}
