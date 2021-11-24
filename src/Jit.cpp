#include "Jit.hpp"
#include "Cpu.hpp"
#include "Bus.hpp"
#include "Instruction.hpp"

//cpuの状態を実機レジスタに対応させる。
//acc:al
//sp:ah
//x:bl
//y:bh
//status:cl
//pc:di
//MOV AL, Imm8
//空いてるレジスタはEDX, ESI

Jit::Jit(Cpu* cpu, Bus* bus){
    this->cpu = cpu;
    this->bus = bus;
    for(int i=0; i<MEM_SIZE_6502; i++){
        this->pc2code[i] = NULL;
    }
    for(int i=0; i<INSTRUCTION_SIZE; i++){
        this->instructions[i] = NULL;
    }
    this->instructions[0x78] = new Sei("Sei", 1, 2);
    this->instructions[0x85] = new StaZeropage("StaZeropage", 2, 3);
    this->instructions[0x9A] = new Txs("Txs", 1, 2);
    this->instructions[0xA2] = new LdxImmediate("LdxImmediate", 2, 2);
    this->instructions[0xA9] = new LdaImmediate("LdaImmediate", 2, 2);
}

void* Jit::AllocCodeRegion(int size){
    return mmap(0, size, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);   
}

bool Jit::IsCompiledBlock(uint16_t pc){
    return this->pc2code[pc]!=NULL;
}

uint8_t* Jit::CompileBlock(){
    uint8_t* code = NULL;
    uint8_t* first_loc;
    uint16_t first_pc = this->cpu->GetPc();
    bool stop = false;
    int size = 0;
    while(!stop){
        uint8_t op_code = this->bus->Read8(this->cpu->GetPc());
        this->cpu->AddPc(1);
        if(this->instructions[op_code]==NULL){
            this->Error("Not implemented: 0x%02X at Jit::CompileBlock", op_code);
        }
        size += this->instructions[op_code]->CompileStep(&code, &stop, this->cpu);
    }
    code = (uint8_t*)this->AllocCodeRegion(size+9);
    first_loc = code;
    this->cpu->SetPc(first_pc);
    //再コンパイル
    *code          = 0x60;            //pushad
    code++;
    *code    = 0x83;            //sub esp, 12
    code++;
    *code    = 0xEC;
    code++;
    *code    = 12;
    code++;
    stop = false;
    
    while(!stop){
        uint8_t op_code = this->bus->Read8(this->cpu->GetPc());
        this->cpu->AddPc(1);
        if(this->instructions[op_code]==NULL){
            this->Error("Not implemented: 0x%02X at Jit::CompileBlock", op_code);
        }
        size += this->instructions[op_code]->CompileStep(&code, &stop, this->cpu);
    }
    *code    = 0x83;        //add rm32, imm8 (rm32=esp, imm8=12)
    code++;
    *code    = 0xC4;
    code++;
    *code    = 12;
    code++;
    *code    = 0x61;       //popad
    code++;
    *code    = 0xC3;       //ret
    code++;
    return first_loc;
}

void Jit::Run(){
    uint8_t* code = NULL;
    if(this->IsCompiledBlock(this->cpu->GetPc())){
        code = this->pc2code[this->cpu->GetPc()];
    }else{
        code = this->CompileBlock();
    }
    void (*func)() = (void (*)()) code;
    func();
    exit(1);
}