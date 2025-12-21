#include <cstdint>

enum class InstFormat : uint8_t { // Instruction format enumeration

    R, // Register-Register
    I, // Immediate-Register
    S, // Store instruction
    B, // Branch instruction
    U, // Upper immediate (lui/auipc)
    J, // Jump instruction (jal)
};

// NOLINTBEGIN
typedef struct S_RtypeField_t { // R(opcode:[0:6] | rd:[7:11] | funct3:[12:14] | rs1:[15:19] | rs2:[20:24] | funct7:[25:31])
    uint32_t opc : 7;
    uint32_t rd  : 5;
    uint32_t fct3: 3;
    uint32_t rs1 : 5;
    uint32_t rs2 : 5;
    uint32_t fct7: 7;
} S_RtypeField_t;

typedef struct S_ItypeField_t { // I(opcode:[0:6] | rd:[7:11] | funct3:[12:14] | rs1:[15:19] | imm0tB:[20:31])
    uint32_t opc   : 7;
    uint32_t rd    : 5;
    uint32_t fct3  : 3;
    uint32_t rs1   : 5;
    uint32_t imm0tB: 12; // imm0~imm11
} S_ItypeField_t;

typedef struct S_StypeField_t {// S(opcode:[0:6] | imm0t4:[7:11] | funct3:[12:14] | rs1:[15:19] | rs2:[20:24] | imm5tB:[25:31])
    uint32_t opc   : 7;
    uint32_t imm0t4: 5; // imm0~imm4
    uint32_t fct3  : 3;
    uint32_t rs1   : 5;
    uint32_t rs2   : 5;
    uint32_t imm5tB: 7; // imm5~imm11
} S_StypeField_t;

typedef struct S_BtypeField_t {  // S(opcode:[0:6] | imm0t4:[7:11] | funct3:[12:14] | rs1:[15:19] | rs2:[20:24] | imm5tB:[25:31])
    uint32_t opc   : 7;
    uint32_t immB  : 1; // imm11
    uint32_t imm1t4: 4; // imm1~imm4
    uint32_t fct3  : 3;
    uint32_t rs1   : 5;
    uint32_t rs2   : 5;
    uint32_t imm5tA: 6; // imm5~imm10
    uint32_t immC  : 1; // imm12
} S_BtypeField_t;

typedef struct S_UtypeField_t {  // U(opcode:[0:6] | rd:[7:11] | immCt1F:[12:31])
    uint32_t opc    : 7;
    uint32_t rd     : 5;
    uint32_t immCt1F: 20; // imm12~imm31
} S_UtypeField_t;

typedef struct S_JtypeField_t { // J(opcode:[0:6] | rd:[7:11] | immCt13:[12:19] | immB:[20:20] | imm1tA:[21:30] | imm14:[31:31])
    uint32_t opc    : 7;
    uint32_t rd     : 5;
    uint32_t immCt13: 8;  // imm12~imm19
    uint32_t immB   : 1;  // imm11
    uint32_t imm1tA : 10; // imm1~imm10
    uint32_t imm14  : 1;  // imm20
} S_JtypeField_t;

// 32-bit instruction parsing union
union InstLayout {
    uint32_t lo_; // Complete 32-bit instruction
    S_RtypeField_t R;
    S_ItypeField_t I;
    S_StypeField_t S;
    S_BtypeField_t B;
    S_UtypeField_t U;
    S_JtypeField_t J;
    // Constructor: Initialize from 32-bit instruction
    explicit InstLayout(uint32_t instruction= 0): lo_(instruction) { }
};

// NOLINTEND
