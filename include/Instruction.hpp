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
};
