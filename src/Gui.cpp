#include "JoyPad.hpp"
#include "Gui.hpp"

Gui::Gui(JoyPad* joy_pad){
    this->joy_pad = joy_pad;
    assert(joy_pad!=NULL);
    if( SDL_Init( SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0 ){
        cout << SDL_GetError() << endl;
        this->Error("at Gui::Gui");
    }
    this->window = SDL_CreateWindow("EMULATOR", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, this->SCREEN_WIDTH*2, this->SCREEN_HEIGHT*2, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
    if(this->window==NULL){
        cout << SDL_GetError() << endl;
        this->Error("at Gui::Gui");
    }
    this->renderer      = SDL_CreateRenderer(this->window, -1, SDL_RENDERER_ACCELERATED|SDL_RENDERER_PRESENTVSYNC);
    //this->texture       = SDL_CreateTexture(this->renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, this->SCREEN_WIDTH, this->SCREEN_HEIGHT); 
    this->texture       = SDL_CreateTexture(this->renderer, SDL_PIXELFORMAT_BGRA8888, SDL_TEXTUREACCESS_STREAMING, this->SCREEN_WIDTH, this->SCREEN_HEIGHT); 
    if(SDL_RenderSetLogicalSize(this->renderer, WIDTH, HEIGHT)<0){
        cout << SDL_GetError() << endl;
        this->Error("at Gui::Gui");
    }
    for(int i=0; i<BUTTON_KIND_CNT; i++){
        this->button_state[i] = false;
    }
    this->quit  = false;
    this->image = (Pixel*)malloc(this->SCREEN_WIDTH*this->SCREEN_HEIGHT*sizeof(Pixel));
}

void Gui::Update(){
  SDL_UpdateTexture(this->texture, NULL, this->image, this->SCREEN_WIDTH * sizeof(Pixel));
  SDL_RenderCopy(this->renderer, this->texture, NULL, NULL);
  SDL_RenderPresent(this->renderer);
}

bool Gui::IsQuit(){
    return this->quit;
}

void Gui::PollEvents(){
    SDL_Event e;
    while (SDL_PollEvent(&e)){
        if (e.type == SDL_QUIT){
            this->quit = true;
        }
        if(e.type==SDL_KEYDOWN){
            this->HandleKeyDown(&e);
            break;
        }
        if(e.type==SDL_KEYUP){
            this->HandleKeyUp(&e);
            break;
        }
    }
    this->joy_pad->UpdateButtonState(this->button_state);
}

void Gui::SetPixel(int x, int y, Pixel pixel){
    this->image[x+y*this->SCREEN_WIDTH] = pixel;
}

BUTTON_KIND Gui::SdlScancode2KeyCode(SDL_Event *e){
    switch (e->key.keysym.sym){
        case SDLK_a:
            return BUTTON_A_KIND;
        case SDLK_b:
            return BUTTON_B_KIND;
        case SDLK_SPACE:
            return BUTTON_SELECT_KIND;
        case SDLK_RETURN:
            return BUTTON_START_KIND;
        case SDLK_UP:
            return BUTTON_UP_KIND;
        case SDLK_DOWN:
            return BUTTON_DOWN_KIND;
        case SDLK_LEFT:
            return BUTTON_LEFT_KIND;
        case SDLK_RIGHT:
            return BUTTON_RIGHT_KIND;
        default:
            return NOT_BUTTON_KIND;
    }
}

void Gui::HandleKeyUp(SDL_Event *e){
    BUTTON_KIND button_kind;
    button_kind = this->SdlScancode2KeyCode(e);
    if(button_kind==NOT_BUTTON_KIND){
        return;
    }
    this->button_state[button_kind] = false;
}

void Gui::HandleKeyDown(SDL_Event *e){
    BUTTON_KIND button_kind;
    button_kind = this->SdlScancode2KeyCode(e);
    if(button_kind==NOT_BUTTON_KIND){
        return;
    }
    this->button_state[button_kind] = true;
}