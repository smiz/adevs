#ifndef _Display_h_
#define _Display_h_
#include <exception>
#include <string>
#include <vector>
#include "SDL/SDL.h"
#include "SDL/SDL_opengl.h"
/**
 * Exception class for reporting problems with the display.
 */
class DisplayException : public std::exception {
  public:
    DisplayException(char const* sdl_error, char const* other_info = NULL)
        : std::exception() {
        if (other_info == NULL) {
            err_msg = sdl_error;
        } else {
            err_msg = std::string(other_info) + " : " + sdl_error;
        }
    }
    char const* what() const throw() { return err_msg.c_str(); }
    ~DisplayException() throw() {}

  private:
    std::string err_msg;
};

/**
 * This is an interface for listening to events from the display.
 */
class DisplayEventListener {
  public:
    /// User requested that the simulation be reset
    virtual void reset() = 0;
    /// User requested that the program exit
    virtual void quit() = 0;
    /// User requested a higher frequency
    virtual void increaseFreq() = 0;
    /// User requested a lower frequency
    virtual void decreaseFreq() = 0;
    /// Destructor
    virtual ~DisplayEventListener() {}
};

/**
 * This class is for rendering the tank on the screen.
 */
class Display {
  public:
    /**
		 * Create the display. Throws a DisplayException if there is a problem
		 * opening the display.
		 */
    Display();
    /**
		 * Register to receive interface events from the display.
		 */
    void addEventListener(DisplayEventListener* l);
    /**
		 * Close the display.
		 */
    ~Display();
    /**
		 * Set the state of the visible tank.
		 */
    void setTankState(double x, double y, double theta);
    /**
		 * Clear the store trajectory
		 */
    void clearStoredTraj();
    /**
		 * Update the display.
		 */
    void draw();
    /**
		 * Poll and process media events.
		 */
    void pollEvents();

  private:
    /**
		 * State variables describing the tank position and orientation.
		 */
    double x, y, theta;
    /**
		 * Trajectory followed by the tank
		 */
    struct coord_t {
        float x, y, theta;
    };
    std::vector<coord_t> traj;
    /**
		 * Clock for timing the runs.
		 */
    unsigned int msecs;
    /**
		 * List of event listeners.
		 */
    std::vector<DisplayEventListener*> listeners;
    /**
		 * Enumeration of event types
		 */
    typedef enum { RESET, QUIT, FREQ_UP, FREQ_DOWN } DisplayEventType;
    /**
		 * Generate a reset() event for every listener
		 */
    void raiseEvent(DisplayEventType event);
    /**
		 * Create the SDL window.
		 */
    void setup();
};

#endif
