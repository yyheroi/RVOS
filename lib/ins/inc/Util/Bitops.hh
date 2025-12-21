#pragma once

#include <bitset>
#include <stdexcept>

// General utility function: Write value to specified continuous bit segment of bitset
template <size_t N>
inline void setBits(std::bitset<N> &bs, size_t start, size_t length, uint32_t value)
{
    if(start + length > N) {
        throw std::out_of_range("Bit range exceeds bitset size");
    }

    for(size_t i= 0; i < length; ++i) {
        size_t bitPos= start + i;
        bool bitVal  = (value >> i) & 0x1; // Extract the i-th bit of value
        bs.set(bitPos, bitVal);
    }
}