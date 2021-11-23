#include "Instruction.hpp"
#include "Cpu.hpp"

//cpuの状態を実機レジスタに対応させる。
//acc:al
//sp:ah
//x:bl
//y:bh
//status:cl
//pc:di
//MOV AL, Imm8
//空いてるレジスタはEDX, ESI

/***
        union{
            uint8_t raw;
            struct{
                unsigned C:1;
                unsigned Z:1;
                unsigned I:1;
                unsigned D:1;
                unsigned B:1;
                unsigned R:1;
                unsigned V:1;
                unsigned N:1;
            }flgs;
        }P;
***/

InstructionBase::InstructionBase(string name, int nbytes, int cycles){
    this->name = name;
    this->nbytes = nbytes;
    this->cycles = cycles;
}

int InstructionBase::GetCycle(){
    return this->cycles;
}

uint8_t InstructionBase::SetRm8(uint8_t mod, uint8_t rm, uint8_t reg_idx){
    uint8_t modrm;
    modrm = (mod<<6) | rm | (reg_idx<<3);
    return modrm;
}

Sei::Sei(string name, int nbytes, int cycles):InstructionBase(name, nbytes, cycles){

}

int Sei::Execute(Cpu* cpu){
    cpu->SetI();
    return this->cycles;
}

int Sei::CompileStep(uint8_t** code, bool* stop, Cpu* cpu){
    *stop = false;
    if(*code!=NULL){
        //status:cl
        //OR CL, 0x04
        **code = 0x80;
        *code++;
        **code = 0xC9;
        *code++;
        **code = (uint8_t)0x04;
        *code++;
        return 3;
    }
    return 3;
}

LdxImmediate::LdxImmediate(string name, int nbytes, int cycles):InstructionBase(name, nbytes, cycles){

}

int LdxImmediate::Execute(Cpu* cpu){
    uint8_t value = cpu->Read8(cpu->GetPc());
    cpu->AddPc(1);
    cpu->Set8(X_KIND, value);
    cpu->UpdateNflg(value);
    cpu->UpdateZflg(value);
    return this->cycles;
}

int LdxImmediate::CompileStep(uint8_t** code, bool* stop, Cpu* cpu){
    uint8_t imm8;
    *stop = false;
    if(*code!=NULL){
        //即値をXレジスタに格納する。
        //NとZフラグの更新も行う
        //x:bl
        imm8 = cpu->Read8(cpu->GetPc());
        cpu->AddPc(1);
        **code = 0xC6;//MOV RM8, IMM8, MOV BL, IMM
        *code++;
        **code = (uint8_t)this->SetRm8(0x03, 0x03, 0x00);
        *code++;
        **code = imm8;
        *code++;
        return 3;
    }
    cpu->AddPc(1);
    return 3;
}

Txs::Txs(string name, int nbytes, int cycles):InstructionBase(name, nbytes, cycles){

}

int Txs::Execute(Cpu* cpu){
    uint8_t x_value = cpu->GetGprValue(X_KIND);
    cpu->Set8(SP_KIND, x_value);
    return this->cycles;
}

int Txs::CompileStep(uint8_t** code, bool* stop, Cpu* cpu){
    *stop = false;
    if(*code!=NULL){
        //sp:ah
        //x:bl
        //xの値をspに保存
        //MOV AH, BL
        **code = 0x8A;//MOV R8, RM8
        *code++;
        **code = this->SetRm8(0x03, 0x03, 0x04);
        *code++;
        return 2;
    }
    return 2;
}

LdaImmediate::LdaImmediate(string name, int nbytes, int cycles):InstructionBase(name, nbytes, cycles){

}

int LdaImmediate::Execute(Cpu* cpu){
    uint8_t value = cpu->Read8(cpu->GetPc());
    cpu->AddPc(1);
    cpu->Set8(A_KIND, value);
    cpu->UpdateNflg(value);
    cpu->UpdateZflg(value);
    return this->cycles;
}

int LdaImmediate::CompileStep(uint8_t** code, bool* stop, Cpu* cpu){
    uint8_t imm8;
    *stop = false;
    if(*code!=NULL){
        //即値をAレジスタに格納
        //Aレジスタ:al
        imm8 = cpu->Read8(cpu->GetPc());
        cpu->AddPc(1);
        **code = 0xC6;//MOV RM8, IMM8, MOV AL, IMM8
        *code++;
        **code = (uint8_t)this->SetRm8(0x03, 0x00, 0x00);
        *code++;
        **code = imm8;
        *code++;
        return 3;
    }
    cpu->AddPc(1);
    return 3;
}