package adevs;

/**
 * The Event class is used to inject models into a running
 * simulation and to route input and output within a network
 * model, and for receiving notifications of output at an
 * EventListener.
 */
public class Event
{
	/// The model produced or should receive the event
	public Devs model;
	/// The value of the event
	public Object value;
	/// Create an event with both fields set to null
	public Event()
	{
		this(null,null);
	}
	/// Create an event with both fields set
	public Event(Devs model, Object value)
	{
		this.model = model;
		this.value = value;
	}
}
