#include <iostream>
#include "Core/InstFactory.hh"

std::unique_ptr<IBaseTypeInst> InstFactory::CreateInst(uint32_t ins)
{
    uint8_t opcode= ins & 0x7F; // Extract lower 7 bits of opcode

    auto it= G_Opcode2Format.find(opcode);
    if(it == G_Opcode2Format.end()) {
        std::cout << "Unsupported opcode: " << std::to_string(opcode) << '\n';
        return nullptr;
    }

    switch(it->second) {
    case InstFormat::R:
        return std::make_unique<RType>(ins, InstFormat::R);
    case InstFormat::I:
        return std::make_unique<IType>(ins, InstFormat::I);
    case InstFormat::S:
        return std::make_unique<SType>(ins, InstFormat::S);
    case InstFormat::B:
        return std::make_unique<BType>(ins, InstFormat::B);
    case InstFormat::U:
        return std::make_unique<UType>(ins, InstFormat::U);
    case InstFormat::J:
        return std::make_unique<JTypeInst>(ins, InstFormat::J);
    default:
        throw std::logic_error("Unsupported instruction format");
    }
}
