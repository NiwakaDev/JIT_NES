#include "Gui.hpp"

Gui::Gui(){
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
    FILE* fp = fopen("hello_image.img", "rb");
    fread(this->image, 1,this->SCREEN_WIDTH*this->SCREEN_HEIGHT*sizeof(Pixel), fp);
    fclose(fp);
}

void Gui::Update(){
  SDL_UpdateTexture(this->texture, NULL, this->image, this->SCREEN_WIDTH * sizeof(Pixel));
  SDL_RenderCopy(this->renderer, this->texture, NULL, NULL);
  SDL_RenderPresent(this->renderer);
}
