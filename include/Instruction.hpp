#pragma once
#include "common.hpp"
using namespace std;
class Cpu;
class InstructionBase:public Object{
    protected:
        int nbytes;
        int cycles;
        uint8_t SetRm8(uint8_t mod, uint8_t rm, uint8_t reg_idx);
        template<typename type>void Write(type data, uint8_t** code){
            uint8_t* pointer = (uint8_t*)&data;
            for(int i=0; i<sizeof(data); i++){
                **code = pointer[i];
                *code  = *code + 1;
            }
        }
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

class Txs:public InstructionBase{
    public:
        Txs(string name, int nbytes, int cycles);
        int Execute(Cpu* cpu);
        int CompileStep(uint8_t** code, bool* stop, Cpu* cpu);
};

class LdaImmediate:public InstructionBase{
    public:
        LdaImmediate(string name, int nbytes, int cycles);
        int Execute(Cpu* cpu);
        int CompileStep(uint8_t** code, bool* stop, Cpu* cpu);
};

class StaAbsolute:public InstructionBase{
    public:
        StaAbsolute(string name, int nbytes, int cycles);
        int Execute(Cpu* cpu);
        int CompileStep(uint8_t** code, bool* stop, Cpu* cpu);
};

class LdyImmediate:public InstructionBase{
    public:
        LdyImmediate(string name, int nbytes, int cycles);
        int Execute(Cpu* cpu);
        int CompileStep(uint8_t** code, bool* stop, Cpu* cpu);
};

class LdaAbsoluteX:public InstructionBase{
    public:
        LdaAbsoluteX(string name, int nbytes, int cycles);
        int Execute(Cpu* cpu);
        int CompileStep(uint8_t** code, bool* stop, Cpu* cpu);
};