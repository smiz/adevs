import adevs.*;

/**
 * This event listener displays the state of the cell space.
 */
public class Display implements EventListener
{
	/**
	 * Create a display for Cells in a CellSpace.
	 */
	public Display(CellSpace space)
	{
		w = space.getWidth();
		h = space.getHeight();
		state = new char[w][h];
	}
	/**
	 * Update the displayable state when a Cell produces an
	 * output.
	 */
	public void outputEvent(Event x, double t)
	{
		Cell cell = (Cell)x.model;
		Boolean value = (Boolean)x.value;
		if (value.booleanValue())
			state[cell.getX()][cell.getY()] = '*';
		else
			state[cell.getX()][cell.getY()] = ' ';
	}
	// Unused callback for state changes
	public void stateChange(Atomic model, double t){}
	// Update the display
	void update()
	{
		for (int j = 0; j < h; j++)
		{
			for (int i = 0; i < w; i++)
			{
				System.out.print(state[i][j]);
			}
			System.out.print('\n');
		}
	}

	private int w, h;
	private char [][] state;
}
