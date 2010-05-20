import adevs.*;

public class ServerSim
{
	public static Devs buildModel()
	{
		ServerWithQueue q1 = new ServerWithQueue(0.5);
		ServerWithQueue q2 = new ServerWithQueue(0.5);
		SimpleDigraph server = new SimpleDigraph();
		server.add(q1);
		server.add(q2);
		server.couple(q1,q2);
		server.couple(server,q1);
		server.couple(q2,server);
		g = new GenrAndTransd(1.0);
		SimpleDigraph model = new SimpleDigraph();
		model.add(server);
		model.add(g);
		model.couple(server,g);
		model.couple(g,server);
		return model;
	}

	public static void main(String args[])
	{
		Devs model = buildModel();
		Simulator sim = new Simulator(model);
		while (sim.nextEventTime() <= 10)
		{
			sim.execNextEvent();
		}
		System.out.println("in = " + g.getInCount() +
				", out = " + g.getOutCount());
		if (g.getOutCount() != 10 || g.getInCount() != 9)
		{
			System.err.println("Test FAILED");
			System.exit(-1);
		}
		else
		{
			System.err.println("Test PASSED");
		}
	}

	public static GenrAndTransd g = null;
}
