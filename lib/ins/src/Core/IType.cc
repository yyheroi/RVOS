#include <cctype>
#include <iostream>
#include <sstream>
#include <string>

#include "Core/IType.hh"
#include "ISA/Regs.hpp"

namespace {
// fence pred/succ nibble bit values: i=8, o=4, r=2, w=1 (spec letter order "iorw").
constexpr uint32_t FENCE_I= 8, FENCE_O= 4, FENCE_R= 2, FENCE_W= 1;

// Render one pred/succ nibble as iorw-style text; a zero nibble prints "0".
std::string fenceNibbleText(uint32_t nibble)
{
    nibble&= UINT32_C(0xF);
    if(0 == nibble) {
        return "0";
    }
    std::string text;
    if((nibble & FENCE_I) != 0) { text += 'i'; }
    if((nibble & FENCE_O) != 0) { text += 'o'; }
    if((nibble & FENCE_R) != 0) { text += 'r'; }
    if((nibble & FENCE_W) != 0) { text += 'w'; }
    return text;
}

// Parse one pred/succ operand: an iorw letter set ("w", "ior") or a numeric nibble ("0", "0xf").
uint32_t fenceNibbleBits(const std::string &token)
{
    uint32_t bits= 0;
    bool letters= !token.empty();
    for(const char c: token) {
        switch(std::tolower(static_cast<unsigned char>(c))) {
        case 'i': bits |= FENCE_I; break;
        case 'o': bits |= FENCE_O; break;
        case 'r': bits |= FENCE_R; break;
        case 'w': bits |= FENCE_W; break;
        default:  letters= false; break;
        }
        if(!letters) {
            break;
        }
    }
    if(letters) {
        return bits;
    }
    return static_cast<uint32_t>(std::stoul(token, nullptr, 0)) & UINT32_C(0xF);
}
} // namespace

IType::IType(uint32_t inst, InstFormat format, bool hasSetABI)
    : IBaseInstType(inst, format, hasSetABI)
{
    init();
}

IType::IType(std::vector<std::string> instAssembly, InstFormat format, bool hasSetABI)
    : IBaseInstType(std::move(instAssembly), format, hasSetABI)
{
    init();
}

void IType::Parse()
{
    InstBitsField_.emplace_back(static_cast<uint32_t>(Layout_.I.opc));
    InstBitsField_.emplace_back(static_cast<uint32_t>(Layout_.I.rd));
    InstBitsField_.emplace_back(static_cast<uint32_t>(Layout_.I.fct3));
    InstBitsField_.emplace_back(static_cast<uint32_t>(Layout_.I.rs1));
    InstBitsField_.emplace_back(static_cast<uint32_t>(Layout_.I.imm0tB));

    std::cout << "opcode: 0x" << std::hex << Opcode_ << '\n'
              << "Hexadecimal: 0x" << Layout_.entity_ << '\n'
              << "funct3: " << Layout_.I.fct3 << '\n'
              << "rs1: " << Layout_.I.rs1 << '\n'
              << "rd: " << Layout_.I.rd << '\n'
              << "imm: " << std::dec << static_cast<int32_t>(static_cast<int16_t>(Layout_.I.imm0tB & 0xFFF)) << '\n';
}

void IType::mnemonicHelper()
{
    const uint32_t opc= Layout_.I.opc;
    if(opc == 0x73) {
        if(Layout_.I.fct3 != 0) { // Zicsr: `csrrw rd, csr, rs1` / `csrrwi rd, csr, zimm`
            const auto rd = isa::LOOKUP_REG_NAME(Layout_.I.rd, HasSetABI_);
            std::ostringstream csrOs;
            csrOs << "0x" << std::hex << (Layout_.I.imm0tB & 0xFFF);
            const std::string csrStr= csrOs.str();

            if(Layout_.I.fct3 >= 5) { // immediate form: last operand is zimm
                const std::string zimmStr= std::to_string(Layout_.I.rs1);
                appendOperands({" ", rd, ",", std::string_view(csrStr), ",", std::string_view(zimmStr) });
            } else {
                const auto rs1= isa::LOOKUP_REG_NAME(Layout_.I.rs1, HasSetABI_);
                appendOperands({" ", rd, ",", std::string_view(csrStr), ",", rs1 });
            }
            return;
        }
        if(InstAssembly_.size() > 1) {
            InstAssembly_.resize(1);
        }
        return;
    }

    const int32_t imm= static_cast<int32_t>(static_cast<int16_t>(Layout_.I.imm0tB & 0xFFF));
    const std::string immStr= std::to_string(imm);

    if(opc == 0x0F) {
        if(Layout_.I.fct3 != 0) { // fence.i: no operands
            if(InstAssembly_.size() > 1) {
                InstAssembly_.resize(1);
            }
            return;
        }
        // fence: `pred, succ` occupy imm[7:4] / imm[3:0] (instruction bits [27:24] / [23:20]); fm must be zero.
        const std::string pred= fenceNibbleText(Layout_.I.imm0tB >> 4);
        const std::string succ= fenceNibbleText(Layout_.I.imm0tB);
        appendOperands({" ", std::string_view(pred), ",", std::string_view(succ) });
        return;
    }

    auto rd = isa::LOOKUP_REG_NAME(Layout_.I.rd, HasSetABI_);
    auto rs1= isa::LOOKUP_REG_NAME(Layout_.I.rs1, HasSetABI_);
    appendOperands({" ", rd, ",", rs1, ",", std::string_view(immStr) });
}

const std::vector<std::string> &IType::Disassembly()
{
    if(!InstTable_) {
        InstTable_= buildTable();
    }

    if(InstAssembly_.empty()) {
        const auto &info= LookupNameAndInfo();
        InstAssembly_.emplace_back(info.name_);
        mnemonicHelper();
    }

    return InstAssembly_;
}

const InstLayout &IType::Assembly()
{
    const auto &info= LookupIdxAndInfo();

    Layout_.I.opc= Opcode_= info.opcode_;
    const uint16_t key= info.funct_;

    if(info.opcode_ == 0x13) {
        Layout_.I.fct3  = key & 7;
        Layout_.I.imm0tB= (static_cast<uint32_t>((key >> 3) & 0x7F) << 5);
    } else if(info.opcode_ == 0x73) {
        if(ITypeKey::IS_CSR(key)) { // Zicsr: funct3 from key; csr# comes from operands
            Layout_.I.fct3  = (key >> 4) & 7;
            Layout_.I.imm0tB= 0;
        } else { // ecall / ebreak
            Layout_.I.fct3  = 0;
            Layout_.I.imm0tB= static_cast<uint32_t>(key & 0xFF);
        }
    } else {
        Layout_.I.fct3  = key & 7;
        Layout_.I.imm0tB= 0;
    }

    if(info.opcode_ == 0x73 && Layout_.I.fct3 == 0) { // ecall / ebreak carry no operands
        Layout_.I.rd = 0;
        Layout_.I.rs1= 0;
    }

    if(info.opcode_ == 0x0F) { // MISC-MEM: rd/rs1 are zero; fm comes from pred/succ
        Layout_.I.rd = 0;
        Layout_.I.rs1= 0;
        if(Layout_.I.fct3 != 0) { // fence.i: no operands
            Layout_.I.imm0tB= 0;
        } else if(InstAssembly_.size() == 1) { // bare `fence` means `fence iorw, iorw`
            Layout_.I.imm0tB= UINT32_C(0xFF);
        } else if(InstAssembly_.size() == 3) { // spec form: `fence pred, succ`
            Layout_.I.imm0tB= (fenceNibbleBits(InstAssembly_.at(1)) << 4)
                            |  fenceNibbleBits(InstAssembly_.at(2));
        } else if(InstAssembly_.size() >= 4) { // legacy numeric form: `fence rd, rs1, imm`
            Layout_.I.imm0tB= static_cast<uint32_t>(std::stoul(InstAssembly_.at(3), nullptr, 0)) & UINT32_C(0xFFF);
        }
    } else if(!InstAssembly_.empty() && InstAssembly_.size() >= 4) {
        if(info.opcode_ == 0x73 && Layout_.I.fct3 != 0) {
            // Zicsr: `csrrw rd, csr, rs1` / `csrrwi rd, csr, zimm`
            if(auto rdOpt= isa::LOOKUP_REG_IDX(InstAssembly_.at(1))) {
                Layout_.I.rd= *rdOpt;
            }
            Layout_.I.imm0tB= static_cast<uint32_t>(std::stoi(InstAssembly_.at(2), nullptr, 0)) & 0xFFF;
            if(auto rs1Opt= isa::LOOKUP_REG_IDX(InstAssembly_.at(3))) {
                Layout_.I.rs1= *rs1Opt;
            }
        } else {
            if(auto rdOpt= isa::LOOKUP_REG_IDX(InstAssembly_.at(1))) {
                Layout_.I.rd= *rdOpt;
            }
            if(auto rs1Opt= isa::LOOKUP_REG_IDX(InstAssembly_.at(2))) {
                Layout_.I.rs1= *rs1Opt;
            }
            const int32_t imm= std::stoi(InstAssembly_.at(3));
            if(info.opcode_ == 0x13) {
                if(Layout_.I.fct3 == 1 || Layout_.I.fct3 == 5) {
                    Layout_.I.imm0tB= (Layout_.I.imm0tB & UINT32_C(0xFE0)) | (static_cast<uint32_t>(imm) & 0x1F);
                } else {
                    Layout_.I.imm0tB= static_cast<uint32_t>(imm) & 0xFFF;
                }
            } else if(info.opcode_ != 0x73) {
                Layout_.I.imm0tB= static_cast<uint32_t>(imm) & 0xFFF;
            }
        }
    }

    mnemonicHelper();

    return Layout_;
}

IBaseInstType::KeyT IType::calculateFunctKey()
{
    switch(Layout_.I.opc) {
    case 0x13:
        FunctKey_= static_cast<KeyT>(((Layout_.I.imm0tB >> 5) << 3) | Layout_.I.fct3);
        break;
    case 0x73:
        if(Layout_.I.fct3 != 0) { // Zicsr: key on funct3 only; csr# is operand data
            FunctKey_= static_cast<KeyT>((0x73u << 8) | (static_cast<uint32_t>(Layout_.I.fct3) << 4));
        } else {
            FunctKey_= static_cast<KeyT>((0x73u << 8) | (Layout_.I.imm0tB & 0xFFFu));
        }
        break;
    default:
        FunctKey_= static_cast<KeyT>((static_cast<uint32_t>(Layout_.I.opc) << 8) | Layout_.I.fct3);
        break;
    }
    return FunctKey_;
}

IBaseInstType::pBiTable_u IType::buildTable()
{
    static auto s_instTable= [](const std::string &baseURL) -> pBiTable_u {
        BiLookupTable<KeyT>::intMapName_u code2info;
        BiLookupTable<KeyT>::strMapIndex_u name2info;

        for(const auto &entry: G_INST_TABLE) {
            if(0x00 == entry.opcode_ || entry.XLEN_.empty() || entry.name_.empty()) {
                continue;
            }
            auto manualURL= baseURL + std::string(entry.name_);

            code2info.emplace(entry.funct_,
                              BiLookupTable<KeyT>::NameInfo { .manual_= manualURL,
                                                              .XLEN_  = entry.XLEN_,
                                                              .name_  = entry.name_ });
            name2info.emplace(entry.name_,
                              BiLookupTable<KeyT>::IndexInfo { .manual_= manualURL,
                                                               .XLEN_  = entry.XLEN_,
                                                               .funct_ = entry.funct_,
                                                               .opcode_= entry.opcode_ });
        }

        return std::make_shared<const BiLookupTable<KeyT>>(std::move(code2info), std::move(name2info));
    }(BaseURL_);

    return s_instTable;
}
