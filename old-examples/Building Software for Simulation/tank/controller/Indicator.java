import javax.swing.JComponent;
import javax.swing.JFrame;
import java.awt.Graphics;
import java.awt.geom.Rectangle2D;
import java.awt.Insets;
import java.awt.Color;
/**
Dial and numeric indicator for the pilot display.
*/
public class Indicator extends JComponent
{
	/**
	Create an indicator with the specified label and range.
	*/
	public Indicator(String name, double min, double max)
	{
		super();
		this.name = name;
		this.min = min;
		this.value = min;
		rad_per_unit = Math.PI/(max-min);
	}
	protected void paintComponent(Graphics g)
	{
		super.paintComponent(g);
		Color color = g.getColor();
		// Draw the label
		g.setColor(Color.black);
		Insets inset = getInsets();
		Rectangle2D str_bounds = g.getFontMetrics().getStringBounds(name,g);
		int str_height = (int)str_bounds.getHeight();
		g.drawString(name,inset.left,inset.top+str_height);
		// Draw the indicator
		float r = (float)(getHeight()-str_height-inset.top);
		if (r > (float)getWidth()/2) r = (float)(getWidth()/2);
		int x1 = getWidth()/2;
		int y1 = getHeight()-inset.bottom-3;
		// Draw indicator dots at every fifth position
		g.setColor(Color.blue);
		for (int i = Byte.MIN_VALUE; i <= Byte.MAX_VALUE; i += 15)
		{
			double angle = 0.5*Math.PI*(double)(i)/(double)128;
			int x2 = x1+(int)(r*Math.sin(angle));
			int y2 = y1-(int)(r*Math.cos(angle));
			g.drawOval(x2,y2,3,3);
		}
		// Draw the indicator
		g.setColor(Color.black);
		double angle = rad_per_unit*(value-min) - Math.PI/2.0;
		int x2 = x1+(int)(r*Math.sin(angle));
		int y2 = y1-(int)(r*Math.cos(angle));
		g.drawLine(x1,y1,x2,y2);
		g.setColor(color);
	}
	void setValue(double value)
	{
		this.value = value;
		repaint();
	}
 
	private String name;
	private double value, min, rad_per_unit;

	public static void main(String args [])
	{
		JFrame frame = new JFrame();
		Indicator indicator = new Indicator("Label",0.0,1.0);
		frame.getContentPane().add(indicator);
		frame.setBounds(100,100,100,100);
		frame.setVisible(true);
		for (double k = 0.0; k < 1.0; k += 0.01)
		{
			try
			{
				Thread.sleep(100);
			}
			catch(Exception err)
			{
			}
			indicator.setValue(k);
		}
	}
}
