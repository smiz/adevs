package adevs;
import java.util.Collection;

/**
 * This is the Java interface for a DEVS network model.
 */
public abstract class Network extends Devs
{
	public Network()
	{
		super(false);
	}
	/**
	 * Fill the collection with the network's components.
	 * Duplicates will be ignored by the simulator. Unlike
	 * the C++ simulator, the parent of a component is
	 * automatically set to the model whose component set
	 * it appears in.
	 */
	public abstract void getComponents(Collection<Devs> c);
	/**
	 * Route events coming into to the model or being produced
	 * by one of its components.
	 */
	public abstract void route(Object x, Devs model, Collection<Event> r);
}
