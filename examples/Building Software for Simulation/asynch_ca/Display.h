#ifndef Display_h_
#define Display_h_
#include <cmath>
#include <SDL/SDL.h>
#include <string>
#include <exception>

/**
 * Exception class for reporting problems with the display.
 */
class DisplayException:
	public std::exception
{
	public:
		DisplayException(const char* sdl_error, const char* other_info = NULL):
			std::exception()
		{
			if (other_info == NULL) err_msg = sdl_error;
			else err_msg = std::string(other_info) + " : " + sdl_error;
		}
		const char* what() const throw() { return err_msg.c_str(); }
		~DisplayException() throw () {}
	private:
		std::string err_msg;
};

class Display {
	public:
		Display(int xSize, int ySize);
		~Display() { close(); };
		void setStatus();
		void setColor(int x, int y, int r, int g, int b);
		void drawStatus();
		
		static void redraw();
		void checkEvents();
		void toBmp(const char* filename);
		
	private:
		void init(int xSize, int ySize);
		int windowWidth, windowHeight;
		
		void init();
		void close();
		void createColormap();
		int getColor(int r, int g, int b);
		unsigned int xSize, ySize;

		// SDL drawing surface
		static SDL_Surface* sdl_drw;
		// SDL display surface
		static SDL_Surface* sdl_dsp;
		const static int xunit, yunit;
};
		
#endif /*GRAPHER_H_*/
