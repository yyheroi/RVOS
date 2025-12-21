
#include <iostream>
#include <format>
#include <string>
#include "Core/Instruction.hh"
#include "Util/Regs.hh"

// Implementation of IBaseTypeInst::SetBitField
int IBaseTypeInst::SetBitField()
{
    bitField_= inst_.lo_;
    std::cout << "Binary: " << bitField_ << '\n';
    return 0;
}

// RType

void RType::Decode()
{
    std::cout << "R-Type Instruction\n";
    std::cout << "opcode: 0x" << std::hex << GetOpcode() << '\n';
    std::cout << "funct3: " << inst_.R.fct3 << '\n';
    std::cout << "funct7: " << inst_.R.fct7 << '\n';
    std::cout << "rs1: " << inst_.R.rs1 << '\n';
    std::cout << "rs2: " << inst_.R.rs2 << '\n';
    std::cout << "rd: " << inst_.R.rd << '\n';
    rTypeField_ = inst_.R;
}


std::string format_hex(uint32_t raw) {
    return std::format("0x{:08X}", raw);
}
std::string RType::ToAssemblyString() {
    insStrVector_.clear();
    insStrVector_.emplace_back(mnemonicStrIns_);
    insStrVector_.emplace_back(strRd_);
    insStrVector_.emplace_back(strRs1_);
    insStrVector_.emplace_back(strRs2_);

    return std::format("{:<6} {:<4}, {:<4}, {:<4}",
        mnemonicStrIns_,
        strRd_,
        strRs1_,
        strRs2_);
}
void RType::ProcessFunct()
{
    uint16_t functKey= (inst_.R.fct7 << 3) | inst_.R.fct3;
    std::cout << "functKey: " << functKey << std::endl;
    std::cout << "inst_.R.fct7: " << inst_.R.fct7 << std::endl;
    std::cout << "inst_.R.fct3: " << inst_.R.fct3 << std::endl;
    auto it          = functMnemonicRTypeMap_.find(functKey);
   
    if(it != functMnemonicRTypeMap_.end()) {
        std::cout << "Mnemonic: " << it->second << ' ' << G_INDEX_REGS_ABI[inst_.R.rd].first << ", "
                  << G_INDEX_REGS_ABI[inst_.R.rs1].first << ", "
                  << G_INDEX_REGS_ABI[inst_.R.rs2].first << '\n';
        
        mnemonicStrIns_ = it->second;
        strRd_ = G_INDEX_REGS_ABI[inst_.R.rd].first;
        strRs1_ = G_INDEX_REGS_ABI[inst_.R.rs1].first;
        strRs2_ = G_INDEX_REGS_ABI[inst_.R.rs2].first;
        insHexStr_ = format_hex(inst_.lo_);
        mnemonicStrRd_ = G_INDEX_REGS_ABI[inst_.R.rd].second;
        mnemonicStrRs1_ = G_INDEX_REGS_ABI[inst_.R.rs1].second;
        mnemonicStrRs2_ = G_INDEX_REGS_ABI[inst_.R.rs2].second;
    } else {
        std::cout << "Unsupported R-type funct combination: funct7=" << inst_.R.fct7
                  << ", funct3=" << inst_.R.fct3 << '\n';
    }
}

std::string IType::ToAssemblyString() { return "NULL";}
void IType::Decode()
{
    std::cout << "I-Type Instruction\n";
    std::cout << "opcode: " << GetOpcode() << '\n';
    std::cout << "funct3: " << inst_.I.fct3 << '\n';
    std::cout << "rs1: " << inst_.I.rs1 << '\n';
    std::cout << "rd: " << inst_.I.rd << '\n';
    std::cout << "imm12: " << inst_.I.imm0tB << '\n';
}

void IType::ProcessFunct() { }

std::string SType::ToAssemblyString() { return "NULL"; }
void SType::Decode()
{
    std::cout << "S-Type Instruction\n";
    std::cout << "opcode: " << GetOpcode() << '\n';
    std::cout << "funct3: " << inst_.S.fct3 << '\n';
    std::cout << "rs1: " << inst_.S.rs1 << '\n';
    std::cout << "rs2: " << inst_.S.rs2 << '\n';
    uint16_t imm= inst_.S.imm5tB << 5 | inst_.S.imm0t4;
    std::cout << "imm: " << imm << '\n';
}

void SType::ProcessFunct() { }

std::string BType::ToAssemblyString() { return "NULL"; }
void BType::Decode()
{
    std::cout << "B-Type Instruction\n";
    std::cout << "opcode: " << GetOpcode() << '\n';
    std::cout << "funct3: " << inst_.B.fct3 << '\n';
    std::cout << "rs1: " << inst_.B.rs1 << '\n';
    std::cout << "rs2: " << inst_.B.rs2 << '\n';
    uint32_t imm= (inst_.B.immC << 0xC) | (inst_.B.immB << 0xB) | (inst_.B.imm5tA << 5) | (inst_.B.imm1t4 << 1);
    std::cout << "imm: " << imm << '\n';
}

void BType::ProcessFunct() { }

std::string UType::ToAssemblyString() { return "NULL"; }
void UType::Decode()
{
    std::cout << "U-Type Instruction\n";
    std::cout << "opcode: " << GetOpcode() << '\n';
    std::cout << "rd: " << inst_.U.rd << '\n';
    std::cout << "imm: " << inst_.U.immCt1F << '\n';
}

void UType::ProcessFunct() { }

// JTypeInst
std::string JTypeInst::ToAssemblyString() { return "NULL"; }
void JTypeInst::Decode()
{
    std::cout << "J-Type Instruction\n";
    std::cout << "opcode: " << GetOpcode() << '\n';
    std::cout << "rd: " << inst_.J.rd << '\n';
    uint32_t imm= (inst_.J.imm14 << 0x14) | (inst_.J.immCt13 << 0xC) | (inst_.J.immB << 0xB) | (inst_.J.imm1tA << 1);
    std::cout << "imm: " << imm << '\n';
}

void JTypeInst::ProcessFunct() { }
