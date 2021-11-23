#pragma once
#include "common.hpp"

class Gui;
class Ppu;
class Dma;
class Bus;
class Memory;
class InesParser;
class InterruptManager;
class JoyPad;
class Mapper;
class Cpu;

class Emulator:public Object{
    private:
        JoyPad* joy_pad = NULL;
        InesParser* ines_parser = NULL;
        Cpu* cpu = NULL;
        Memory* memory = NULL;
        Dma* dma = NULL;
        Ppu* ppu = NULL;
        Bus* bus = NULL;
        Gui* gui = NULL;
        Mapper* mapper = NULL;
        InterruptManager *interrupt_manager = NULL;
        thread* emu_thread = NULL;
        int now_cycle = 0;
    public:
        Emulator(int argc, char** argv);
        void Execute();
};
