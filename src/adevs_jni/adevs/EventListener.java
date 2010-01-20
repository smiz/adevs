package adevs;

/**
 * The EventListener interface is implemented by classes
 * that will register with a Simulator to receive notification
 * when changes occur in a model's state and output.
 *
 * @see Simulator
 */
public interface EventListener
{
	/**
	 * This method is called when a model produces an output.
	 * The time t is the time when the output occurs
	 * and the Event has the model that produced the output
	 * and the output's value.
	 *
	 * @param	x	An Event holding the model that produced the output and the output itself
	 * @param	t	The time that the output occurred
	 * @see	Event
	 */
	public void outputEvent(Event x, double t);
	/**
	 * This method is called immediately after an Atomic model
	 * changes its state.
	 *
	 * @param	model	The model that changed state
	 * @param	t	The time at which the change occurred
	 * @see Atomic
	 */
	public void stateChange(Atomic model, double t);
}
