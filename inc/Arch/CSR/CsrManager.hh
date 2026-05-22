#include <cstdint>
namespace Arch::CSR {

enum class CsrReg: uint64_t {
    /*Machine Information Registers*/
    MVENDORID = 0xf11UL,
    MARCHID = 0xf12UL,
    MIMPID = 0xf13UL,
    MHARTID = 0xf14UL,
    /*Machine Trap Setup*/
    MSTATUS = 0x300UL,
    MISA = 0x301UL,
    MEDELEG = 0x302UL,
    MIDELEG = 0x303UL,
    MIE = 0x304UL,
    MTVEC = 0x305UL,
    MCOUNTEREN = 0x306UL,
    MSTATUSH = 0x310UL,
    /*Machine Trap Handling*/
    MSCRATCH = 0x340UL,
    MEPC = 0x341UL,
    MCAUSE = 0x342UL,
    MTVAL = 0x343UL,
    MIP = 0x344UL,
    MTINST = 0x34AUL,
    MTVAL2 = 0x34BUL,
};

class CsrManager {
public:
    CsrManager(const CsrManager &) noexcept = delete;
    CsrManager &operator=(const CsrManager &) noexcept = delete;
    CsrManager(CsrManager &&) noexcept = delete;
    CsrManager &operator=(CsrManager &&) noexcept = delete;
    CsrManager() noexcept = default;
    ~CsrManager() noexcept = default;

    [[nodiscard]] static CsrManager& Instance() noexcept
    {
        static CsrManager s_instance { };
        return s_instance;
    }

    void Write(CsrReg csr, uint64_t value) noexcept;
    [[nodiscard]] uint64_t Read(CsrReg csr) noexcept;


private:
    void writeReg(CsrReg csr, uint64_t value) noexcept;
    [[nodiscard]] uint64_t readReg(CsrReg csr) noexcept;
};

} // namespace Arch::CSR