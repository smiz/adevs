import javax.swing.JFrame;
import java.awt.GridBagLayout;
import java.awt.GridBagConstraints;
import javax.swing.JSlider;
import javax.swing.SwingConstants;
import java.awt.GridLayout;
import javax.swing.JPanel;
/**
This is the pilot's display.
*/
public class DriverDisplay
{
	/**
	Create a display with the initial instrumentation
	data.
	*/
	public DriverDisplay(float max_throttle)
	{
		super();
		left_throttle_indicator = 
			new Indicator("L Track",-max_throttle,max_throttle);
		right_throttle_indicator = 
			new Indicator("R Track",-max_throttle,max_throttle);
		timeout = 
			new JSlider(SwingConstants.HORIZONTAL,
					Link.default_timeout,5000,Link.default_timeout);
		packet_loss = new JSlider(SwingConstants.HORIZONTAL,0,100,0);
		frame = new JFrame();
		JPanel panel = new JPanel(new GridLayout(1,2));
		panel.add(left_throttle_indicator);
		panel.add(right_throttle_indicator);
		GridBagLayout g = new GridBagLayout();
		GridBagConstraints c = new GridBagConstraints();
		frame.getContentPane().setLayout(g);
		c.fill = GridBagConstraints.BOTH;
		c.weightx = 100;
		c.weighty = 100;
		g.setConstraints(panel,c);
		frame.getContentPane().add(panel);
		panel = new JPanel(new GridLayout(2,1));
		panel.add(timeout);
		panel.add(packet_loss);
		c.gridy = 1;
		c.weighty = 1;
		g.setConstraints(panel,c);
		frame.getContentPane().add(panel);
		frame.setBounds(100,100,400,100);
		frame.setVisible(true);
	}
	/**
	Set the left throttle indicator
	*/
	public void setLeftThrottle(double value)
	{
		left_throttle_indicator.setValue(value);
		left_throttle_indicator.repaint();
	}
	/**
	Set the right throttle indicator
	*/
	public void setRightThrottle(double value)
	{
		right_throttle_indicator.setValue(value);
		right_throttle_indicator.repaint();
	}

	/**
	 * Get the link timeout setting.
	 */
	public short getTimeOut()
	{
		return (short)(timeout.getValue());
	}

	/**
	 * Get the simulated packet loss setting.
	 */
	public double getLossProb()
	{
		return (double)(packet_loss.getValue())/100.0;
	}
	private Indicator left_throttle_indicator;
	private Indicator right_throttle_indicator;
	private JSlider timeout;
	private JSlider packet_loss;
	private JFrame frame;
}
