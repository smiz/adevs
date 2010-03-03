import adevs.*;
import java.util.Collection;
import java.util.Iterator;

public class DynStructPingPong extends Network
{
	public class Player extends Atomic<String>
	{
		public boolean hasBall() { return has_ball; }
		public Player(boolean has_ball)
		{
			super();
			count = 0;
			this.has_ball = has_ball;
		}
		public void delta_int()
		{
			count++;
			has_ball = false;
		}
		public void delta_ext(double e, Collection<String> xb)
		{
			Iterator iter = xb.iterator();
			while (iter.hasNext())
			{
				if (!DynStructPingPong.quiet)
					System.out.println(iter.next().toString());
				else iter.next();
			}
			has_ball = true;
		}
		public void delta_conf(Collection<String> xb)
		{
			delta_int();
			delta_ext(0.0,xb);
		}
		public double ta()
		{
			if (has_ball) return 1.0;
			else return Double.MAX_VALUE;
		}
		public void output_func(Collection<String> yb)
		{
			yb.add(new String("Ping"));
			yb.add(new String("Pong"));
		}
		@Override
		public boolean model_transition()
		{
			return count > 2;
		}
		private int count;
		private boolean has_ball;
	}

	public DynStructPingPong()
	{
		super();
		p1 = new Player(true);
		p2 = new Player(false);
	}
	public void getComponents(Collection<Devs> c)
	{
		c.add(p1);
		c.add(p2);
	}
	public void route(Object x, Devs model, Collection<Event> r)
	{
		if (model == p1) r.add(new Event(p2,x));
		else r.add(new Event(p1,x));
	}
	@Override
	public boolean model_transition()
	{
		instances++;
		if (p1 instanceof Atomic)
			p1 = new DynStructPingPong();
		else
			p1 = new Player(!((Player)p2).hasBall());
		if (!DynStructPingPong.quiet)
			System.out.println("pnew #" + instances);
		return true;
	}

	private Devs p1, p2;
	private static int instances = 0;
	public static boolean quiet = false;

	public static void main(String args[])
	{
		test();
		long mem_start = Runtime.getRuntime().freeMemory();
		for (int i = 0; i < 10; i++)
		{
			quiet = true;
			instances = 0;
			test();
			System.gc();
			long mem_end = Runtime.getRuntime().freeMemory();
			System.err.println("Memory consumed is " + (mem_end-mem_start));
		}
		System.err.println("These numbers should not increase substantially");
	}
	public static void test()
	{
		Simulator sim = new Simulator(new DynStructPingPong());
		while (sim.nextEventTime() < 1000.0)
		{
			if (!DynStructPingPong.quiet)
				System.out.println(new String("t = ")+ sim.nextEventTime());
			sim.execNextEvent();
		}
	}
}

