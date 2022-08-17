package adevs;

/**
 * The Event class is used to inject models into a running
 * simulation, to route input and output within a network
 * model, and to receive notifications of output by an
 * EventListener.
 */
public class Event
{
	/**
	 * The model that produced or should receive the event.
	 * @see	Simulator 
	 * @see	EventListener
	 */
	public Devs model;
	/**
	 * The value of the input or output event
	 * @see Simulator
	 * @see	EventListener
	 */
	public Object value;
	/**
	 * Create an event with both its model and value set to null
	 */
	public Event()
	{
		this(null,null);
	}
	/**
	 * Create an event with both fields set. The model is the target if the Object
	 * is an input, and the model is the source if the Object is an output.
	 * @param model The model to be affected by this event.
	 * @param value The input or output value for this event.
	 */
	public Event(Devs model, Object value)
	{
		this.model = model;
		this.value = value;
	}
}
