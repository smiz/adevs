import adevs.*;
import java.util.LinkedList;
import java.util.Collection;

public class DigraphTest
{
	public static class Parrot extends Atomic<Digraph.PortValue>
	{
		// Ports for incoming/outgoing jobs
		public static final int shout = 0;
		public static final int whisper = 1;
		// Port for outgoing response
		public static final int output = 2;

		public Parrot(String name)
		{
			this(name,null);
		}
		public Parrot(String name, String word)
		{
			super();
			this.name = new String(name);
			q = new LinkedList<String>();
			if (word != null)
				q.add(word);
		}
		public void delta_int()
		{
			q.removeLast();
		}
		public void delta_ext(double e, Collection<Digraph.PortValue> xb)
		{
			for (Digraph.PortValue x: xb)
			{
				// SHOUT input arriving on shout
				if (x.getPort() == shout)
					q.add(new String(x.getValue().toString()).toUpperCase());
				// whisper input arriving on whisper
				else if (x.getPort() == whisper)
					q.add(new String(x.getValue().toString()).toLowerCase());
			}
		}
		public void delta_conf(Collection<Digraph.PortValue> xb)
		{
			delta_int();
			delta_ext(0.0,xb);
		}
		public void output_func(Collection<Digraph.PortValue> yb)
		{
			// Say something
			System.out.println(name + " says \"" + q.getFirst() + "\"");
			yb.add(new Digraph.PortValue(output,q.getFirst()));
		}
		public double ta()
		{
			if (q.isEmpty()) return Double.MAX_VALUE;
			else return 1.0;
		}

		private String name;
		private LinkedList<String> q;
	}

	public static void main(String args[])
	{
		// Make a digraph model with three birds
		Parrot dusty = new Parrot("Dusty","bird, bird, bird: bird is the word");
		Parrot lefty = new Parrot("Lefty");
		Parrot ned = new Parrot("Ned");
		Digraph model = new Digraph();
		model.add(dusty);
		model.add(lefty);
		model.add(ned);
		// dusty -> lefty; lefty will shout
		model.couple(dusty,dusty.output,lefty,lefty.shout);
		// lefty -> ned; ned will whisper
		model.couple(lefty,lefty.output,ned,ned.whisper);
		// ned -> dusty; dusty will shout messages from ned
		model.couple(ned,ned.output,dusty,dusty.shout);
		// Simulate the model
		Simulator sim = new Simulator(model);
		while (sim.nextEventTime() <= 10)
		{
			System.out.println("@ t = " + sim.nextEventTime());
			sim.execNextEvent();
		}
	}
}
