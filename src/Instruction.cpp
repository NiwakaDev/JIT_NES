#include "Instruction.hpp"
#include "Cpu.hpp"

InstructionBase::InstructionBase(string name, int nbytes, int cycles){
    this->name = name;
    this->nbytes = nbytes;
    this->cycles = cycles;
}

int InstructionBase::GetCycle(){
    return this->cycles;
}

Sei::Sei(string name, int nbytes, int cycles):InstructionBase(name, nbytes, cycles){

}

int Sei::Execute(Cpu* cpu){
    cpu->SetI();
    return this->cycles;
}

int Sei::CompileStep(uint8_t** code, bool* stop){
    this->Error("Not implemented: %s::Run", this->name.c_str());
}