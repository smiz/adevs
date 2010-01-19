package adevs;

/**
 * The EventListener interface is implemented by classes
 * that will register with a Simulator to receive notification
 * when changes occur in a model's state and output.
 */
public interface EventListener
{
	/**
	 * This method is called when a model produces an output.
	 * The time t is the absolute time when the output occurs
	 * and the Event has the model that produced the output
	 * and the output value.
	 */
	public void outputEvent(Event x, double t);
	/**
	 * This method is called immediately after an Atomic model
	 * changes its state.
	 */
	public void stateChange(Atomic model, double t);
}
