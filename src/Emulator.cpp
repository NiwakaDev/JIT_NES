#include "Dma.hpp"
#include "Ppu.hpp"
#include "Cpu.hpp"
#include "InesParser.hpp"
#include "Memory.hpp"
#include "Bus.hpp"
#include "InterruptManager.hpp"
#include "JoyPad.hpp"
#include "Mapper.hpp"
#include "Mapper0.hpp"
#include "Mapper3.hpp"
#include "Jit.hpp"
#include "Emulator.hpp"

Emulator::Emulator(int argc, char** argv){
    this->joy_pad = new JoyPad();
    assert(this->joy_pad!=NULL);
    this->ines_parser = new InesParser(argv[1]);
    //this->ines_parser = new InesParser("/Users/mori/Desktop/Super Mario Bros. (World).nes");
    assert(this->ines_parser!=NULL);
    switch(this->ines_parser->GetMapperNumber()){
        case 0:
            this->mapper = new Mapper0(this->ines_parser);
            break;
        case 3:
            this->mapper = new Mapper3(this->ines_parser);
            break;
        default:
            this->Error("Not implemented:  mapper_number = %d at Emulator::Emulator", this->ines_parser->GetMapperNumber());
    }
    assert(this->mapper!=NULL);
    this->interrupt_manager = new InterruptManager();
    assert(this->interrupt_manager!=NULL);
    this->memory = new Memory(this->ines_parser);
    assert(this->memory!=NULL);
    this->dma = new Dma(this->memory);
    assert(this->dma!=NULL);
    this->ppu = new Ppu(this->ines_parser, this->mapper);
    assert(this->ppu!=NULL);
    this->bus = new Bus(this->memory, this->ppu, this->joy_pad, this->dma, this->ines_parser, this->mapper);
    assert(this->bus!=NULL);
    this->cpu = new Cpu(this->bus);
    assert(this->cpu!=NULL);
    this->jit = new Jit(this->cpu, this->bus);
    assert(this->jit!=NULL);
}

void Emulator::Execute(){
    int cycle;
    bool flg = false;
    uint32_t start;
    uint32_t end;
    uint32_t offset;
    const uint32_t DELAY = 16;
    this->cpu->Reset();
    //while(!flg){
    for(int i=0; i<3; i++){
        cycle = 0;
        //JITコンパイラでは、割り込み処理を未実装
        if(this->dma->IsRunning()){
            this->dma->Execute(this->ppu);
            cycle = 512;
        }
        this->jit->Run();
        //cycle += this->cpu->Execute();
        flg = this->ppu->Execute(cycle*3, this->interrupt_manager);
    }
    flg = false;
}