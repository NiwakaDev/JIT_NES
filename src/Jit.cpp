#include "Jit.hpp"
#include "Cpu.hpp"
#include "Bus.hpp"

Jit::Jit(Cpu* cpu, Bus* bus){
    this->cpu = cpu;
    this->bus = bus;
    for(int i=0; i<MEM_SIZE_6502; i++){
        this->pc2code[i] = NULL;
    }
    for(int i=0; i<INSTRUCTION_SIZE; i++){
        this->instructions[i] = NULL;
    }
}

void* Jit::AllocCodeRegion(int size){
    return mmap(0, size, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);   
}

bool Jit::IsCompiledBlock(uint16_t pc){
    return this->pc2code[pc]!=NULL;
}

uint8_t* Jit::CompileBlock(uint16_t pc){
    uint8_t* code;
    uint8_t op_code = this->bus->Read8(pc);
    //機械語命令を1つコンパイル
    if(this->instructions[op_code]==NULL){
        this->Error("Not implemented: %02X at Jit::CompileBlock", op_code);
    }
    return NULL;
}

void Jit::Run(uint16_t pc){
    uint8_t* code;
    if(this->IsCompiledBlock(pc)){
        code = this->pc2code[pc];
    }else{
        code = this->CompileBlock(pc);
    }
    void (*func)() = (void (*)()) code;
    func();
}