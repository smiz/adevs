package adevs;

/**
 * These exceptions are thrown by the simulator to indicate problems
 * at execution time.
 */
public class SimulationException extends Exception
{
	/**
	 * Create an exception attributed to a specific model.
	 * @param msg Description of the error
	 * @param src The model that caused the problem
	 */
	public SimulationException(String msg, Devs src)
	{
		super(msg);
		this.src = src;
	}
	/**
	 * Create an exception not attributed to any particular model.
	 */
	public SimulationException(String msg)
	{
		this(msg,null);
	}
	/**
	 * Get the model that caused the error, or null if it is not
	 * attributed to a model.
	 */
	public Devs who()
	{
		return src;
	}
	
	public Devs src;
}
