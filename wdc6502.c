#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#include "include/wdc6502.h"
#include "include/bus.h"

#define INSTR(OP, ADDR, INSTRUCTION, DUMMY) \
    case OP: \
        cpu->dummy_read = DUMMY; \
        ADDR(cpu); \
        INSTRUCTION(cpu); \
        break

#define INSTR_IMPL(OP, INSTRUCTION) \
    case OP: \
        INSTRUCTION(cpu); \
        break;

typedef enum flag : short {
    C, Z, I, D, B, U, V, N
} flag_t;

typedef enum cpu_vector : uint16_t {
    VEC_BRK_IRQ = 0xFFFE,
    VEC_NMI = 0xFFFA,
    VEC_RST = 0xFFFC
} vector_t;

static inline void enter_interrupt(WDC6502_t* cpu, bool brk);

static void set_flag(WDC6502_t* cpu, flag_t flag, bool value);
static inline bool get_flag(WDC6502_t* cpu, flag_t flag);
static inline void set_nz(WDC6502_t* cpu, uint8_t value);

static uint8_t read_b(WDC6502_t* cpu, uint16_t address);
static uint16_t read_w(WDC6502_t* cpu, uint16_t address);

static inline uint16_t read_w_masked(WDC6502_t* cpu, uint16_t address, uint16_t address_mask);

static inline uint8_t read_imm_b(WDC6502_t* cpu);
static inline uint16_t read_imm_w(WDC6502_t* cpu);

static void write_b(WDC6502_t* cpu, uint16_t address, uint8_t data);

static inline void push_stack(WDC6502_t* cpu, uint8_t data);
static inline uint8_t pull_stack(WDC6502_t* cpu);

static void cycle(WDC6502_t* cpu);

static inline bool page_crossed(uint16_t addr_a, uint16_t addr_b);

static inline void branch(WDC6502_t* cpu, bool should_branch);
static inline void compare(WDC6502_t* cpu, uint8_t value);
static inline void load_register(WDC6502_t* cpu, uint8_t* reg);
static inline void transfer_register(WDC6502_t* cpu, uint8_t* reg_src, uint8_t* reg_dest);

static inline void addr_imm(WDC6502_t* cpu);
static inline void addr_a(WDC6502_t* cpu);
static inline void addr_zp(WDC6502_t* cpu);
static inline void addr_a_x(WDC6502_t* cpu);
static inline void addr_a_y(WDC6502_t* cpu);
static inline void addr_zp_x(WDC6502_t* cpu);
static inline void addr_zp_y(WDC6502_t* cpu);
static inline void addr_indr(WDC6502_t* cpu);
static inline void addr_indr_x(WDC6502_t* cpu);
static inline void addr_indr_y(WDC6502_t* cpu);

static inline void ADC(WDC6502_t* cpu);
static inline void AND(WDC6502_t* cpu);
static inline void ASL(WDC6502_t* cpu);     static inline void ASL_A(WDC6502_t* cpu);
static inline void BCC(WDC6502_t* cpu);
static inline void BCS(WDC6502_t* cpu);
static inline void BEQ(WDC6502_t* cpu);
static inline void BMI(WDC6502_t* cpu);
static inline void BNE(WDC6502_t* cpu);
static inline void BPL(WDC6502_t* cpu);
static inline void BVC(WDC6502_t* cpu);
static inline void BVS(WDC6502_t* cpu);
static inline void BIT(WDC6502_t* cpu);
static inline void BRK(WDC6502_t* cpu);
static inline void CLC(WDC6502_t* cpu);
static inline void CLD(WDC6502_t* cpu);
static inline void CLI(WDC6502_t* cpu);
static inline void CLV(WDC6502_t* cpu);
static inline void CMP(WDC6502_t* cpu); 
static inline void CPY(WDC6502_t* cpu);
static inline void CPX(WDC6502_t* cpu);
static inline void DEC(WDC6502_t* cpu);
static inline void DEX(WDC6502_t* cpu);
static inline void DEY(WDC6502_t* cpu);
static inline void EOR(WDC6502_t* cpu);
static inline void INC(WDC6502_t* cpu);
static inline void INX(WDC6502_t* cpu);
static inline void INY(WDC6502_t* cpu);
static inline void JMP(WDC6502_t* cpu);
static inline void JSR(WDC6502_t* cpu);
static inline void LDA(WDC6502_t* cpu);
static inline void LDX(WDC6502_t* cpu);
static inline void LDY(WDC6502_t* cpu);
static inline void LSR(WDC6502_t* cpu);     static inline void LSR_A(WDC6502_t* cpu);
static inline void NOP(WDC6502_t* cpu);
static inline void ORA(WDC6502_t* cpu);
static inline void PHA(WDC6502_t* cpu);
static inline void PHP(WDC6502_t* cpu);
static inline void PLA(WDC6502_t* cpu);
static inline void PLP(WDC6502_t* cpu);
static inline void ROL(WDC6502_t* cpu);     static inline void ROL_A(WDC6502_t* cpu);
static inline void ROR(WDC6502_t* cpu);     static inline void ROR_A(WDC6502_t* cpu);
static inline void RTI(WDC6502_t* cpu);
static inline void RTS(WDC6502_t* cpu);
static inline void SBC(WDC6502_t* cpu);
static inline void SEC(WDC6502_t* cpu);
static inline void SED(WDC6502_t* cpu);
static inline void SEI(WDC6502_t* cpu);
static inline void STA(WDC6502_t* cpu);
static inline void STX(WDC6502_t* cpu);
static inline void STY(WDC6502_t* cpu);
static inline void TAX(WDC6502_t* cpu);
static inline void TAY(WDC6502_t* cpu);
static inline void TSX(WDC6502_t* cpu);
static inline void TXA(WDC6502_t* cpu);
static inline void TXS(WDC6502_t* cpu);
static inline void TYA(WDC6502_t* cpu);

struct WDC6502 {
    struct {
        uint8_t A;
        uint8_t X;
        uint8_t Y;
        uint8_t P;
        uint8_t S;

        uint16_t PC;
    } registers;

    bus_t* bus;

    uint16_t operand_address;
    bool dummy_read : 1;
    
    bool irq_pending : 1;
    bool nmi_pending : 1;

    bool should_take_irq : 1;

    unsigned int cycle_count;
};

WDC6502_t* new_wdc6502(bus_t* bus) {
    WDC6502_t* result = (WDC6502_t*) calloc(sizeof(WDC6502_t), 1);
    result->bus = bus;
    result->should_take_irq = true;
    return result;
}

void free_wdc6502(WDC6502_t* cpu) {
    free(cpu);
}

void wdc6502_print_state(WDC6502_t* cpu) {
    printf("A: %u\n", cpu->registers.A);
    printf("X: %u\n", cpu->registers.X);
    printf("Y: %u\n", cpu->registers.Y);
    printf("S: %u\n", cpu->registers.S);
    printf("PC: %u\n\n", cpu->registers.PC);

    printf("P: %u\n\n", cpu->registers.P);

    printf("Flags: \n");
    printf("C: %u\n", get_flag(cpu, C));
    printf("Z: %u\n", get_flag(cpu, Z));
    printf("I: %u\n", get_flag(cpu, I));
    printf("D: %u\n", get_flag(cpu, D));
    printf("B: %u\n", get_flag(cpu, B));
    printf("V: %u\n", get_flag(cpu, V));
    printf("N: %u\n\n", get_flag(cpu, N));

    printf("Cycle count: %u\n\n", cpu->cycle_count);
}

void wdc6502_set_irq_line(WDC6502_t* cpu, bool value) {
    cpu->irq_pending = value;
}

void wdc6502_set_nmi(WDC6502_t* cpu) {
    cpu->nmi_pending = true;
}
// //

void set_flag(WDC6502_t* cpu, flag_t flag, bool value) {
    if (value) {
        cpu->registers.P |= (1 << flag);
        return;
    }

    cpu->registers.P &= ~(1 << flag);
}

bool get_flag(WDC6502_t* cpu, flag_t flag) {
    return (cpu->registers.P >> flag) & 1;
}

uint8_t read_b(WDC6502_t* cpu, uint16_t address) {
    cycle(cpu);
    return read_bus(cpu->bus, address);
}

uint16_t read_w(WDC6502_t* cpu, uint16_t address) {
    uint8_t low = read_b(cpu, address);
    uint8_t high = read_b(cpu, address + 1);
    return (high << 8) | low;
}

uint8_t read_imm_b(WDC6502_t* cpu) {
    return read_b(cpu, cpu->registers.PC++);
}

uint16_t read_w_masked(WDC6502_t* cpu, uint16_t address, uint16_t address_mask) {
    uint8_t low = read_b(cpu, address & address_mask);
    uint8_t high = read_b(cpu, (address + 1) & address_mask);
    return (high << 8) | low;
}

uint16_t read_imm_w(WDC6502_t* cpu) {
    uint8_t low = read_imm_b(cpu);
    uint8_t high = read_imm_b(cpu);
    return (high << 8) | low;
}

void write_b(WDC6502_t* cpu, uint16_t address, uint8_t data) {
    write_bus(cpu->bus, address, data);
    cycle(cpu);
}

void push_stack(WDC6502_t* cpu, uint8_t data) {
    write_b(cpu, (0x01 << 8) | cpu->registers.S--, data);
}
uint8_t pull_stack(WDC6502_t* cpu) {
    return read_b(cpu, (0x01 << 8) | ++cpu->registers.S);
}

bool page_crossed(uint16_t addr_a, uint16_t addr_b) {
    return (addr_a & 0xFF00) != (addr_b & 0xFF00);
}

void branch(WDC6502_t* cpu, bool should_branch) {
    int8_t offset = (int8_t) read_imm_b(cpu);

    if (should_branch) {
        if (page_crossed(cpu->registers.PC, cpu->registers.PC + offset)) cycle(cpu);
        cycle(cpu);
        cpu->registers.PC += offset;
    }
}

void transfer_register(WDC6502_t* cpu, uint8_t* reg_src, uint8_t* reg_dest) {
    *reg_dest = *reg_src;
    cycle(cpu);
    set_nz(cpu, *reg_dest);
}

void enter_interrupt(WDC6502_t* cpu, bool brk) {
    uint8_t reg_p = cpu->registers.P;
    vector_t vector = VEC_BRK_IRQ;

    if (brk) {
        read_imm_b(cpu);
        printf("brk\n"); 
        reg_p |= (0b11 << 4);
    } else {
        read_b(cpu, cpu->registers.PC);
    }

    push_stack(cpu, cpu->registers.PC >> 8);
    push_stack(cpu, cpu->registers.PC & 0xFF);
    push_stack(cpu, reg_p);

    if (cpu->nmi_pending) {
        printf("nmi\n");     
        vector = VEC_NMI;
        cpu->nmi_pending = false;
    } else {
        printf("maybe irq or brk\n"); 
    }

    set_flag(cpu, I, true);
    cpu->should_take_irq = false;

    cpu->registers.PC = read_w(cpu, vector);
}

void compare(WDC6502_t* cpu, uint8_t value) {
    uint8_t operand = read_b(cpu, cpu->operand_address);
    uint8_t result = value - operand;
    set_flag(cpu, C, value >= operand);
    set_nz(cpu, result);
}

void set_nz(WDC6502_t* cpu, uint8_t value) {
    set_flag(cpu, Z, value == 0);
    set_flag(cpu, N, value >> 7);
}

void load_register(WDC6502_t* cpu, uint8_t* reg) {
    *reg = read_b(cpu, cpu->operand_address);
    set_nz(cpu, *reg);
}

void cycle(WDC6502_t* cpu) {
    cpu->cycle_count++;
    //handle logic
}

// <addressing> //
void addr_imm(WDC6502_t* cpu) {
    cpu->operand_address = cpu->registers.PC++;
}

void addr_a(WDC6502_t* cpu) {
    cpu->operand_address = read_imm_w(cpu);
}

void addr_zp(WDC6502_t* cpu) {
    cpu->operand_address = read_imm_b(cpu);
}

void addr_a_x(WDC6502_t* cpu) {
    uint16_t addr = read_imm_w(cpu);
    if (page_crossed(addr, addr + cpu->registers.X) || cpu->dummy_read) cycle(cpu);
    cpu->operand_address = addr + cpu->registers.X;
}

void addr_a_y(WDC6502_t* cpu) {
    uint16_t addr = read_imm_w(cpu);
    if (page_crossed(addr, addr + cpu->registers.Y) || cpu->dummy_read) cycle(cpu);
    cpu->operand_address = addr + cpu->registers.Y;
}

void addr_zp_x(WDC6502_t* cpu) {
    cycle(cpu);
    cpu->operand_address = (read_imm_b(cpu) + cpu->registers.X) & 0xFF;
}

void addr_zp_y(WDC6502_t* cpu) {
    cycle(cpu);
    cpu->operand_address = (read_imm_b(cpu) + cpu->registers.Y) & 0xFF;
}

void addr_indr(WDC6502_t* cpu) {
    uint16_t ptr = read_imm_w(cpu);
    uint8_t page = ptr >> 8;
    uint8_t low = read_b(cpu, ptr);
    cpu->operand_address =  (read_b(cpu, (page << 8) | ((ptr + 1) & 0xFF)) << 8) | low;
}

void addr_indr_x(WDC6502_t* cpu) {
    uint8_t ptr_a = read_imm_b(cpu);
    read_b(cpu, ptr_a);
    ptr_a += cpu->registers.X;
    cpu->operand_address = read_w_masked(cpu, ptr_a, 0xFF);
}

void addr_indr_y(WDC6502_t* cpu) {
    uint8_t ptr_a = read_imm_b(cpu);
    uint16_t addr = read_w_masked(cpu, ptr_a, 0xFF);
    if (cpu->dummy_read || page_crossed(addr, addr + cpu->registers.Y)) cycle(cpu);
    cpu->operand_address = addr + cpu->registers.Y;
}
// //

void wdc6502_execute_instruction(WDC6502_t* cpu) {
    cpu->cycle_count = 0;
    uint8_t opcode = read_imm_b(cpu);

    switch (opcode) {
        INSTR(0x69, addr_imm, ADC, false);
        INSTR(0x65, addr_zp, ADC, false);
        INSTR(0x75, addr_zp_x, ADC, false);
        INSTR(0x6D, addr_a, ADC, false);
        INSTR(0x7D, addr_a_x, ADC, false);
        INSTR(0x79, addr_a_y, ADC, false);
        INSTR(0x61, addr_indr_x, ADC, false);
        INSTR(0x71, addr_indr_y, ADC, false);

        INSTR(0x29, addr_imm, AND, false);
        INSTR(0x25, addr_zp, AND, false);
        INSTR(0x35, addr_zp_x, AND, false);
        INSTR(0x2D, addr_a, AND, false);
        INSTR(0x3D, addr_a_x, AND, false);
        INSTR(0x39, addr_a_y, AND, false);
        INSTR(0x21, addr_indr_x, AND, false);
        INSTR(0x31, addr_indr_y, AND, false);

        INSTR_IMPL(0x0A, ASL_A);
        INSTR(0x06, addr_zp, ASL, true);
        INSTR(0x16, addr_zp_x, ASL, true);
        INSTR(0x0E, addr_a, ASL, true);
        INSTR(0x1E, addr_a_x, ASL, true);

        INSTR_IMPL(0x90, BCC);
        INSTR_IMPL(0xB0, BCS);
        INSTR_IMPL(0xF0, BEQ);
        INSTR_IMPL(0x30, BMI);
        INSTR_IMPL(0xD0, BNE);
        INSTR_IMPL(0x10, BPL);
        INSTR_IMPL(0x50, BVC);
        INSTR_IMPL(0x70, BVS);

        INSTR(0x24, addr_zp, BIT, false);
        INSTR(0x2C, addr_a, BIT, false);

        INSTR_IMPL(0x00, BRK);

        INSTR_IMPL(0x18, CLC);
        INSTR_IMPL(0xD8, CLD);
        INSTR_IMPL(0x58, CLI);
        INSTR_IMPL(0xB8, CLV);

        INSTR(0xC9, addr_imm, CMP, false);
        INSTR(0xC5, addr_zp, CMP, false);
        INSTR(0xD5, addr_zp_x, CMP, false);
        INSTR(0xCD, addr_a, CMP, false);
        INSTR(0xDD, addr_a_x, CMP, false);
        INSTR(0xD9, addr_a_y, CMP, false);
        INSTR(0xC1, addr_indr_x, CMP, false);
        INSTR(0xD1, addr_indr_y, CMP, false);

        INSTR(0xE0, addr_imm, CPX, false);
        INSTR(0xE4, addr_zp, CPX, false);
        INSTR(0xEC, addr_a, CPX, false);

        INSTR(0xC0, addr_imm, CPY, false);
        INSTR(0xC4, addr_zp, CPY, false);
        INSTR(0xCC, addr_a, CPY, false);

        INSTR(0xC6, addr_zp, DEC, true);
        INSTR(0xD6, addr_zp_x, DEC, true);
        INSTR(0xCE, addr_a, DEC, true);
        INSTR(0xDE, addr_a_x, DEC, true);

        INSTR_IMPL(0xCA, DEX);
        INSTR_IMPL(0x88, DEY);

        INSTR(0x49, addr_imm, EOR, false);
        INSTR(0x45, addr_zp, EOR, false);
        INSTR(0x55, addr_zp_x, EOR, false);
        INSTR(0x4D, addr_a, EOR, false);
        INSTR(0x5D, addr_a_x, EOR, false);
        INSTR(0x59, addr_a_y, EOR, false);
        INSTR(0x41, addr_indr_x, EOR, false);
        INSTR(0x51, addr_indr_y, EOR, false);

        INSTR(0xE6, addr_zp, INC, true);
        INSTR(0xF6, addr_zp_x, INC, true);
        INSTR(0xEE, addr_a, INC, true);
        INSTR(0xFE, addr_a_x, INC, true);
        
        INSTR_IMPL(0xE8, INX);
        INSTR_IMPL(0xC8, INY);

        INSTR(0x4C, addr_a, JMP, false);
        INSTR(0x6C, addr_indr, JMP, false);

        INSTR(0x20, addr_a, JSR, false);

        INSTR(0xA9, addr_imm, LDA, false);
        INSTR(0xA5, addr_zp, LDA, false);
        INSTR(0xB5, addr_zp_x, LDA, false);
        INSTR(0xAD, addr_a, LDA, false);
        INSTR(0xBD, addr_a_x, LDA, false);
        INSTR(0xB9, addr_a_y, LDA, false);
        INSTR(0xA1, addr_indr_x, LDA, false);
        INSTR(0xB1, addr_indr_y, LDA, false);

        INSTR(0xA2, addr_imm, LDX, false);
        INSTR(0xA6, addr_zp, LDX, false);
        INSTR(0xB6, addr_zp_y, LDX, false);
        INSTR(0xAE, addr_a, LDX, false);
        INSTR(0xBE, addr_a_y, LDX, false);

        INSTR(0xA0, addr_imm, LDY, false);
        INSTR(0xA4, addr_zp, LDY, false);
        INSTR(0xB4, addr_zp_x, LDY, false);
        INSTR(0xAC, addr_a, LDY, false);
        INSTR(0xBC, addr_a_x, LDY, false);

        INSTR_IMPL(0x4A, LSR_A);
        INSTR(0x46, addr_zp, LSR, true);
        INSTR(0x56, addr_zp_x, LSR, true);
        INSTR(0x4E, addr_a, LSR, true);
        INSTR(0x5E, addr_a_x, LSR, true);

        INSTR_IMPL(0xEA, NOP);

        INSTR(0x09, addr_imm, ORA, false);
        INSTR(0x05, addr_zp, ORA, false);
        INSTR(0x15, addr_zp_x, ORA, false);
        INSTR(0x0D, addr_a, ORA, false);
        INSTR(0x1D, addr_a_x, ORA, false);
        INSTR(0x19, addr_a_y, ORA, false);
        INSTR(0x01, addr_indr_x, ORA, false);
        INSTR(0x11, addr_indr_y, ORA, false);

        INSTR_IMPL(0x48, PHA);
        INSTR_IMPL(0x08, PHP);

        INSTR_IMPL(0x68, PLA);
        INSTR_IMPL(0x28, PLP);

        INSTR_IMPL(0x2A, ROL_A);
        INSTR(0x26, addr_zp, ROL, true);
        INSTR(0x36, addr_zp_x, ROL, true);
        INSTR(0x2E, addr_a, ROL, true);
        INSTR(0x3E, addr_a_x, ROL, true);

        INSTR_IMPL(0x6A, ROR_A);
        INSTR(0x66, addr_zp, ROR, true);
        INSTR(0x76, addr_zp_x, ROR, true);
        INSTR(0x6E, addr_a, ROR, true);
        INSTR(0x7E, addr_a_x, ROR, true);

        INSTR_IMPL(0x40, RTI);
        INSTR_IMPL(0x60, RTS);

        INSTR(0xE9, addr_imm, SBC, false);
        INSTR(0xE5, addr_zp, SBC, false);
        INSTR(0xF5, addr_zp_x, SBC, false);
        INSTR(0xED, addr_a, SBC, false);
        INSTR(0xFD, addr_a_x, SBC, false);
        INSTR(0xF9, addr_a_y, SBC, false);
        INSTR(0xE1, addr_indr_x, SBC, false);
        INSTR(0xF1, addr_indr_y, SBC, false);

        INSTR_IMPL(0x38, SEC);
        INSTR_IMPL(0xF8, SED);
        INSTR_IMPL(0x78, SEI);

        INSTR(0x85, addr_zp, STA, true);
        INSTR(0x95, addr_zp_x, STA, true);
        INSTR(0x8D, addr_a, STA, true);
        INSTR(0x9D, addr_a_x, STA, true);
        INSTR(0x99, addr_a_y, STA, true);
        INSTR(0x81, addr_indr_x, STA, true);
        INSTR(0x91, addr_indr_y, STA, true);

        INSTR(0x86, addr_zp, STX, true);
        INSTR(0x96, addr_zp_y, STX, true);
        INSTR(0x8E, addr_a, STX, true);

        INSTR(0x84, addr_zp, STY, true);
        INSTR(0x94, addr_zp_x, STY, true);
        INSTR(0x8C, addr_a, STY, true);

        INSTR_IMPL(0xAA, TAX);
        INSTR_IMPL(0xA8, TAY);
        INSTR_IMPL(0xBA, TSX);
        INSTR_IMPL(0x8A, TXA);
        INSTR_IMPL(0x9A, TXS);
        INSTR_IMPL(0x98, TYA);
    }

    if (cpu->nmi_pending || (cpu->should_take_irq && cpu->irq_pending)) {
        enter_interrupt(cpu, false);
    }

    cpu->should_take_irq = !get_flag(cpu, I);
}


// <instructions> //
void ADC(WDC6502_t* cpu) {
    uint8_t operand = read_b(cpu, cpu->operand_address);
    unsigned int result = cpu->registers.A + operand + get_flag(cpu, C);
    set_flag(cpu, C, result > 0xFF);
    set_flag(cpu, V, (result ^ cpu->registers.A) & (result ^ operand) & 0x80);
    set_nz(cpu, result);
    cpu->registers.A = result & 0xFF;
}

void AND(WDC6502_t* cpu) {
    uint8_t operand = read_b(cpu, cpu->operand_address);
    cpu->registers.A &= operand;
    set_nz(cpu, cpu->registers.A);
}

void ASL_A(WDC6502_t* cpu) {
    bool c = cpu->registers.A >> 7;
    cpu->registers.A <<= 1;
    set_nz(cpu, cpu->registers.A);
    set_flag(cpu, C, c);
    cycle(cpu);
}

void ASL(WDC6502_t* cpu) {
    uint8_t operand = read_b(cpu, cpu->operand_address);
    bool c = operand >> 7;
    write_b(cpu, cpu->operand_address, operand);
    operand <<= 1;
    write_b(cpu, cpu->operand_address, operand);
    set_nz(cpu, operand);
    set_flag(cpu, C, c);
}

void BCC(WDC6502_t* cpu) {
    branch(cpu, !get_flag(cpu, C));
}

void BCS(WDC6502_t* cpu) {
    branch(cpu, get_flag(cpu, C));
}

void BEQ(WDC6502_t* cpu) {
    branch(cpu, get_flag(cpu, Z));
}

void BMI(WDC6502_t* cpu) {
    branch(cpu, get_flag(cpu, N));
}

void BNE(WDC6502_t* cpu) {
    branch(cpu, !get_flag(cpu, Z));
}

void BPL(WDC6502_t* cpu) {
    branch(cpu, !get_flag(cpu, N));
}

void BVC(WDC6502_t* cpu) {
    branch(cpu, !get_flag(cpu, V));
}

void BVS(WDC6502_t* cpu) {
    branch(cpu, get_flag(cpu, V));
}

void BIT(WDC6502_t* cpu) {
    uint8_t operand = read_b(cpu, cpu->operand_address);
    uint8_t result = cpu->registers.A & operand;
    set_flag(cpu, V, (operand >> 6) & 1);
    set_flag(cpu, N, operand >> 7);
    set_flag(cpu, Z, result == 0);
}

void BRK(WDC6502_t* cpu) {
    enter_interrupt(cpu, true);
}

void CLC(WDC6502_t* cpu) {
    cycle(cpu);
    set_flag(cpu, C, false);
}

void CLD(WDC6502_t* cpu) {
    cycle(cpu);
    set_flag(cpu, D, false);
}

void CLI(WDC6502_t* cpu) {
    cycle(cpu);
    set_flag(cpu, I, false);
}

void CLV(WDC6502_t* cpu) {
    cycle(cpu);
    set_flag(cpu, V, false);
}

void CMP(WDC6502_t* cpu) {
    compare(cpu, cpu->registers.A);
}

void CPY(WDC6502_t* cpu) {
    compare(cpu, cpu->registers.Y);
}

void CPX(WDC6502_t* cpu) {
    compare(cpu, cpu->registers.X);
}

void DEC(WDC6502_t* cpu) {
    uint8_t operand = read_b(cpu, cpu->operand_address);
    write_b(cpu, cpu->operand_address, operand);
    operand--;
    write_b(cpu, cpu->operand_address, operand);
    set_nz(cpu, operand);
}

void DEX(WDC6502_t* cpu) {
    cycle(cpu);
    cpu->registers.X--;
    set_nz(cpu, cpu->registers.X);
}

void DEY(WDC6502_t* cpu) {
    cycle(cpu);
    cpu->registers.Y--;
    set_nz(cpu, cpu->registers.Y);
}

static inline void EOR(WDC6502_t* cpu) {
    uint8_t operand = read_b(cpu, cpu->operand_address);
    cpu->registers.A ^= operand;
    set_nz(cpu, cpu->registers.A);
}

void INC(WDC6502_t* cpu) {
    uint8_t operand = read_b(cpu, cpu->operand_address);
    write_b(cpu, cpu->operand_address, operand);
    operand++;
    write_b(cpu, cpu->operand_address, operand);
    set_nz(cpu, operand);
}

void INX(WDC6502_t* cpu) {
    cycle(cpu);
    cpu->registers.X++;
    set_nz(cpu, cpu->registers.X);
}

void INY(WDC6502_t* cpu) {
    cycle(cpu);
    cpu->registers.Y++;
    set_nz(cpu, cpu->registers.Y);
}

void JMP(WDC6502_t* cpu) {
    cpu->registers.PC = cpu->operand_address;
}

void JSR(WDC6502_t* cpu) {
    cpu->registers.PC--;
    cycle(cpu);
    push_stack(cpu, cpu->registers.PC >> 8);
    push_stack(cpu, cpu->registers.PC & 0xFF);
    cpu->registers.PC = cpu->operand_address;
}

void LDA(WDC6502_t* cpu) {
    load_register(cpu, &cpu->registers.A);
}

void LDX(WDC6502_t* cpu) {
    load_register(cpu, &cpu->registers.X);
}

void LDY(WDC6502_t* cpu) {
    load_register(cpu, &cpu->registers.Y);
}

void LSR_A(WDC6502_t* cpu) {
    bool c = cpu->registers.A & 0x1;
    cpu->registers.A >>= 1;
    set_flag(cpu, Z, cpu->registers.A == 0);
    set_flag(cpu, N, false);
    set_flag(cpu, C, c);
    cycle(cpu);
}

void LSR(WDC6502_t* cpu) {
    uint8_t operand = read_b(cpu, cpu->operand_address);
    bool c = operand & 0x1;
    write_b(cpu, cpu->operand_address, operand);
    operand >>= 1;
    write_b(cpu, cpu->operand_address, operand);
    set_flag(cpu, Z, operand == 0);
    set_flag(cpu, N, false);
    set_flag(cpu, C, c);
}

void NOP(WDC6502_t* cpu) {
    cycle(cpu);
}

void ORA(WDC6502_t* cpu) {
    uint8_t operand = read_b(cpu, cpu->operand_address);
    cpu->registers.A |= operand;
    set_nz(cpu, cpu->registers.A);
}

void PHA(WDC6502_t* cpu) {
    cycle(cpu);
    push_stack(cpu, cpu->registers.A);
}

void PHP(WDC6502_t* cpu) {
    cycle(cpu);
    push_stack(cpu, cpu->registers.P | (0b11 << 4));
}

void PLA(WDC6502_t* cpu) {
    cycle(cpu);
    cycle(cpu);
    cpu->registers.A = pull_stack(cpu);
    set_nz(cpu, cpu->registers.A);
}

void PLP(WDC6502_t* cpu) {
    cycle(cpu);
    cycle(cpu);
    cpu->registers.P = pull_stack(cpu);
}

void ROL_A(WDC6502_t* cpu) {
    bool c = cpu->registers.A >> 7;
    cpu->registers.A <<= 1;
    cpu->registers.A |= get_flag(cpu, C);
    set_nz(cpu, cpu->registers.A);
    set_flag(cpu, C, c);
    cycle(cpu);
}

void ROL(WDC6502_t* cpu) {
    uint8_t operand = read_b(cpu, cpu->operand_address);
    bool c = operand >> 7;
    write_b(cpu, cpu->operand_address, operand);
    operand <<= 1;
    operand |= get_flag(cpu, C);
    write_b(cpu, cpu->operand_address, operand);
    set_nz(cpu, operand);
    set_flag(cpu, C, c);
}

void ROR_A(WDC6502_t* cpu) {
    bool c = cpu->registers.A & 0x1;
    cpu->registers.A >>= 1;
    cpu->registers.A |= (get_flag(cpu, C) << 7);
    set_nz(cpu, cpu->registers.A);
    set_flag(cpu, C, c);
    cycle(cpu);
}

void ROR(WDC6502_t* cpu) {
    uint8_t operand = read_b(cpu, cpu->operand_address);
    bool c = operand & 0x1;
    write_b(cpu, cpu->operand_address, operand);
    operand >>= 1;
    operand |= (get_flag(cpu, C) << 7);
    write_b(cpu, cpu->operand_address, operand);
    set_nz(cpu, operand);
    set_flag(cpu, C, c);
}

void RTI(WDC6502_t* cpu) {
    uint16_t pc;
    cycle(cpu);
    cpu->registers.P = pull_stack(cpu);
    cpu->should_take_irq = !get_flag(cpu, I);
    cycle(cpu);
    pc = pull_stack(cpu);
    pc |= pull_stack(cpu) << 8;
    cpu->registers.PC = pc;
}

void RTS(WDC6502_t* cpu) {
    uint16_t pc;
    cycle(cpu);
    pc = pull_stack(cpu);
    cycle(cpu);
    pc |= pull_stack(cpu) << 8;
    cpu->registers.PC = pc + 1;
    cycle(cpu);
}

void SBC(WDC6502_t* cpu) {
    uint8_t operand = read_b(cpu, cpu->operand_address);
    int result = cpu->registers.A - operand - !get_flag(cpu, C);
    set_flag(cpu, C, !(result < 0x00));
    set_flag(cpu, V, (result ^ cpu->registers.A) & (result ^ ~operand) & 0x80);
    set_nz(cpu, result);
    cpu->registers.A = result & 0xFFFF;
}

void SEC(WDC6502_t* cpu) {
    cycle(cpu);
    set_flag(cpu, C, true);
}
void SED(WDC6502_t* cpu) {
    cycle(cpu);
    set_flag(cpu, D, true);
}

void SEI(WDC6502_t* cpu) {
    cycle(cpu);
    set_flag(cpu, I, true);
}

void STA(WDC6502_t* cpu) {
    write_b(cpu, cpu->operand_address, cpu->registers.A);
}

void STX(WDC6502_t* cpu) {
    write_b(cpu, cpu->operand_address, cpu->registers.X);
}

void STY(WDC6502_t* cpu) {
    write_b(cpu, cpu->operand_address, cpu->registers.Y);
}

void TAX(WDC6502_t* cpu) {
    transfer_register(cpu, &cpu->registers.A, &cpu->registers.X);
}

void TAY(WDC6502_t* cpu) {
    transfer_register(cpu, &cpu->registers.A, &cpu->registers.Y);
}

void TSX(WDC6502_t* cpu) {
    transfer_register(cpu, &cpu->registers.S, &cpu->registers.X);
}

void TXA(WDC6502_t* cpu) {
    transfer_register(cpu, &cpu->registers.X, &cpu->registers.A);
}

void TXS(WDC6502_t* cpu) {
    cycle(cpu);
    cpu->registers.S = cpu->registers.X;
}

void TYA(WDC6502_t* cpu) {
    transfer_register(cpu, &cpu->registers.Y, &cpu->registers.A);
}

#undef INSTR
#undef INSTR_IMPL