import adevs.*;
import java.util.Collection;
import java.util.Iterator;

public class AtomicTestModel extends Atomic<String>
{
	public AtomicTestModel()
	{
		super();
	}
	public void delta_int() { System.out.println("Internal event"); }
	public void delta_ext(double e, Collection<String> xb)
	{
		System.out.println("External event");
		System.out.println("\te="+e);
		Iterator iter = xb.iterator();
		while (iter.hasNext())
			System.out.println("\t"+iter.next().toString());
	}
	public void delta_conf(Collection<String> xb)
	{
		System.out.println("Confluent event");
		Iterator iter = xb.iterator();
		while (iter.hasNext())
			System.out.println("\t"+iter.next().toString());
	}
	public void output_func(Collection<String> yb)
	{
		System.out.println("Output");
		yb.add(new String("An output"));
		yb.add(new String("Another output"));
	}
	public double ta() { return 1.0; }
	public boolean model_transition()
	{
		System.out.println("Model transition");
		return false;
	}
}

