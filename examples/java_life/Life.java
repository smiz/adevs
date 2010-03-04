import adevs.*;

/**
 * Main function for the Game of Life.
 */
public class Life
{
	public static void main(String args[])
	{
		// Create the model
		CellSpace gol = new CellSpace(20,20,0.3);
		// Create the simulator
		Simulator sim = new Simulator(gol);
		// Create the display and register it with the simulator
		Display disp = new Display(gol);
		sim.addEventListener(disp);
		// Run the simulation for 10 generations
		while (sim.nextEventTime() <= 10.0)
		{
			System.out.println("----- " +
					sim.nextEventTime() + "-----");
			sim.execNextEvent();
			disp.update();
			// Wait 1 second between each generation
			try
			{
				Thread.sleep(1000);
			}
			catch(InterruptedException exp)
			{
			}
		}
	}
}
