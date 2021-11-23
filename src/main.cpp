#include "Emulator.hpp"

int main(int argc, char** argv){
    if(argc<2){
        fprintf(stderr, "No input file\n");
        exit(EXIT_FAILURE);
    }

    Emulator* emu = new Emulator(argc, argv);
    emu->Execute();
    return 0;
}