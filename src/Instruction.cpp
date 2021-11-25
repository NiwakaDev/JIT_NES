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

void WriteCall(Cpu* cpu){
    //fprintf(stderr, "cpu->addr = %08X, cpu->data = %08X\n", cpu->addr, cpu->data);
    cpu->Write(cpu->addr, cpu->data);
}

//とりあえず、READもこれを利用
void ReadCall8(Cpu* cpu){
    fprintf(stderr, "cpu->addr = %08X\n", cpu->addr);
    //return cpu->Read8(cpu->addr);
}

/***
void WriteCall(){
    for(int i=0; i<1; i++){
        fprintf(stderr, "Hello\n");
    }
}
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
        *code  = *code + 1;
        **code = 0xC9;
        *code  = *code + 1;
        **code = (uint8_t)0x04;
        *code  = *code + 1;
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
        *code  = *code + 1;
        **code = (uint8_t)this->SetRm8(0x03, 0x03, 0x00);
        *code  = *code + 1;
        **code = imm8;
        *code  = *code + 1;
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
        *code  = *code + 1;
        **code = this->SetRm8(0x03, 0x03, 0x04);
        *code  = *code + 1;
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
        *code  = *code + 1;
        **code = (uint8_t)this->SetRm8(0x03, 0x00, 0x00);
        *code  = *code + 1;
        **code = imm8;
        *code  = *code + 1;
        return 3;
    }
    cpu->AddPc(1);
    return 3;
}

StaAbsolute::StaAbsolute(string name, int nbytes, int cycles):InstructionBase(name, nbytes, cycles){

}

int StaAbsolute::Execute(Cpu* cpu){
    uint8_t a_value = cpu->GetGprValue(A_KIND);
    uint16_t addr;
    addr = cpu->Read16(cpu->GetPc());
    cpu->AddPc(2);
    cpu->Write(addr, a_value);
    return this->cycles;
}

int StaAbsolute::CompileStep(uint8_t** code, bool* stop, Cpu* cpu){
    uint16_t addr = cpu->Read16(cpu->GetPc());
    cpu->addr = addr;
    cpu->AddPc(2);
    *stop = false;
    if(*code!=NULL){
        //メモリにAレジスタの値を格納する
        //Aレジスタ:al
        //MOV EDX, IMM32 (IMM32=buff)
        //LEA ESI, [EDX]+disp32
        //MOV BYTE[ESI], AL

        //cpu->addrにaddrを転送
        //MOV R32, IMM32 (R32=ESI, IMM32=&cpu->data)
        **code    = 0xB8+6;     //mov esi, imm32
        *code  = *code + 1;
        this->Write(&(cpu->addr), code); //imm32=Function

        //MOV rm16, imm16  (rm16=[ESI], im16=addr)
        **code = 0x66;
        *code  = *code + 1;
        **code = 0xC7;
        *code = *code + 1;
        **code = this->SetRm8(0x00, 0x06, 0x00);
        *code = *code + 1;
        this->Write(addr, code);

        //MOV R32, IMM32 (R32=ESI, IMM32=&cpu->data)
        **code    = 0xB8+6;     //mov esi, imm32
        *code  = *code + 1;
        this->Write(&(cpu->data), code); //imm32=&cpu->data

        //MOV rm8, r8  (rm8=[ESI], r8=al)
        **code = 0x88;
        *code = *code + 1;
        **code = this->SetRm8(0x00, 0x06, 0x00);
        *code = *code + 1;

        //書き込みは特別な関数を実行 
        **code    = 0xB8+6;     //mov esi, imm32
        *code  = *code + 1;
        this->Write(WriteCall, code); //imm32=Function

        **code    = 0x83;        //sub rm32, imm8 (rm32=esp, imm8=12)
        *code  = *code + 1;
        **code    = 0xEC;
        *code  = *code + 1;
        **code    = 12;
        *code  = *code + 1;

        **code = 0x60;          //pushad
        *code = *code + 1;

        **code    = 0x68;       //push imm32
        *code  = *code + 1;
        this->Write(cpu, code); //imm32=cpu

        **code    = 0xFF;       //call rm32
        *code  = *code + 1;
        **code    = 0xD6;       //rm32=esi
        *code  = *code + 1;

        //add esp, 4            pushしたcpuを捨てる
        **code    = 0x83;        //add rm32, imm8 (rm32=esp, imm8=12)
        *code  = *code + 1;
        **code    = 0xC4;
        *code  = *code + 1;
        **code    = 4;
        *code  = *code + 1;

        **code = 0x61;          //popad
        *code = *code + 1;

        //add esp, 12            ESPを元の位置に戻す
        **code    = 0x83;        //add rm32, imm8 (rm32=esp, imm8=12)
        *code  = *code + 1;
        **code    = 0xC4;
        *code  = *code + 1;
        **code    = 12;
        *code  = *code + 1;

        return 9+8+10+7+12;
    }
    return 9+8+10+7+12;
}

LdyImmediate::LdyImmediate(string name, int nbytes, int cycles):InstructionBase(name, nbytes, cycles){

}

int LdyImmediate::Execute(Cpu* cpu){
    uint8_t value = cpu->Read8(cpu->GetPc());
    cpu->AddPc(1);
    cpu->Set8(Y_KIND, value);
    cpu->UpdateNflg(value);
    cpu->UpdateZflg(value);
    return this->cycles;
}

int LdyImmediate::CompileStep(uint8_t** code, bool* stop, Cpu* cpu){
    uint8_t imm8 = cpu->Read8(cpu->GetPc());
    cpu->AddPc(1);
    *stop = false;
    if(*code!=NULL){
        //即値をXレジスタに格納する。
        //NとZフラグの更新も行う予定
        //y:bh
        **code = 0xC6;//MOV RM8, IMM8, MOV BL, IMM
        *code  = *code + 1;
        **code = (uint8_t)this->SetRm8(0x03, 0x07, 0x00);
        *code  = *code + 1;
        **code = imm8;
        *code  = *code + 1;
        return 3;
    }
    return 3;
}

LdaAbsoluteX::LdaAbsoluteX(string name, int nbytes, int cycles):InstructionBase(name, nbytes, cycles){

}

int LdaAbsoluteX::Execute(Cpu* cpu){
    uint16_t x_value = (int16_t)((uint8_t)cpu->GetGprValue(X_KIND));
    uint16_t addr;
    uint8_t value;
    int cycle = 0;
    addr = cpu->Read16(cpu->GetPc());
    /***
     * 今回の実装ではこれを無視する
    if((x_value+(addr&0x00FF))>0x000000FF){//dummy read
        cpu->Read8((addr&0xFF00)|((x_value+(addr&0x00FF))&0x00FF));
        cycle = 1;
    }
    ***/
    cpu->AddPc(2);
    addr = addr + x_value;
    value = cpu->Read8(addr);
    cpu->Set8(A_KIND, value);  
    cpu->UpdateNflg(value);
    cpu->UpdateZflg(value); 
    return this->cycles+cycle;
}

int LdaAbsoluteX::CompileStep(uint8_t** code, bool* stop, Cpu* cpu){
    uint16_t addr = cpu->Read16(cpu->GetPc());
    cpu->AddPc(2);
    cpu->addr = addr;
    *stop = false;
    if(*code!=NULL){
        //即値2byteをaddrとx_valueを加算した値が示す番地の中身から、1バイトをAレジスタにロードする。
        //Aレジスタ:al
        //Xレジスタ:bl
        //空いてるレジスタはEDX, ESI
        //Xレジスタの値を2byteに符号無し拡張して、dxに転送
        
        //MOVZX R16, RM8 (R16=DX, RM8=bl)
        **code = 0x66;
        *code  = *code + 1;
        **code = 0x0F;
        *code  = *code + 1;
        **code = 0xB6;
        *code  = *code + 1;
        **code = this->SetRm8(0x03, 0x03, 0x02);
        *code  = *code + 1;

        //MOV R32, IMM32 (R32=ESI, IMM32=&cpu->addr)
        **code = 0xB8+6;
        *code  = *code + 1;
        this->Write(&(cpu->addr), code);

        //MOV rm16, imm16  (rm16=[ESI], im16=addr)
        **code = 0x66;
        *code  = *code + 1;
        **code = 0xC7;
        *code = *code + 1;
        **code = this->SetRm8(0x00, 0x06, 0x00);
        *code = *code + 1;
        this->Write(addr, code);

        //ADD RM16, R16  (RM16=[ESI], DX)
        **code = 0x66;
        *code  = *code + 1;
        **code = 0x01;
        *code  = *code + 1;
        **code = this->SetRm8(0x00, 0x06, 0x02);
        *code  = *code + 1;

        //書き込みは特別な関数を実行 
        **code    = 0xB8+6;     //mov esi, imm32
        *code  = *code + 1;
        this->Write(ReadCall8, code); //imm32=ReadCall8

        **code    = 0x83;        //sub rm32, imm8 (rm32=esp, imm8=12)
        *code  = *code + 1;
        **code    = 0xEC;
        *code  = *code + 1;
        **code    = 12;
        *code  = *code + 1;

        **code = 0x60;          //pushad
        *code = *code + 1;

        **code    = 0x68;       //push imm32
        *code  = *code + 1;
        this->Write(cpu, code); //imm32=cpu

        **code    = 0xFF;       //call rm32
        *code  = *code + 1;
        **code    = 0xD6;       //rm32=esi
        *code  = *code + 1;

        //add esp, 4            
        **code    = 0x83;        //add rm32, imm8 (rm32=esp, imm8=12)
        *code  = *code + 1;
        **code    = 0xC4;
        *code  = *code + 1;
        **code    = 4;
        *code  = *code + 1;

        **code = 0x61;          //popad
        *code = *code + 1;

        //add esp, 4            
        **code    = 0x83;        //add rm32, imm8 (rm32=esp, imm8=12)
        *code  = *code + 1;
        **code    = 0xC4;
        *code  = *code + 1;
        **code    = 12;
        *code  = *code + 1;
        return 33;
    }
    return 33;
}

Inx::Inx(string name, int nbytes, int cycles):InstructionBase(name, nbytes, cycles){

}

int Inx::Execute(Cpu* cpu){
    uint8_t value = cpu->GetGprValue(X_KIND)+1;
    cpu->Set8(X_KIND, value);
    cpu->UpdateNflg(value);
    cpu->UpdateZflg(value); 
    return this->cycles;
}

int Inx::CompileStep(uint8_t** code, bool* stop, Cpu* cpu){
    *stop = false;
    if(*code!=NULL){
        //Xレジスタの値を1プラスする
        //x:bl
        //ADD RM8, 1 (RM8=BL)
        **code = 0x80;
        *code  = *code + 1;
        **code = this->SetRm8(0x03, 0x03, 0x00);
        *code  = *code + 1;
        **code = 0x01;
        *code  = *code + 1;
        return 3;
    }
    return 3;
}

Dey::Dey(string name, int nbytes, int cycles):InstructionBase(name, nbytes, cycles){

}

int Dey::Execute(Cpu* cpu){
    uint8_t value = cpu->GetGprValue(Y_KIND)-1;
    cpu->Set8(Y_KIND, value);
    cpu->UpdateNflg(value);
    cpu->UpdateZflg(value);
    return this->cycles; 
}

int Dey::CompileStep(uint8_t** code, bool* stop, Cpu* cpu){
    *stop = false;
    if(*code!=NULL){
        //Yレジスタの値を1引く
        //y:bh
        //status:cl
        //空いてるレジスタはEDX, ESI

        //SUB RM8, 1 (RM8=bh)
        **code = 0x80;
        *code  = *code + 1;
        **code = this->SetRm8(0x03, 0x07, 0x05);
        *code  = *code + 1;
        **code = 0x01;
        *code  = *code + 1;
    
        //SETZ RM8 (RM8=DL)
        **code = 0x0F;
        *code  = *code + 1;
        **code = 0x94;
        *code  = *code + 1;
        **code = this->SetRm8(0x03, 0x02, 0x00);
        *code  = *code + 1;

        //SETS RM8 (RM8=DH)
        **code = 0x0F;
        *code  = *code + 1;
        **code = 0x98;
        *code  = *code + 1;
        **code = this->SetRm8(0x03, 0x06, 0x00);
        *code  = *code + 1;

        //status:CL
        /***
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
        ***/
        //Z : 1ビット目(0から数えて)
        //N : 7ビット目(0から数えて)

        //DLを左に1bitずらす。
        //SAL RM8 (RM8=DL)
        **code = 0xD0;
        *code  = *code + 1;
        **code = this->SetRm8(0x03, 0x02, 0x04);
        *code  = *code + 1;

        //DHを左に7bitずらす
        //SAL RM8, IMM8 (RM8=DH, IMM8=7)
        **code = 0xC0;
        *code  = *code + 1;
        **code = this->SetRm8(0x03, 0x06, 0x04);
        *code  = *code + 1;
        **code = 0x07;
        *code  = *code + 1;

        //OR RM8, R8 (RM8=CL, R8=DL)
        **code = 0x08;
        *code  = *code + 1;
        **code = this->SetRm8(0x03, 0x01, 0x02);
        *code  = *code + 1;

        //OR RM8, R8 (RM8=CL, R8=DH)
        **code = 0x08;
        *code  = *code + 1;
        **code = this->SetRm8(0x03, 0x01, 0x06);
        *code  = *code + 1;
        return 18;
    }
    return 18;
}

Bne::Bne(string name, int nbytes, int cycles):InstructionBase(name, nbytes, cycles){

}

int Bne::Execute(Cpu* cpu){
    uint16_t value = (int16_t)((int8_t)cpu->Read8(cpu->GetPc()));
    cpu->AddPc(1);
    if(!cpu->IsZflg()){
        cpu->AddPc(value);
        return this->cycles  + 1;
    }
    return this->cycles;
}

int Bne::CompileStep(uint8_t** code, bool* stop, Cpu* cpu){
    uint16_t value = (int16_t)((int8_t)cpu->Read8(cpu->GetPc()));
    cpu->AddPc(1);
    *stop = true;
    if(*code!=NULL){
        //Zフラグ立っている時、即値をpcに代入
        //立っていない時はスルー
        //Zフラグを0bit目に持って行って、
        //その値とvalueの掛け算をとる。
        //Zフラグが1ならばvalueはそのままpcに加算
        //Zフラグが0ならばpcに0が加算されるので、そのままpc
        /***
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
        ***/
        //Zフラグを0bit目に持ってくるには、右に1bitシフト
        //空いてるレジスタはEDX, ESI
        //MOV RM8, R8 (RM8=DL, R8=CL)
        **code = 0x88;
        *code  = *code + 1;
        **code = this->SetRm8(0x03, 0x02, 0x01);
        *code  = *code + 1;

        //AND RM8, IMM8 (RM8=DL, IMM8=0x02)
        **code = 0x80;
        *code  = *code + 1;
        **code = this->SetRm8(0x03, 0x02, 0x04);
        *code  = *code + 1;
        **code = 0x02;
        *code  = *code + 1;

        //DLを右に1bitずらす
        //SHR RM8 (RM8=DL)
        **code = 0xD0;
        *code  = *code + 1;
        **code = this->SetRm8(0x03, 0x02, 0x05);
        *code  = *code + 1;

        //DLを符号無し拡張し、SIに格納する
        //MOVZX R16, RM8 (R16=SI, RM8=DL)
        **code  = 0x66;
        *code   = *code + 1;
        **code  = 0x0F;
        *code   = *code + 1;
        **code  = 0xB6;
        *code   = *code + 1;
        **code  = this->SetRm8(0x03, 0x02, 0x06);
        *code   = *code + 1;

        //DXにvalueを格納する
        **code  = 0x66;
        *code   = *code + 1;
        **code  = 0xB8+2;
        *code   = *code + 1;
        this->Write(value, code);
        
        //DXとSIを掛け算
        //IMUL R16, RM32 (R16=DX, RM16=SI)
        **code = 0x66;
        *code = *code + 1;
        **code = 0x0F;
        *code = *code + 1;
        **code = 0xAF;
        *code = *code + 1;
        **code = this->SetRm8(0x03, 0x06, 0x02);
        *code = *code + 1;

        //DXとPCを加算
        //pc:di
        //ADD RM16, R16 (RM16=DI, R16=DX)
        **code = 0x01;
        *code  = *code + 1;
        **code = this->SetRm8(0x03, 0x07, 0x02);
        *code  = *code + 1;
        return 21;
    }
    return 21;
}