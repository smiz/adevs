#include "Display.h"
#include <cassert>
#include <iostream>
#include "GL/glut.h"


// Initial window size
static int const WIDTH = 640;
static int const HEIGHT = 480;

Display::Display() {
    setup();
}

Display::~Display() {
    SDL_Quit();
}

void Display::draw() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    // Draw the path followed by the tank
    glColor3f(0.0f, 0.0f, 1.0f);
    glLoadIdentity();
    glTranslated(0.375f, 2.0f, 0.0f);
    glBegin(GL_LINES);
    for (unsigned i = 1; i < traj.size(); i++) {
        glVertex2f(traj[i - 1].x, traj[i - 1].y);
        glVertex2f(traj[i].x, traj[i].y);
    }
    if (!traj.empty()) {
        glVertex2f(traj.back().x, traj.back().y);
        glVertex2f(x, y);
    }
    glEnd();
    // Draw the tank
    glColor3f(0.0f, 0.0f, 1.0f);
    glLoadIdentity();
    glTranslated(0.375f + x, 2.0f + y, 0.0f);
    glRotated(theta * 180.0 / 3.14, 0.0, 0.0, -1.0);
    glBegin(GL_TRIANGLES);
    glVertex2f(0.0f, 0.1f);
    glVertex2f(-0.1f, -0.1f);
    glVertex2f(0.1f, -0.1f);
    glEnd();
    // Draw the track
    glLoadIdentity();
    glColor3f(0.0f, 0.0f, 0.0f);
    glBegin(GL_LINES);
    // Bottom
    glVertex2f(1.5f, 0.5f);
    glVertex2f(2.5f, 0.5f);
    glVertex2f(1.5f, 0.25f);
    glVertex2f(2.5f, 0.25f);
    // Right lower corner
    glVertex2f(2.5f, 0.5f);
    glVertex2f(3.5f, 1.5f);
    glVertex2f(2.5f, 0.25f);
    glVertex2f(3.75f, 1.5f);
    // Right
    glVertex2f(3.5f, 1.5f);
    glVertex2f(3.5f, 2.5f);
    glVertex2f(3.75f, 1.5f);
    glVertex2f(3.75f, 2.5f);
    // Right upper corner
    glVertex2f(3.5f, 2.5f);
    glVertex2f(2.5f, 3.5f);
    glVertex2f(3.75f, 2.5f);
    glVertex2f(2.5f, 3.75f);
    // Top
    glVertex2f(2.5f, 3.5f);
    glVertex2f(1.5f, 3.5f);
    glVertex2f(2.5f, 3.75f);
    glVertex2f(1.5f, 3.75f);
    // Left upper corner
    glVertex2f(1.5f, 3.5f);
    glVertex2f(0.5f, 2.5f);
    glVertex2f(1.5f, 3.75f);
    glVertex2f(0.25f, 2.5f);
    // Left
    glVertex2f(0.5f, 2.5f);
    glVertex2f(0.5f, 1.5f);
    glVertex2f(0.25f, 2.5f);
    glVertex2f(0.25f, 1.5f);
    // Left lower corner
    glVertex2f(0.5f, 1.5f);
    glVertex2f(1.5f, 0.5f);
    glVertex2f(0.25f, 1.5f);
    glVertex2f(1.5f, 0.25f);
    glEnd();
    // Update the display
    SDL_GL_SwapBuffers();
}

void Display::setTankState(double x, double y, double theta) {
    this->x = x;
    this->y = y;
    this->theta = theta;
    if (!traj.empty() && traj.front().theta == theta) {
        return;
    }
    coord_t coord;
    coord.x = (float)x;
    coord.y = (float)y;
    coord.theta = (float)theta;
    traj.push_back(coord);
}

void Display::clearStoredTraj() {
    traj.clear();
}

void Display::setup() {
    SDL_VideoInfo const* video = NULL;
    // Try to initialize the video subsystem
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) < 0) {
        DisplayException err(SDL_GetError(), "Couldn't initialize SDL");
        throw err;
    }
    // Get the display information
    video = SDL_GetVideoInfo();
    if (!video) {
        DisplayException err(SDL_GetError(), "Couldn't get video information");
        throw err;
    }
    /* Set the minimum requirements for the OpenGL window */
    SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 5);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 5);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 5);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

    /* Note the SDL_DOUBLEBUF flag is not required to enable double 
     * buffering when setting an OpenGL video mode. 
     * Double buffering is enabled or disabled using the 
     * SDL_GL_DOUBLEBUFFER attribute.
     */
    if (SDL_SetVideoMode(WIDTH, HEIGHT, video->vfmt->BitsPerPixel,
                         SDL_OPENGL) == NULL) {
        DisplayException err(SDL_GetError(), "Couldn't set video mode");
        throw err;
    }
    /// Setup OpenGL
    glViewport(0, 0, WIDTH, HEIGHT);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0.0, 4.0, 0.0, 4.0, 1.0, -1.0);
    glMatrixMode(GL_MODELVIEW);
    glClearColor(1.0, 1.0, 1.0, 0);
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
}

void Display::pollEvents() {
    SDL_Event event;
    /// Extract all of the events in the queue
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_KEYDOWN:
                // Reset the timer
                if (event.key.keysym.sym == 't') {
                    std::cout << "time = " << SDL_GetTicks() - msecs << std::endl;
                    msecs = SDL_GetTicks();
                }
                // Reset the simulation
                else if (event.key.keysym.sym == 'r') {
                    raiseEvent(RESET);
                }
                // Increase the frequency
                else if (event.key.keysym.scancode == 86) {
                    raiseEvent(FREQ_UP);
                }
                // Decrease the frequency
                else if (event.key.keysym.scancode == 82) {
                    raiseEvent(FREQ_DOWN);
                }
                // Quit the simulation
                else if (event.key.keysym.sym == 'q') {
                    raiseEvent(QUIT);
                }
                // Take a screenshot
                else if (event.key.keysym.sym == 'c') {
                    FILE* out = fopen("capture.bmp", "w");
                    char pixel_data[3 * WIDTH * HEIGHT];
                    short TGAhead[] = {0, 2, 0, 0, 0, 0, WIDTH, HEIGHT, 24};
                    glReadBuffer(GL_FRONT);
                    glReadPixels(0, 0, WIDTH, HEIGHT, GL_BGR, GL_UNSIGNED_BYTE,
                                 pixel_data);
                    fwrite(&TGAhead, sizeof(TGAhead), 1, out);
                    fwrite(pixel_data, 3 * WIDTH * HEIGHT, 1, out);
                    fclose(out);
                }
                break;
            case SDL_VIDEORESIZE:
                draw();
                break;
            case SDL_VIDEOEXPOSE:
                draw();
                break;
            case SDL_SYSWMEVENT:
                break;
            default:
                break;
        }
    }
}

void Display::raiseEvent(DisplayEventType event) {
    std::vectorDisplayEventListener*>::iterator iter;
    for (iter = listeners.begin(); iter != listeners.end(); iter++) {
        if (event == RESET) {
            (*iter)->reset();
        } else if (event == QUIT) {
            (*iter)->quit();
        } else if (event == FREQ_UP) {
            (*iter)->increaseFreq();
        } else if (event == FREQ_DOWN) {
            (*iter)->decreaseFreq();
        }
    }
}

void Display::addEventListener(DisplayEventListener* l) {
    listeners.push_back(l);
}
