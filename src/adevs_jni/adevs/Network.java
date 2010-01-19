package adevs;
import java.util.Collection;

/**
 * This is the base class for all Network (coupled) models.
 */
public abstract class Network extends Devs
{
	public Network()
	{
		super(false);
	}
	/**
	 * The collection must be Fill with Network's components.
	 * Duplicates will be ignored by the simulator. Unlike
	 * the C++ simulator, the parent of a component is
	 * automatically set to be the Network whose set of
	 * components it appears in.
	 *
	 * @param	c	Empty collection to fill with the Network's components
	 */
	public abstract void getComponents(Collection<Devs> c);
	/**
	 * Route events coming into to the model or being produced
	 * by one of its components. The collection is filled with
	 * events whose model field is the target of its value field.
	 *
	 * @param	x	The value of the event to be routed
	 * @param	model	The source of the event
	 * @param	r	An empty collection to be filled with (target,value) pairs
	 * @see	Event
	 */
	public abstract void route(Object x, Devs model, Collection<Event> r);
}
