#ifndef CommandPacket_h_
#define CommandPacket_h_
#include <cassert>

/**
This class is used to described operator commands to the
tank.
*/
class CommandPacket {
  public:
    /**
		 * Create a command packet from the supplied data.
		 */
    CommandPacket(char const* data) {
        assert(sizeof(int) == sizeof(float));
        assert(sizeof(int) == 4);
        unsigned tmp;
        memcpy(&tmp, data, 4);
        memcpy(&left_throttle, &tmp, 4);
        memcpy(&tmp, data + 4, 4);
        memcpy(&right_throttle, &tmp, 4);
    }
    /// Get the left motor power
    float getLeftThrottle() const { return left_throttle; }
    /// Get the right motor power
    float getRightThrottle() const { return right_throttle; }
    /**
		* Get the number of bytes needed to hold the command data
		*/
    static int getPacketSize() { return 10; }

  private:
    // Left throttle [-1,1]
    float left_throttle;
    // Right throttle [-1,1]
    float right_throttle;
};

#endif
