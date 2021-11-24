#include "Gui.hpp"
#include "common.hpp"

int main(){
    Gui* gui = new Gui();
    SDL_Event e;
    bool quit = false;

    while(!quit){
        while (SDL_PollEvent(&e)){
            if (e.type == SDL_QUIT){
                quit = true;
            }
        }
        gui->Update();
    }
}