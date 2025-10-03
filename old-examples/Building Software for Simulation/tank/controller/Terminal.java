import sdljava.SDLMain;
import sdljava.SDLException;
import sdljava.event.SDLEvent;
import java.net.DatagramPacket;
import sdljava.event.SDLJoyAxisEvent;
import sdljava.event.SDLEventState;
import sdljava.joystick.SDLJoystick;
import sdljava.joystick.SDLJoystick;
import java.net.DatagramSocket;
import java.net.InetAddress;
/**
This class implements the operator terminal interface.
It accepts keyboard and joystick commands, and sends
them to the tank. This class polls for input,
and transmit the requested command at the poll rate.
*/
public class Terminal
	implements Runnable
{
	/**
	 * This is the maximum throttle setting. 	 
	 */
	private static final float MAX_THROTTLE = 1.0f;
	/**
	 * This is the joystick being used to control the tank
	 */
	private SDLJoystick stick;
	/**
	 * Socket for sending data to the tank
	 */
	private DatagramSocket socket;
	private DatagramPacket packet;
	/**
	 * Recyclable command event for the tank
	 */
	private CommandEvent cmd_event;
	private byte [] buffer;
	/**
	 * Driver command display.
	 */
	private DriverDisplay display;

	/**
	 * Constructor. This opens the socket and initializes the
	 * joystick. It requires the tank IP address and joystick number.
	 */
	public Terminal(String tank_addr, int stick_choice)
	{
		// Display for the pilot
		display = new DriverDisplay(MAX_THROTTLE);
		// Driver command structure and message buffer
		cmd_event = new CommandEvent();
		display.setLeftThrottle(cmd_event.left_throttle);
		display.setRightThrottle(cmd_event.right_throttle);
		buffer = CommandEvent.getPackArray();
		// Open the UDP socket
		try
		{
			socket = new DatagramSocket();
			packet = new DatagramPacket(buffer,buffer.length, 
			InetAddress.getByName(tank_addr),Link.tank_port);
		}
		catch(Exception err)
		{
			System.out.println("Could not open socket");
			System.out.println(err.toString());
			System.exit(-1);;
		}
		// Initialize the joystick and event subsystems
		try
		{
			SDLMain.init(SDLMain.SDL_INIT_JOYSTICK | 
					SDLMain.SDL_INIT_VIDEO);
			// Get the selected joystick 
			stick = SDLJoystick.joystickOpen(stick_choice);
			if (stick == null)
			{
				System.out.println("Could not open joystick " + stick_choice);
				System.exit(-1);
			}
			// Enable event handling
			SDLEvent.joystickEventState(SDLEventState.ENABLE);
		}
		catch(Exception err)
		{
			System.out.println("Could not open joystick");
			System.out.println(err.toString());
			System.exit(-1);;
		}
	}

	/**
	 * Thread periodically transmits the command state.
	 */
	public void run()
	{
		while (true)
		{
			sendCommandEvent();
			try
			{
				Thread.sleep(100);
			}
			catch(InterruptedException err)
			{
				System.out.println(err.toString());
			}
		}
	}

	/**
	 * Method for handling joystick events. This method loops
	 * indefinitely.
	 */
	public void handleEvents()
	{
		while (true)
		{
			try
			{
				SDLEvent e = SDLEvent.waitEvent();
				if (e.getType() == SDLEvent.SDL_JOYAXISMOTION)
				{
					handleEvent((SDLJoyAxisEvent)e);
				}
			}
			catch(SDLException err)
			{
				System.out.println("Error in SDL event loop");
				System.out.println(err.toString());
			}
		}
	}

	/**
	 * Process a joystick axis event.
	 */
	private void handleEvent(SDLJoyAxisEvent e)
	{
		sendCommandEvent();
		display.setLeftThrottle(cmd_event.left_throttle);
		display.setRightThrottle(cmd_event.right_throttle);
	}

	/**
	 * Get and send the joystick state. This also updates the 
	 * command event cmd_event.
	 */
	private synchronized void sendCommandEvent()
	{
		// Test for packet loss
		if (Math.random() < display.getLossProb()) return;
		// Get the joystick state
		stick.joystickUpdate();
		/*
		Range of returned values is [-32768,32768]
		so this gives a number in [-327,327]
		*/
		int stick_dir = stick.joystickGetAxis(0)/100;
		int stick_throttle = -stick.joystickGetAxis(2)/100;
		// Convert to [-1,1]
		float throttle = (float)stick_throttle/327.0f;
		float direction = (float)stick_dir/327.0f;
		// Set directional command
		cmd_event.left_throttle = cmd_event.right_throttle = throttle;
		if (direction != 0.0f && throttle == 0.0f)
		{
			cmd_event.left_throttle = direction;
			cmd_event.right_throttle = -direction;
		}
		else if (direction > 0.0f) cmd_event.left_throttle *= (1.0f-direction);
		else if (direction < 0.0f) cmd_event.right_throttle *= (1.0f+direction);
		// Scale the command back to max throttle
		cmd_event.left_throttle = computeThrottle(cmd_event.left_throttle);
		cmd_event.right_throttle = computeThrottle(cmd_event.right_throttle);
		// Set the link timeout field
		cmd_event.link_timeout = display.getTimeOut();
		// Back and send the command
		cmd_event.pack(buffer);
		try
		{
			socket.send(packet);
		}
		catch(Exception err)
		{
			System.out.println(err.toString());
		}
	}

	/**
	 * Utility for enforcing throttle range.
	 */
	private static float computeThrottle(float pos)
	{
		float result = (float)((double)pos*MAX_THROTTLE);
		// Double check the range
		if (result < -MAX_THROTTLE) result = -MAX_THROTTLE;
		if (result > MAX_THROTTLE) result = MAX_THROTTLE;
		return result;
	}

	/**
	 * Main method for Terminal program
	 */
	public static void main(String [] args)
	{
		// User options
		String tank_addr = new String("127.0.0.1");
		int stick_choice = 0;
		// Get command line options
		for (int i = 0; i < args.length; i++)
		{
			if (args[i].equals("-a") && i < args.length-1)
			{
				i++;
				tank_addr = args[i];
			}
			else if (args[i].equals("-s") && i < args.length-1)
			{
				i++;
				try
				{
					stick_choice = Integer.parseInt(args[0]);
				}
				catch(NumberFormatException err)
				{
				}
			}
		}
		System.out.println("Using address " + tank_addr);
		Terminal terminal = new Terminal(tank_addr,stick_choice);
		Thread thread = new Thread(terminal);
		thread.start();
		terminal.handleEvents();
	}

} // end of Terminal
