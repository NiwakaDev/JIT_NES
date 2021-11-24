#pragma once
#include "common.hpp"
#include <SDL2/SDL.h>
using namespace std;

class Gui:public Object{
    private:
        bool quit;
        int SCREEN_WIDTH = WIDTH;
        int SCREEN_HEIGHT = HEIGHT;
        SDL_Window* window = NULL;
        SDL_Renderer *renderer = NULL;
        SDL_Texture *texture   = NULL;
        void HandleKeyUp(SDL_Event *e);
        void HandleKeyDown(SDL_Event *e);
        BUTTON_KIND SdlScancode2KeyCode(SDL_Event *e);
        bool button_state[BUTTON_KIND_CNT];
        Pixel* image;
    public:
        Gui();
        void Update();
        void PollEvents();
        bool IsQuit();
        void SetPixel(int x, int y, Pixel pixel);
};