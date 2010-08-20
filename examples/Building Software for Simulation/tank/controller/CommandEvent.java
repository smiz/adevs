/**
This class is used to described operator commands to the
tank.
*/
public class CommandEvent
{
	// Left throttle [-1,1]
	public float left_throttle;
	// Right throttle [-1,1]
	public float right_throttle;
	// Link drop timeout
	public short link_timeout;

	public CommandEvent()
	{
		super();
		link_timeout = Link.default_timeout;
		left_throttle = 0.0f;
		right_throttle = 0.0f;
	}
	/**
	Set the value of this object from a buffer.
	*/
	public void pack(byte [] array)
	{
		writeFloat(left_throttle,array,0);
		writeFloat(right_throttle,array,4);
		writeShort(link_timeout,array,8);
	}
	/**
	Get a buffer large enough to hold a packed version of the command.
	*/
	public static byte [] getPackArray()
	{
		return new byte[10];
	}
	/**
	Pack the object into a byte array.
	*/
	public void unpack(byte[] array)
	{
		left_throttle = readFloat(array,0);
		right_throttle = readFloat(array,4);
		link_timeout = readShort(array,8);
	}

	private void writeFloat(float f, byte [] array, int offset)
	{
		int bits = Float.floatToIntBits(f);
		array[offset] = (byte)bits;
		array[offset+1] = (byte)(bits >>> 8);
		array[offset+2] = (byte)(bits >>> 16);
		array[offset+3] = (byte)(bits >>> 24);
	}

	private void writeShort(short s, byte [] array, int offset)
	{
		array[offset] = (byte)s;
		array[offset+1] = (byte)(s >> 8);
	}

	private float readFloat(byte [] array, int offset)
	{
		int bits = 
			((int)array[offset] & 0x000000ff) |
			((((int)array[offset+1])<<8) & 0x0000ff00) |
			((((int)array[offset+2])<<16) & 0x00ff0000) |
			((((int)array[offset+3])<<24) & 0xff000000);
		return Float.intBitsToFloat(bits);
	}

	private short readShort(byte [] array, int offset)
	{
		int bits = 
			((int)array[offset] & 0x000000ff) |
			((((int)array[offset+1])<<8) & 0x0000ff00); 
		return (short)bits; 
	}

	/* The tini convertor allows only one main */
	/**	
	public static void main(String args[])
	{
		byte[] buffer = CommandEvent.getPackArray();
		CommandEvent e = new CommandEvent();
		e.link_timeout = 10;
		e.left_throttle = 0.99f;
		e.right_throttle = 0.876f;
		CommandEvent f = new CommandEvent();
		System.out.println(e.left_throttle + " " + f.left_throttle);
		System.out.println(e.right_throttle + " " + f.right_throttle);
		System.out.println(e.link_timeout + " " + f.link_timeout);
		e.pack(buffer);
		f.unpack(buffer);
		System.out.println(e.left_throttle + " " + f.left_throttle);
		System.out.println(e.right_throttle + " " + f.right_throttle);
		System.out.println(e.link_timeout + " " + f.link_timeout);
		if (e.left_throttle != f.left_throttle)
			System.out.println("left throttle fail");
		if (e.right_throttle != f.right_throttle)
			System.out.println("right throttle fail");
		if (e.link_timeout != f.link_timeout)
			System.out.println("timeout fail");
	}
	*/
}
