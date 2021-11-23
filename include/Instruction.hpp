#pragma once
#include "common.hpp"
using namespace std;
class Cpu;
class InstructionBase:public Object{
    protected:
        int nbytes;
        int cycles;
        uint8_t SetRm8(uint8_t mod, uint8_t rm, uint8_t reg_idx);
    public:
        string name;
        InstructionBase(string name, int nbytes, int cycles);
        virtual int Execute(Cpu* cpu) = 0;
        int GetCycle();
        virtual int CompileStep(uint8_t** code, bool* stop, Cpu* cpu) = 0;
};

class Sei:public InstructionBase{
    public:
        Sei(string name, int nbytes, int cycles);
        int Execute(Cpu* cpu);
        int CompileStep(uint8_t** code, bool* stop, Cpu* cpu);
};

class LdxImmediate:public InstructionBase{
    public:
        LdxImmediate(string name, int nbytes, int cycles);
        int Execute(Cpu* cpu);
        int CompileStep(uint8_t** code, bool* stop, Cpu* cpu);
};