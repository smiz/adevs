import adevs.*;
import java.util.Collection;
import java.util.Iterator;

public class ExceptionTestModel
	extends AtomicTestModel
{
	public ExceptionTestModel(double time_advance)
	{
		super();
		sigma = time_advance;
	}
	public void delta_int()
	{
		super.delta_int();
		causeThrow();
	}
	public void delta_ext(double e, Collection<String> xb)
	{
		super.delta_ext(e,xb);
		causeThrow();
	}
	public void delta_conf(Collection<String> xb)
	{
		super.delta_conf(xb);
		causeThrow();
	}
	public void output_func(Collection<String> yb)
	{
		super.output_func(yb);
		causeThrow();
	}
	public double ta() { return sigma; }
	public boolean model_transition()
	{
		causeThrow();
		return super.model_transition();
	}
	private void causeThrow()
	{
		Double tmp = null;
		tmp.doubleValue();
	}

	private double sigma;
}

