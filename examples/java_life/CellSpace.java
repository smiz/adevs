import adevs.*;
import java.util.Collection;

/**
 * This class implements a 2D space of Cells for the
 * game of life. The specified fraction of the cells are
 * set to an alive initial condition.
 */
public class CellSpace extends Network
{
	/**
	 * Create a space of dimensions w x h
	 * with p percent of the cells started
	 * in an live state.
	 */
	public CellSpace(int w, int h, double p)
	{
		super();
		this.w = w;
		this.h = h;
		cells = new Cell[w][h];
		// Instantiate the cells
		for (int i = 0; i < w; i++)
		{
			for (int j = 0; j < h; j++)
			{
				cells[i][j] =
					new Cell(i,j,Math.random() < p);
			}
		}
	}
	/**
	 * Route an event inside the CellSpace.
	 */
	 public void route(Object x, Devs model, Collection<Event> r)
	 {
		 Cell cell = (Cell)model;
		 int centerx = cell.getX();
		 int centery = cell.getY();
		 // Add the neighboring cells to the set of receivers
		 for (int i = -1; i <= 1; i++)
		 {
			 for (int j = -1; j <= 1; j++)
			 {
				 if (i != j)
				 {
					 // Wrap the space at the edges
					 int xtarget = (centerx+i);
					 int ytarget = (centery+j);
					 if (xtarget < 0) xtarget = w-1;
					 else if (xtarget >= w) xtarget = 0;
					 if (ytarget < 0) ytarget = h-1;
					 else if (ytarget >= h) ytarget = 0;
					 r.add(new Event(cells[xtarget][ytarget],x));
				 }
			 }
		 }
	 }
	 /**
	  * Get the set of cells in this CellSpace.
	  */
	 public void getComponents(Collection<Devs> c)
	 {
		 for (int i = 0; i < w; i++)
		 {
			 for (int j = 0; j < h; j++)
			 {
				 c.add(cells[i][j]);
			 }
		 }
	 }
	/**
	 * Get the width of the space
	 */
	 public int getWidth() { return w; }
	 /**
	  * Get the height of the space
	  */
	 public int getHeight() { return h; }

	 private Cell [][] cells; // The array of Cell models
	 private int w, h; // Dimensions of the CellSpace
}
