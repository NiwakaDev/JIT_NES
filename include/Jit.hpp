#pragma once
#include "common.hpp"
using namespace std;
class Cpu;
class InstructionBase;
class Bus;

#define CODE_SIZE     2000
#define MEM_SIZE_6502 0x00010000

#define INSTRUCTION_SIZE 256

class Jit:public Object{
    private:
        uint8_t* code = NULL;
        Cpu* cpu;
        Bus* bus;
        uint8_t* pc2code[MEM_SIZE_6502];
        InstructionBase* instructions[INSTRUCTION_SIZE];
    public:
        Jit(Cpu* cpu, Bus* bus);
        void Run(uint16_t pc);
        void* AllocCodeRegion(int size);
        /***
        template<typename type>void Write(type data){
            uint8_t* pointer = (uint8_t*)&data;
            for(int i=0; i<sizeof(data); i++){
                this->code[this->pc++] = pointer[i];
            }
        }
        ***/
        void Restore(REGISTER_KIND register_kind);
        void ToBinary(const char* file_name);//コード領域をバイナリファイルに保存する
        uint8_t* CompileBlock(uint16_t pc);
        bool IsCompiledBlock(uint16_t pc);
};