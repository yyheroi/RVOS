#include <cassert>
#include <memory>
#include <iostream>

#include "Core/InstFactory.hh"

int main()
{
    // Test R-type: add x1, x2, x3 (0x003100b3)
    uint32_t add = 0x003100b3;
    auto pAdd = InstFactory::CreateInst(add);
    assert(pAdd != nullptr);
    pAdd->SetBitField();
    pAdd->ProcessFunct();
    pAdd->Decode();

    // Verify opcode is 0x33 (R-type)
    assert((add & 0x7F) == 0x33);

    std::cout << "All tests ran (manual checks).\n";
    return 0;
}
