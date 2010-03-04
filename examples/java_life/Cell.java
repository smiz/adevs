import adevs.*;
import java.util.Collection;

/**
 * A single cell in the game of life.
 */
class Cell extends Atomic<Boolean>
{
	/**
	 * Create a cell at position x,y and set its initial
	 * state.
	 */
	public Cell(int x, int y, boolean is_alive)
	{
		super();
		this.x = x; 
		this.y = y;
		this.is_alive = is_alive;
		// Negative value triggers an output a t = 0
		alive_neighbors = -1;
	}
	/**
	 * Get the x position of the cell.
	 */
	public int getX() { return x; }
	/**
	 * Get the y position of the cell.
	 */
	public int getY() { return y; }
	/**
	 * Time advance function
	 */
	public double ta()
	{
		// Send initial output?
		if (alive_neighbors == -1 && is_alive)
			return 0.0;
		// If a phase change should occur
		else if (check_death_rule() // cell will die
			|| check_born_rule()) // cell will be birthed
			return 1.0;
		// Otherwise, do nothing
		return Double.MAX_VALUE;
	}
	/**
	 * Internal transition function
	 */
	public void delta_int() 
	{
		if (alive_neighbors < 0) alive_neighbors = 0;
		// Change the cell state if necessary
		if (check_death_rule()) is_alive = false;
		else if (check_born_rule()) is_alive = true;
	}
	/**
	 * External transition function
	 */
	public void delta_ext(double e, Collection<Boolean> xb) 
	{
		if (alive_neighbors < 0) alive_neighbors = 0;
		// Update the count if living neighbors
		for (Boolean x: xb)
		{
			if (x.booleanValue() == true)
				alive_neighbors++;
			else
				alive_neighbors--;
		}
	}
	/**
	 * Confluent transition function
	 */
	public void delta_conf(Collection<Boolean> xb) 
	{
		delta_int();
		delta_ext(0.0,xb);
	}
	/**
	 * Output function
	 */	
	public void output_func(Collection<Boolean> yb) 
	{
		// Check in case this in not true
		if (alive_neighbors == -1)
			yb.add(new Boolean(is_alive));
		else if (check_born_rule())
			yb.add(new Boolean(true));
		else yb.add(new Boolean(false));
	}
	/**
	 * Should the cell be birthed?
	 */
	private boolean check_born_rule()
	{
		return (!is_alive && alive_neighbors == 3);
	}
	/**
	 * Should the cell expire?
	 */
	private boolean check_death_rule()
	{
		return (is_alive && (alive_neighbors < 2 || alive_neighbors > 3));
	}

	private int x, y; // Position of the cell
	private int alive_neighbors; // Number of living neighbors
	private boolean is_alive; // Cell's state
}
