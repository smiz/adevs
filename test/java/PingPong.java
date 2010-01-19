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
				System.out.println(iter.next().toString());
			has_ball = true;
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
		private boolean has_ball;
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
		for (int i = 0; i < 100000; i++)
			test();
	}
	public static void test()
	{
		Simulator sim = new Simulator(new PingPong());
		while (sim.nextEventTime() < Double.MAX_VALUE)
		{
			System.out.println(new String("t = ")+ sim.nextEventTime());
			sim.execNextEvent();
		}
	}
}

