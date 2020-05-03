#include "CPU.h"

void CPU::Reset() {
	Debug::Log("Resetting CPU");

	// Clear the registers
	for (int i = 0; i < NUM_REGISTERS; i++)
		registers[i] = 0;

	// Reset the stack pointer
	sp = 0;

	// Reset the program counter
	pc = 0;
}

// Executes the instruction in memory at the address specified by the program counter
uint8_t CPU::Execute(bool printExecution) {
	// Read value from memory at the address specified by the program counter
	uint8_t opCode = mmu->Read(pc);

	// Execute the instruction
	(this->*(instructions[opCode].execute))();

	if (printExecution) {
		Debug::PrintInstruction(pc, opCode, instructions[opCode].mnemonic);
	}

	// Increment program counter
	pc += instructions[opCode].length;

	return instructions[opCode].cycles;
}

// No operation
void CPU::NOP() {

}

/*
=========================================================================================
	16-Bit Load Commands

	Loads the 16-bit immediate value NN into the register BC, DE, HL, or SP
=========================================================================================
*/

void CPU::LD_BC_NN() {
	SetReg(REG_B, mmu->Read(pc + 1));
	SetReg(REG_C, mmu->Read(pc + 2));
}

void CPU::LD_DE_NN() {
	SetReg(REG_D, mmu->Read(pc + 1));
	SetReg(REG_E, mmu->Read(pc + 2));
}

void CPU::LD_HL_NN() {
	SetReg(REG_H, mmu->Read(pc + 1));
	SetReg(REG_L, mmu->Read(pc + 2));
}

void CPU::LD_SP_NN() {
	SetSP(mmu->Read(pc + 1), mmu->Read(pc + 2));
}

void CPU::LD_NN_SP() {
	uint8_t addr1 = mmu->Read(pc + 1);
	uint8_t addr2 = mmu->Read(pc + 2);
	uint16_t addr = (addr1 << 8) | addr2;

	uint8_t spLow = sp & 0x00FF;
	uint8_t spHigh = sp & 0xFF00;

	mmu->Write(addr, spLow);
	mmu->Write(addr + 1, spHigh);
}

/*
=========================================================================================
	8-Bit Load Commands
=========================================================================================
*/

// Writes the contents of register A into memory at the address BC
void CPU::LD_BC_A() {
	mmu->Write(GetReg(REG_BC), GetReg(REG_A));
}

// Loads immediate 8-bit value N into register B
void CPU::LD_B_N() {
	uint8_t n = mmu->Read(pc + 1);
	SetReg(REG_B, n);
}

// Loads immediate 8-bit value N into register C
void CPU::LD_C_N() {
	uint8_t n = mmu->Read(pc + 1);
	SetReg(REG_C, n);
}

// Loads contents in memory at register-pair address BC into register A
void CPU::LD_A_BC() {
	uint16_t addr = GetReg(REG_BC);
	SetReg(REG_A, mmu->Read(addr));
}

/*
=========================================================================================
	8-bit arithmetic/logic commands
=========================================================================================
*/
void CPU::INC_B() {
	uint8_t regB = GetReg(REG_B);
	SetReg(REG_B, regB + 1);

	SetFlag(FLAG_Z, (GetReg(REG_B) == 0));
	SetFlag(FLAG_N, 0);
	SetFlag(FLAG_H, CheckHalfCarry(regB, 1));
}

// Decrements register B by one
void CPU::DEC_B() {
	uint8_t regB = GetReg(REG_B);
	SetReg(REG_B, regB - 1);

	SetFlag(FLAG_Z, (GetReg(REG_B) == 0));
	SetFlag(FLAG_N, 1);
	SetFlag(FLAG_H, CheckHalfCarry(regB, 1, false));
}

void CPU::INC_C() {
	uint8_t regC = GetReg(REG_C);
	SetReg(REG_C, regC + 1);

	SetFlag(FLAG_Z, (GetReg(REG_C) == 0));
	SetFlag(FLAG_N, 0);
	SetFlag(FLAG_H, CheckHalfCarry(regC, 1));
}

void CPU::DEC_C() {
	uint8_t regC = GetReg(REG_C);
	SetReg(REG_C, regC - 1);

	SetFlag(FLAG_Z, (GetReg(REG_C) == 0));
	SetFlag(FLAG_N, 1);
	SetFlag(FLAG_H, CheckHalfCarry(regC, 1, false));
}

/*
=========================================================================================
	16-bit arithmetic/logic commands
=========================================================================================
*/

// Increments register pair BC by 1
void CPU::INC_BC() {
	SetReg(REG_BC, GetReg(REG_BC) + 1);
}

// Decrements register pair BC by 1
void CPU::DEC_BC() {
	SetReg(REG_BC, GetReg(REG_BC) - 1);
}

// Adds the contents of the register pair BC to the register pair HL
void CPU::ADD_HL_BC() {
	uint16_t regBC = GetReg(REG_BC);
	uint16_t regHL = GetReg(REG_HL);
	SetReg(REG_HL, regBC + regHL);

	SetFlag(FLAG_N, 0);
	SetFlag(FLAG_H, CheckHalfCarry(regBC, regHL));
	SetFlag(FLAG_CY, CheckCarry(regBC, regHL));
}

/*
=========================================================================================
	Rotate and shift commands
=========================================================================================
*/

// Rotates bits of register A by one to the left. Sets carry if bit 7 was 1 and sets bit 0 to former bit 7 value
void CPU::RLCA() {
	uint8_t regA = GetReg(REG_A);
	uint8_t regAShift = regA << 1;

	if ((regA & 0x80) == 0x80)
		regAShift |= 1;

	SetReg(REG_A, regAShift);

	SetFlag(FLAG_H, 0);
	SetFlag(FLAG_N, 0);
	SetFlag(FLAG_Z, 0);
	SetFlag(FLAG_CY, ((regA & 0x80) == 0x80));
}

// Rotates bits of register A by one ot the right. Sets carry if bit 0 was 1 and sets bit 7 to former bit 0 value
void CPU::RRCA() {
	uint8_t regA = GetReg(REG_A);
	uint8_t regAShift = regA >> 1;

	if ((regA & 0x01) == 0x01)
		regAShift |= 0x80;

	SetReg(REG_A, regAShift);


	SetFlag(FLAG_H, 0);
	SetFlag(FLAG_N, 0);
	SetFlag(FLAG_Z, 0);
	SetFlag(FLAG_CY, ((regA & 0x01) == 0x01));
}

