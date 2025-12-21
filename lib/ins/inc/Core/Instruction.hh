#pragma once

#include <cstdint>
#include <bitset>
#include <unordered_map>
#include <string>
#include <vector>
// #include <string_view>

#include "ISA/Inst.hh"

class IBaseTypeInst {
public:
    InstLayout inst_;          // Instruction parsing structure
    InstFormat format_;        // Instruction format
    std::bitset<32> bitField_; // 32-bit bit field storage
    std::string insHexStr_;
    std::vector<std::string> insStrVector_;
    using IntMapStrV_u= std::unordered_map<uint16_t, std::string_view>;

    IBaseTypeInst(InstLayout inst, InstFormat format): inst_(inst), format_(format) { }

    IBaseTypeInst(uint32_t ins, InstFormat format): inst_(ins), format_(format) { }

    virtual ~IBaseTypeInst()= default;

    virtual int SetBitField();
    virtual void ProcessFunct()= 0;
    virtual void Decode()      = 0;
    virtual std::string ToAssemblyString() = 0;
    virtual std::string GetFormatType() const = 0;
    virtual std::string ToAsm() const { return {}; }

    uint16_t GetOpcode() const { return inst_.lo_ & 0x7F; }
};

// R-type instruction
class RType: public IBaseTypeInst {
public:
    using IBaseTypeInst::IBaseTypeInst;
    S_RtypeField_t rTypeField_;
    std::string strRd_;
    std::string strRs1_;
    std::string strRs2_;
    std::string mnemonicStrIns_;
    std::string mnemonicStrRd_;
    std::string mnemonicStrRs1_;
    std::string mnemonicStrRs2_;
    const static inline IntMapStrV_u functMnemonicRTypeMap_ {
        { 0x00, "add"  },
        { 0x100, "sub"  },
        { 0x01, "sll"  },
        { 0x02, "slt"  },
        { 0x03, "sltu" },
        { 0x04, "xor"  },
        { 0x05, "srl"  },
        { 0x25, "sra"  },
        { 0x06, "or"   },
        { 0x07, "and"  }
    };


    void Decode() override;
    void ProcessFunct() override;
    std::string ToAssemblyString() override;
    std::string GetFormatType() const override { return "R"; }
};

// I-type
class IType: public IBaseTypeInst {
public:
    using IBaseTypeInst::IBaseTypeInst;
    const static inline IntMapStrV_u functMnemonicIMap_ {
        { 0x00,  "addi"  },
        { 0x02,  "slti"  },
        { 0x03,  "sltiu" },
        { 0x04,  "xori"  },
        { 0x06,  "ori"   },
        { 0x07,  "andi"  },
        { 0x01,  "slli"  },
        { 0x205, "srai"  }
    };

    void Decode() override;
    void ProcessFunct() override;
    std::string ToAssemblyString() override;
    std::string GetFormatType() const override { return "I"; }
};

// S-type
class SType: public IBaseTypeInst {
public:
    using IBaseTypeInst::IBaseTypeInst;

    void Decode() override;
    void ProcessFunct() override;
    std::string ToAssemblyString() override;
    std::string GetFormatType() const override { return "S"; }
};

// B-type
class BType: public IBaseTypeInst {
public:
    using IBaseTypeInst::IBaseTypeInst;

    void Decode() override;
    void ProcessFunct() override;
    std::string ToAssemblyString() override;
    std::string GetFormatType() const override { return "B"; }
};

// U-type
class UType: public IBaseTypeInst {
public:
    using IBaseTypeInst::IBaseTypeInst;

    void Decode() override;
    void ProcessFunct() override;
    std::string ToAssemblyString() override;
    std::string GetFormatType() const override { return "U"; }
};

// J-type
class JTypeInst: public IBaseTypeInst {
public:
    using IBaseTypeInst::IBaseTypeInst;

    void Decode() override;
    void ProcessFunct() override;
    std::string ToAssemblyString() override;
    std::string GetFormatType() const override { return "J"; }
};

// Date:25/12/19/16:23
