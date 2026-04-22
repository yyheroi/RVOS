#pragma once

#include <cstddef>
#include <cstdint>

namespace RVOSHal {

/** Freestanding byte view (no libstdc++ span — avoids medany libstdc++.a at 0x80000000). */
struct ByteSpan {
    uint8_t *data{};
    std::size_t size{};

    [[nodiscard]] bool empty() const noexcept { return size == 0U; }
    [[nodiscard]] uint8_t *begin() const noexcept { return data; }
    [[nodiscard]] uint8_t *end() const noexcept { return data + size; }
};

} // namespace RVOSHal
