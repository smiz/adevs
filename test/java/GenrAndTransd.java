import adevs.*;
import java.util.Collection;

/**
 * Generator and transducer for the server with queue.
 */
public class GenrAndTransd extends Atomic<Object>
{
	public GenrAndTransd(double period)
	{
		super();
		in = out = 0;
		ttg = this.period = period;
	}
	/// Internal transition function
	public void delta_int()
	{
		out++;
		ttg = period;
	}
	/// External transition function
	public void delta_ext(double e, Collection<Object> xb)
	{
		ttg -= e;
		in += xb.size();
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
		return ttg;
	}
	/// Output function
	public void output_func(Collection<Object> yb)
	{
		yb.add(new Object());
	}
	/// Number of output produced
	public int getInCount() { return in; }
	/// Number of input seen
	public int getOutCount() { return out; }
	/// Service time and time to serve first item in list
	private double period, ttg;
	private int in, out;
}
