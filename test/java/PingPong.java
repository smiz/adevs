import adevs.*;
import java.util.Collection;
import java.util.Iterator;

public class PingPong extends Network
{
	public class Player extends Atomic<String>
	{
		public Player(boolean has_ball)
		{
			super();
			count = 0;
			t = 0.0;
			even = this.has_ball = has_ball;
		}
		public void delta_int()
		{
			t += ta();
			if ((even && (int)t%2 != 1) || (!even && (int)t%2 != 0))
			{
				System.out.println("Bad internal event");
				System.exit(0);
			}
			count++;
			has_ball = false;
		}
		public void delta_ext(double e, Collection<String> xb)
		{
			int count = 0;
			t += e;
			Iterator iter = xb.iterator();
			while (iter.hasNext()) 
			{
				count++;
				iter.next();
			}
			if (count != 2) 
			{
				System.out.println("Not enough input");
				System.exit(0);
			}
			has_ball = true;
			if ((even && (int)t%2 != 0) || (!even && (int)t%2 != 1))
			{
				System.out.println("Bad internal event");
				System.exit(0);
			}
		}
		public void delta_conf(Collection<String> xb)
		{
			delta_int();
			delta_ext(0.0,xb);
		}
		public double ta()
		{
			if (has_ball && count < 10) return 1.0;
			else return Double.MAX_VALUE;
		}
		public void output_func(Collection<String> yb)
		{
			yb.add(new String("Ping"));
			yb.add(new String("Pong"));
		}
		public boolean model_transition()
		{
			return false;
		}
		private int count;
		private boolean has_ball, even;
		private double t;
	}
	public PingPong()
	{
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

	private Player p1, p2;

	public static void main(String args[])
	{
		long mem_start = Runtime.getRuntime().freeMemory();
		for (int i = 0; i < 1000; i++)
		{
			test();
			System.gc();
			long mem_end = Runtime.getRuntime().freeMemory();
			long diff = mem_end-mem_start;
			System.out.println("Final memory consumed " + diff);
		}
	}
	public static void test()
	{
		double t = 0.0;
		Simulator sim = new Simulator(new PingPong());
		while (sim.nextEventTime() < Double.MAX_VALUE)
		{
			t += 1.0;
			if (sim.nextEventTime() != t)
			{
				System.out.println("Bad time of next event");
				System.exit(0);
			}
			sim.execNextEvent();
		}
	}
}

