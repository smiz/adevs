import adevs.*;
import java.util.Collection;
import java.util.ArrayList;

public class AtomicTest implements EventListener
{
	public static Atomic model = null;

	public static void main(String args[])
	{
		test();
		testJavaException();
		testAdevsException();
	}

	public static void testJavaException()
	{
		System.out.println("Exception test");
		model = new ExceptionTestModel(1.0);
		try
		{
			Simulator sim = new Simulator(model);
			sim.execNextEvent();
		}
		catch(Exception exp)
		{
			exp.printStackTrace();
		}
	}

	public static void testAdevsException()
	{
		System.out.println("adevs Exception test");
		model = new ExceptionTestModel(-1.0);
		try
		{
			Simulator sim = new Simulator(model);
			sim.execNextEvent();
		}
		catch(Exception exp)
		{
			exp.printStackTrace();
		}
	}

	public static void test()
	{
		if (model == null)
			model = new AtomicTestModel();
		Simulator sim = new Simulator(model);
		sim.addEventListener(new AtomicTest());
		int i = 0;
		for (; i < 5; i++)
		{
			if (sim.nextEventTime() != i+1)
			{
				System.out.println("FAIL: bad tN");
				System.exit(0);
			}
			sim.execNextEvent();
		}
		ArrayList<adevs.Event> input = new ArrayList<adevs.Event>();
		input.add(new adevs.Event(model,new String("an input")));
		input.add(new adevs.Event(model,new String("another input")));
		sim.computeNextState(input,(double)i + 0.5);
		sim.execNextEvent();
		sim.computeNextState(input,sim.nextEventTime());
		sim.dispose();
		if (model.getNativePeer() == 0)
			System.out.println("Peer is 0");
		else
		{
			System.out.println("Peer not reset! " + model.getNativePeer());
			System.exit(0);
		}
	}
	public void outputEvent(Event x, double t)
	{
		if (model == x.model)
			System.out.println("outputEvent " + x.value.toString() +
				" at t = " + t);
		else System.out.println("outputEvent FAILED");
	}
	public void stateChange(Atomic callback_model, double t)
	{
		if (model == callback_model)
			System.out.println("stateChange at t = " + t);
		else System.out.println("stateChange FAILED");
	}
}

