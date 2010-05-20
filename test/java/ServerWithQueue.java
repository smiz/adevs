import adevs.*;
import java.util.LinkedList;
import java.util.Collection;

/**
 * A server with infinite queue.
 */
public class ServerWithQueue extends Atomic<Object>
{
	/**
	 * Create a server with empty queue.
	 */
	public ServerWithQueue(double service_time)
	{
		super();
		this.ttg = this.service_time = service_time;
		this.q = new LinkedList<Object>();
	}
	/// Internal transition function
	public void delta_int()
	{
		ttg = service_time;
		q.removeLast();
	}
	/// External transition function
	public void delta_ext(double e, Collection<Object> xb)
	{
		if (!q.isEmpty()) ttg -= e;
		q.add(xb);
	}
	/// Confluent transition function
	public void delta_conf(Collection<Object> xb)
	{
		delta_int();
		delta_ext(0,xb);
	}
	/// Time advance function
	public double ta()
	{
		if (q.isEmpty()) return Double.MAX_VALUE;
		else return ttg;
	}
	/// Output function
	public void output_func(Collection<Object> yb)
	{
		yb.add(q.getFirst());
	}
	/// Service time and time to serve first item in list
	private double service_time, ttg;
	/// Queue for holding jobs
	private LinkedList<Object> q;
}
