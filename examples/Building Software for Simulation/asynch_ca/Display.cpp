#include "Display.h"
#include <iostream>
using namespace std;

const int Display::xunit = 1;
const int Display::yunit = 5;
SDL_Surface* Display::sdl_drw = NULL;
SDL_Surface* Display::sdl_dsp = NULL;

Display::Display(int xSize, int ySize)
{
	windowHeight = ySize*yunit;
	windowWidth = xSize*xunit;
	init();
}

void Display::init() {

	const SDL_VideoInfo* video = NULL;
	// Try to initialize the video subsystem
	if (SDL_Init(SDL_INIT_VIDEO) < 0 ) {
		DisplayException err(SDL_GetError(),"Couldn't initialize SDL");
		throw err;
    }
	// Get the display information
    video = SDL_GetVideoInfo( );
    if(video == NULL) {
		DisplayException err(SDL_GetError(),"Couldn't get video information");
		throw err;
    }
    sdl_dsp = SDL_SetVideoMode(windowWidth, windowHeight,
				video->vfmt->BitsPerPixel, SDL_HWSURFACE | SDL_DOUBLEBUF);
	if (sdl_dsp == NULL) {
		DisplayException err(SDL_GetError(),"Couldn't set video mode");
		throw err;
    }
    sdl_drw = SDL_ConvertSurface(sdl_dsp,sdl_dsp->format,SDL_SWSURFACE);
	if (sdl_drw == NULL) {
		DisplayException err(SDL_GetError(),"Couldn't create drawing surface");
		throw err;
    }
	// Draw the first image
    Display::redraw();
}

void Display::redraw() {
	SDL_BlitSurface(sdl_drw,NULL,sdl_dsp,NULL);
	SDL_Flip(sdl_dsp);
}

void Display::close() {
	SDL_Quit();
}

int Display::getColor(int r, int g, int b) {
	int c;
	c = SDL_MapRGB(sdl_drw->format,r,g,b);
	return c;
}

void Display::setColor(int x, int y, int r, int g, int b) {
	int c;
	SDL_Rect rect;
	c = getColor(r,g,b);
	rect.x = xunit*x;
	rect.y = yunit*y;
	rect.w = xunit;
	rect.h = yunit;
	SDL_FillRect(sdl_drw,&rect,c);
}

void Display::checkEvents() {
	SDL_Event event;
	while (SDL_PollEvent(&event)) {
		switch(event.type) {
			case SDL_VIDEOEXPOSE:
				SDL_LockSurface(sdl_dsp);
				SDL_UpdateRect(sdl_dsp,0,0,windowWidth,windowHeight);
				SDL_UnlockSurface(sdl_dsp);
				break;
			case SDL_QUIT:
				break;
			default:
				break;
		}
	}
}

void Display::toBmp(const char* file)
{
	SDL_SaveBMP(sdl_drw,file);
}
