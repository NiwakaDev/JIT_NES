#pragma once
#include "common.hpp"
using namespace std;
class Cpu;
class InstructionBase:public Object{
    protected:
        int nbytes;
        int cycles;
    public:
        string name;
        InstructionBase(string name, int nbytes, int cycles);
        virtual int Execute(Cpu* cpu) = 0;
        int GetCycle();
        virtual int CompileStep(uint8_t** code, bool* stop) = 0;
};

class Sei:public InstructionBase{
    public:
        Sei(string name, int nbytes, int cycles);
        int Execute(Cpu* cpu);
        int CompileStep(uint8_t** code, bool* stop);
};