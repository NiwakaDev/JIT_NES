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
    this->instructions[0x88] = new Dey("Dey", 1, 2);
    this->instructions[0x8D] = new StaAbsolute("StaAbsolute", 3, 4);
    this->instructions[0x9A] = new Txs("Txs", 1, 2);
    this->instructions[0xA0] = new LdyImmediate("LdyImmediate", 2, 2);
    this->instructions[0xA2] = new LdxImmediate("LdxImmediate", 2, 2);
    this->instructions[0xA9] = new LdaImmediate("LdaImmediate", 2, 2);
    this->instructions[0xBD] = new LdaAbsoluteX("LdaAbsoluteX", 3, 4);
    this->instructions[0xD0] = new Bne("Bne", 2, 2);
    this->instructions[0xE8] = new Inx("Inx", 1, 2);
    /***
    this->instructions[0x85] = new StaZeropage("StaZeropage", 2, 3);
    ***/
}

void* Jit::AllocCodeRegion(int size){
    return mmap(0, size, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);   
}

bool Jit::IsCompiledBlock(uint16_t pc){
    return this->pc2code[pc]!=NULL;
}

void Jit::Restore(REGISTER_KIND register_kind, uint8_t** code){
    switch (register_kind){
        case A_KIND:
            **code = (uint8_t)(0xB8+2);//MOV EDX, IMM32
            *code  = *code + 1;
            this->Write((uint32_t)0, code);
            **code = 0x8D;//LEA R32, M
            *code  = *code + 1;
            **code = 0xB2;   // reg : ESI, effective_addr:[EDX]+disp32
            *code  = *code + 1;
            this->Write(&(this->cpu->gprs[A_KIND]), code);
            **code = 0x88; //MOV RM8, R8     MOV BYTE[ESI], AL
            *code  = *code + 1;
            **code = 0x06; //reg : AL, effective_addr:[ESI]
            *code  = *code + 1;
            break;
        case X_KIND:
            **code = (uint8_t)(0xB8+2);//MOV EDX, IMM32
            *code  = *code + 1;
            this->Write((uint32_t)0, code);
            **code = 0x8D;//LEA R32, M
            *code  = *code + 1;
            **code = 0xB2;   // reg : ESI, effective_addr:[EDX]+disp32
            *code  = *code + 1;
            this->Write(&(this->cpu->gprs[X_KIND]), code);
            **code = 0x88; //MOV RM8, R8     MOV BYTE[ESI], BL
            *code  = *code + 1;
            **code = 0x1E; //reg : BL, effective_addr:[ESI]
            *code  = *code + 1;
            break;
        case Y_KIND:
            **code = (uint8_t)(0xB8+2);//MOV EDX, IMM32
            *code  = *code + 1;
            this->Write((uint32_t)0, code);
            **code = 0x8D;//LEA R32, M
            *code  = *code + 1;
            **code = 0xB2;   // reg : ESI, effective_addr:[EDX]+disp32
            *code  = *code + 1;
            this->Write(&(this->cpu->gprs[Y_KIND]), code);
            **code = 0x88; //MOV RM8, R8     MOV BYTE[ESI], BH
            *code  = *code + 1;
            **code = 0x3E; //reg : BH, effective_addr:[ESI]
            *code  = *code + 1;
            break;
        case SP_KIND:
            **code = (uint8_t)(0xB8+2);//MOV EDX, IMM32
            *code  = *code + 1;
            this->Write((uint32_t)0, code);
            **code = 0x8D;//LEA R32, M
            *code  = *code + 1;
            **code = 0xB2;   // reg : ESI, effective_addr:[EDX]+disp32
            *code  = *code + 1;
            this->Write(&(this->cpu->gprs[SP_KIND]), code);
            **code = 0x88; //MOV RM8, R8     MOV BYTE[ESI], AH
            *code  = *code + 1;
            **code = 0x26; //reg : AH, effective_addr:[ESI]
            *code  = *code + 1;
            break;
        case PC_KIND:
            **code = (uint8_t)(0xB8+2);//MOV EDX, IMM32
            *code  = *code + 1;
            this->Write((uint32_t)0, code);
            **code = 0x8D;//LEA R32, M
            *code  = *code + 1;
            **code = 0xB2;   // reg : ESI, effective_addr:[EDX]+disp32
            *code  = *code + 1;
            this->Write(&(this->cpu->pc), code);
            **code = 0x66; //オペランドオーバーライド
            *code  = *code + 1;
            **code = 0x89; //MOV RM16, R16     MOV WORD[ESI], DI
            *code  = *code + 1;
            **code = 0x3E; //reg : DI, effective_addr:[ESI]
            *code  = *code + 1;
            break;
        default:
            fprintf(stderr, "Not implemented: %d\n", register_kind);
            exit(1);
    }
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
    code = (uint8_t*)this->AllocCodeRegion(size+10000);
    this->pc2code[first_pc] = code;
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

    *code = 0xB0;//MOV R8, IMM8
    code++;
    *code = this->cpu->GetGprValue(A_KIND);
    code++;
    *code = 0xB0+4;//MOV R8, IMM8
    code++;
    *code = this->cpu->GetGprValue(SP_KIND);
    code++;
    *code = 0xB0+3;//MOV R8, IMM8
    code++;
    *code = this->cpu->GetGprValue(X_KIND);
    code++;
    *code = 0xB0+7;//MOV R8, IMM8
    code++;
    *code = this->cpu->GetGprValue(Y_KIND);
    code++;
    *code = 0xB0+1;//MOV R8, IMM8
    code++;
    *code = this->cpu->GetP();
    code++;
    *code = 0x66;//MOV R16, IMM16
    code++;
    *code = 0xB8+7;//MOV R16, IMM16
    code++;
    this->Write(this->cpu->GetPc(), &code);

    while(!stop){
        uint8_t op_code = this->bus->Read8(this->cpu->GetPc());
        this->cpu->AddPc(1);
        if(this->instructions[op_code]==NULL){
            this->Error("Not implemented: 0x%02X at Jit::CompileBlock", op_code);
        }
        size += this->instructions[op_code]->CompileStep(&code, &stop, this->cpu);
    }
    //レジスタの値を復元
    this->Restore(A_KIND, &code);
    this->Restore(X_KIND, &code);
    this->Restore(Y_KIND, &code);
    this->Restore(SP_KIND, &code);
    this->Restore(PC_KIND, &code);
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
    this->cpu->ShowSelf();
}