#include "Instruction.hpp"
#include "InterruptManager.hpp"
#include "JoyPad.hpp"
#include "common.hpp"
#include "InesParser.hpp"
#include "Memory.hpp"
#include "Ppu.hpp"
#include "Cpu.hpp"
#define SIGN_FLG 0x80
#define STACK_BASE_ADDR 0x0100

Cpu::Cpu(Bus* bus){
    this->bus = bus;
    assert(this->bus!=NULL);
    this->pc = 0x8000;

    for(int i=0; i<this->instruction_size; i++){
        this->instructions[i] = NULL;
    }

    this->P.raw = 0x24;
    this->gprs[SP_KIND] = 0xFD;
    this->gprs[A_KIND] = 0x00;
    this->gprs[X_KIND] = 0x00;
    this->gprs[Y_KIND] = 0x00;
}

int Cpu::Execute(){
    unsigned char op_code;
    static int idx = 1;
    op_code = this->bus->Read8(this->pc);
    int cycles;
    if(this->instructions[op_code]==NULL){
        this->ShowSelf();
        this->Error("Not implemented: op_code=%02X", op_code);
    }
    this->pc++;
    idx++;
    return this->instructions[op_code]->Execute(this);
}

void Cpu::SetI(){
    this->P.flgs.I = 1;
}

void Cpu::SetN(){
    this->P.flgs.N = 1;
}

void Cpu::SetZ(){
    this->P.flgs.Z = 1;
}

void Cpu::SetC(){
    this->P.flgs.C = 1;
}

void Cpu::SetV(){
    this->P.flgs.V = 1;
}

void Cpu::SetB(){
    this->P.flgs.B = 1;
}

void Cpu::ClearI(){
    this->P.flgs.I = 0;
}

void Cpu::ClearN(){
    this->P.flgs.N = 0;
}

void Cpu::ClearZ(){
    this->P.flgs.Z = 0;
}

void Cpu::ClearC(){
    this->P.flgs.C = 0;
}

void Cpu::ClearV(){
    this->P.flgs.V = 0;
}

uint8_t Cpu::GetGprValue(REGISTER_KIND register_kind){
    return this->gprs[register_kind];
}

void Cpu::AddPc(uint16_t value){
    this->pc = this->pc + value;
}

void Cpu::SetPc(uint16_t value){
    this->pc = value;
}

void Cpu::Write8(uint16_t addr, uint8_t value){
    this->bus->Write8(addr, value);
}

uint8_t Cpu::Read8(uint16_t addr){
    return this->bus->Read8(addr);
}

uint16_t Cpu::Read16(uint16_t addr){
    uint16_t value;
    value = ((uint16_t)this->bus->Read8(addr)) | (((uint16_t)this->bus->Read8(addr+1))<<8);
    return value;
}

uint16_t Cpu::GetPc(){
    return this->pc;
}

void Cpu::Set8(REGISTER_KIND register_kind, uint8_t value){
    this->gprs[register_kind] = value;
}

void Cpu::ShowSelf(){
    fprintf(stderr, "A_REGISTER  : %02X\n", this->gprs[A_KIND]);
    fprintf(stderr, "X_REGISTER  : %02X\n", this->gprs[X_KIND]);
    fprintf(stderr, "Y_REGISTER  : %02X\n", this->gprs[Y_KIND]);
    fprintf(stderr, "S_REGISTER  : %02X\n", this->gprs[SP_KIND]);
    fprintf(stderr, "P_REGISTER  : %02X\n", this->P.raw);
    fprintf(stderr, "PC_REGISTER : %04X\n", this->pc);
}

void Cpu::UpdateNflg(uint8_t value){
    if((value&SIGN_FLG)==SIGN_FLG){
        this->P.flgs.N = 1;
        return;
    }
    this->P.flgs.N = 0;
}

void Cpu::UpdateZflg(uint8_t value){
    if(value==0x00){
        this->P.flgs.Z = 1;
        return;
    }
    this->P.flgs.Z = 0;
}

bool Cpu::IsNflg(){
    return this->P.flgs.N;
}

bool Cpu::IsZflg(){
    return this->P.flgs.Z;
}

bool Cpu::IsCflg(){
    return this->P.flgs.C;
}

bool Cpu::IsVflg(){
    return this->P.flgs.V;
}

uint8_t Cpu::GetCFLg(){
    return this->P.flgs.C;
}

void Cpu::Push8(uint8_t data){
    this->Write(((uint16_t)this->gprs[SP_KIND])|STACK_BASE_ADDR, data);
    this->gprs[SP_KIND] -= 1;
}

void Cpu::Push16(uint16_t data){
    uint8_t *p = (uint8_t*)&data;
    //this->Write((((uint16_t)this->gprs[SP_KIND])|STACK_BASE_ADDR), *(p+1));//upper byte
    //this->Write((((uint16_t)this->gprs[SP_KIND])|STACK_BASE_ADDR)-1, *p);//lower byte
    this->Write((((uint16_t)this->gprs[SP_KIND])|STACK_BASE_ADDR)-1, data);
    this->gprs[SP_KIND] -= 2;

    /***
    for(int i=0; i<sizeof(data); i++){
        this->Write((((uint16_t)this->gprs[SP_KIND])|STACK_BASE_ADDR), p[i]);
        this->gprs[SP_KIND]--;
    }
    ***/
}

uint8_t Cpu::Pop8(){
    uint8_t data;
    this->gprs[SP_KIND]++;
    return this->Read8(((uint16_t)this->gprs[SP_KIND])|STACK_BASE_ADDR);
}

uint16_t Cpu::Pop16(){
    uint16_t data;
    uint8_t* p = (uint8_t*)&data;

    *p = this->Read8((((uint16_t)this->gprs[SP_KIND])|STACK_BASE_ADDR)+1);//lowwer byte
    *(p+1) = this->Read8((((uint16_t)this->gprs[SP_KIND])|STACK_BASE_ADDR)+2);//upper byte
    this->gprs[SP_KIND] += 2;

    /***
    for(int i=0; i<sizeof(data); i++){
        this->gprs[SP_KIND]++;
        *(p+sizeof(data)-1-i) = this->Read8((((uint16_t)this->gprs[SP_KIND])|STACK_BASE_ADDR));
    }
    ***/
    return data;
}

void Cpu::HandleNmi(InterruptManager* interrupt_manager){
    uint16_t pc;
    this->P.flgs.B = 0;
    this->P.flgs.I = 1;
    this->Push16(this->GetPc());
    this->Push8(this->P.raw);
    this->SetPc(this->Read16(0xFFFA));
    interrupt_manager->ClearNmi();
}

void Cpu::SetP(uint8_t value){
    this->P.raw = value;
}

void Cpu::SetD(){
    this->P.flgs.D = 1;
}

void Cpu::ClearD(){
    this->P.flgs.D = 0;
}

uint8_t Cpu::GetP(){
    return this->P.raw;
}

bool Cpu::CmpLastInstructionName(string name){
    if(this->instruction_log.size()<=1){
        return false;
    }
    return this->instruction_log[this->instruction_log.size()-2]==name;
}

void Cpu::Reset(){
    this->pc = this->Read16(0xFFFC);
    if(this->pc==0){
        this->pc = 0x8000;
    }
}