#ifndef Display_h_
#define Display_h_
#include <SDL/SDL.h>
#include <cmath>
#include <exception>
#include <string>

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

class Display {
  public:
    Display(int xSize, int ySize);
    ~Display() { close(); }
    void setStatus();
    void setColor(int x, int y, int r, int g, int b);
    void drawStatus();

    static void redraw();
    void checkEvents();
    void toBmp(char const* filename);

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
    static int const xunit, yunit;
};

#endif /*GRAPHER_H_*/
