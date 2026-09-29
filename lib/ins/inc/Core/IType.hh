#pragma once
#include <array>

#include "IBaseInstType.hh"

namespace ITypeKey {

// OP-IMM (0x13): table key funct_ is only (imm[11:5] << 3) | funct3 (matches decode calculateFunctKey).
constexpr uint16_t OP_IMM(uint8_t imm7, uint8_t f3)
{
    return (static_cast<uint16_t>(imm7) << 3) | static_cast<uint16_t>(f3 & 7);
}
// Other I-type opcodes: lookup key (opcode<<8)|funct3 to avoid collisions with OP-IMM funct_ keys.
constexpr uint16_t OPC_F3(uint8_t opc, uint8_t f3)
{
    return (static_cast<uint16_t>(opc) << 8) | static_cast<uint16_t>(f3 & 7);
}
// SYSTEM (0x73): ecall/ebreak key is (0x73 << 8) | imm (ecall: 0, ebreak: 1).
constexpr uint16_t SYSTEM(uint8_t imm)
{
    return (static_cast<uint16_t>(0x73) << 8) | static_cast<uint16_t>(imm & 0xFF);
}
// Zicsr (0x73): CSR ops key is (0x73 << 8) | (funct3 << 4); the csr number is operand data, not part of the key.
constexpr uint16_t CSR(uint8_t f3)
{
    return (static_cast<uint16_t>(0x73) << 8) | (static_cast<uint16_t>(f3 & 7) << 4);
}
// True when a SYSTEM (0x73) table key identifies a Zicsr CSR operation.
constexpr bool IS_CSR(uint16_t key)
{
    return ((key >> 4) & 7) != 0;
}

} // namespace ITypeKey

class IType: public IBaseInstType {
public:
    // RV32I / Zifencei / Zicsr: OP-IMM rows use funct_ = (imm[11:5]<<3)|funct3; other opcodes use (opcode<<8)|funct3;
    // SYSTEM rows use ITypeKey::SYSTEM()/CSR().
    constexpr static std::array<InstInfo, 25> G_INST_TABLE= {
        { { .name_="lb",    .XLEN_="RV32I",   .funct_=ITypeKey::OPC_F3(0x03, 0), .opcode_=0x03 },
          { .name_="lh",    .XLEN_="RV32I",   .funct_=ITypeKey::OPC_F3(0x03, 1), .opcode_=0x03 },
          { .name_="lw",    .XLEN_="RV32I",   .funct_=ITypeKey::OPC_F3(0x03, 2), .opcode_=0x03 },
          { .name_="lbu",   .XLEN_="RV32I",   .funct_=ITypeKey::OPC_F3(0x03, 4), .opcode_=0x03 },
          { .name_="lhu",   .XLEN_="RV32I",   .funct_=ITypeKey::OPC_F3(0x03, 5), .opcode_=0x03 },

          { .name_="jalr",  .XLEN_="RV32I",   .funct_=ITypeKey::OPC_F3(0x67, 0), .opcode_=0x67 },

          { .name_="addi",  .XLEN_="RV32I",   .funct_=ITypeKey::OP_IMM(0, 0), .opcode_=0x13 },
          { .name_="slli",  .XLEN_="RV32I",   .funct_=ITypeKey::OP_IMM(0, 1), .opcode_=0x13 },
          { .name_="slti",  .XLEN_="RV32I",   .funct_=ITypeKey::OP_IMM(0, 2), .opcode_=0x13 },
          { .name_="sltiu", .XLEN_="RV32I",   .funct_=ITypeKey::OP_IMM(0, 3), .opcode_=0x13 },
          { .name_="xori",  .XLEN_="RV32I",   .funct_=ITypeKey::OP_IMM(0, 4), .opcode_=0x13 },
          { .name_="srli",  .XLEN_="RV32I",   .funct_=ITypeKey::OP_IMM(0, 5), .opcode_=0x13 },
          { .name_="srai",  .XLEN_="RV32I",   .funct_=ITypeKey::OP_IMM(0x20, 5), .opcode_=0x13 },
          { .name_="ori",   .XLEN_="RV32I",   .funct_=ITypeKey::OP_IMM(0, 6), .opcode_=0x13 },
          { .name_="andi",  .XLEN_="RV32I",   .funct_=ITypeKey::OP_IMM(0, 7), .opcode_=0x13 },

          { .name_="fence",   .XLEN_="RV32I", .funct_=ITypeKey::OPC_F3(0x0F, 0), .opcode_=0x0F },
          { .name_="fence.i", .XLEN_="Zifencei", .funct_=ITypeKey::OPC_F3(0x0F, 1), .opcode_=0x0F },

          { .name_="ecall",  .XLEN_="RV32I", .funct_=ITypeKey::SYSTEM(0), .opcode_=0x73 },
          { .name_="ebreak", .XLEN_="RV32I", .funct_=ITypeKey::SYSTEM(1), .opcode_=0x73 },

          { .name_="csrrw",  .XLEN_="Zicsr", .funct_=ITypeKey::CSR(1), .opcode_=0x73 },
          { .name_="csrrs",  .XLEN_="Zicsr", .funct_=ITypeKey::CSR(2), .opcode_=0x73 },
          { .name_="csrc",   .XLEN_="Zicsr", .funct_=ITypeKey::CSR(3), .opcode_=0x73 },
          { .name_="csrrwi", .XLEN_="Zicsr", .funct_=ITypeKey::CSR(5), .opcode_=0x73 },
          { .name_="csrrsi", .XLEN_="Zicsr", .funct_=ITypeKey::CSR(6), .opcode_=0x73 },
          { .name_="csrrci", .XLEN_="Zicsr", .funct_=ITypeKey::CSR(7), .opcode_=0x73 } }
    };

public:
    IType(uint32_t inst, InstFormat format, bool hasSetABI= false);
    IType(std::vector<std::string> instAssembly, InstFormat format, bool hasSetABI= false);

public:
    void Parse() override;

    [[nodiscard]] const std::vector<std::string> &Disassembly() override;
    [[nodiscard]] const InstLayout &Assembly() override;

private:
    KeyT calculateFunctKey() override;
    void mnemonicHelper() override;
    [[nodiscard]] pBiTable_u buildTable() override;
};
