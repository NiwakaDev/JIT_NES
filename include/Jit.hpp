#pragma once
#include "common.hpp"
using namespace std;
class Cpu;
class InstructionBase;
class Bus;

#define CODE_SIZE     2000
#define MEM_SIZE_6502 0x00010000

#define INSTRUCTION_SIZE 256

struct CompileBlockInfo{
    uint8_t* code;
    int total_cycles;
};

class Jit:public Object{
    private:
        uint8_t* code = NULL;
        Cpu* cpu;
        Bus* bus;
        uint8_t* pc2code[MEM_SIZE_6502];
        InstructionBase* instructions[INSTRUCTION_SIZE];
        void Restore(REGISTER_KIND register_kind, uint8_t** code);      
        uint8_t SetRm8(uint8_t mod, uint8_t rm, uint8_t reg_idx);
    public:
        Jit(Cpu* cpu, Bus* bus);
        int Run();
        void* AllocCodeRegion(int size);
        template<typename type>void Write(type data, uint8_t** code){
            uint8_t* pointer = (uint8_t*)&data;
            for(int i=0; i<sizeof(data); i++){
                **code = pointer[i];
                *code  = *code + 1;
            }
        }
        void Restore(REGISTER_KIND register_kind);
        void ToBinary(const char* file_name, uint16_t pc, int size);//コード領域をバイナリファイルに保存する
        uint8_t* CompileBlock();
        bool IsCompiledBlock(uint16_t pc);
};