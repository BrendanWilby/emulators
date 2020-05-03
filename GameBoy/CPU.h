#pragma once

#include <stdint.h>

#include "MMU.h"
#include "Debug.h"

typedef bool Byte2Part;

constexpr auto BYTE_HIGH = 1;
constexpr auto BYTE_LOW = 0;
constexpr auto NUM_REGISTERS = 8;
constexpr auto NUM_INSTRUCTIONS = 256;

class CPU {

private:

	enum FlagTypes {
		FLAG_Z = (1 << 7),		// Zero flag
		FLAG_N = (1 << 6),		// Add/subtract flag
		FLAG_H = (1 << 5),		// Half-carry flag
		FLAG_CY = (1 << 4)		// Carry flag
	};

	enum Registers8Bit {
		REG_A = 0,
		REG_B = 1,
		REG_C = 2,
		REG_D = 3,
		REG_E = 4,
		REG_H = 5,
		REG_L = 6,
		REG_F = 7
	};

	enum Registers16Bit {
		REG_AF,
		REG_BC,
		REG_DE,
		REG_HL
	};

	uint8_t registers[NUM_REGISTERS];
	
	uint16_t sp;
	uint16_t pc;

	MMU *mmu;

public:
	CPU(MMU *mmu) : mmu(mmu) {
		Reset();
	}

	void Reset();

	// Execute a CPU instruction
	uint8_t Execute(bool printExecution);

private:
	// Returns the value of the 8-bit register "reg"
	uint8_t GetReg(Registers8Bit reg) { return registers[reg]; }

	// Returns the value of a 16-bit register
	uint16_t GetReg(Registers16Bit reg) {
		uint16_t regValue = 0;

		if(reg == REG_AF)
			regValue = (GetReg(REG_A) << 8) | GetReg(REG_F);
		if(reg == REG_BC)
			regValue = (GetReg(REG_B) << 8) | GetReg(REG_C);
		if(reg == REG_DE)
			regValue = (GetReg(REG_D) << 8) | GetReg(REG_E);
		if(reg == REG_HL)
			regValue = (GetReg(REG_H) << 8) | GetReg(REG_L);

		return regValue;
	}

	// Sets the value of an 8-bit register
	void SetReg(Registers8Bit reg, uint8_t value) { registers[reg] = value; }

	// Sets the value of an 8-bit register to either the high/low byte of a 16-bit value
	void SetReg(Registers8Bit reg, uint16_t value, Byte2Part bytePart) {
		if (BYTE_HIGH)
			SetReg(reg, (value & 0xFF00) >> 8);
		else
			SetReg(reg, value & 0x00FF);
	}

	// Sets a value to a 16-bit register
	void SetReg(Registers16Bit reg, uint16_t value) {
		switch (reg) {
			case REG_AF:
				SetReg(REG_A, value, BYTE_HIGH);
				SetReg(REG_F, value, BYTE_LOW);
				break;
			case REG_BC:
				SetReg(REG_B, value, BYTE_HIGH);
				SetReg(REG_C, value, BYTE_LOW);
				break;
			case REG_DE:
				SetReg(REG_D, value, BYTE_HIGH);
				SetReg(REG_E, value, BYTE_LOW);
				break;
			case REG_HL:
				SetReg(REG_H, value, BYTE_HIGH);
				SetReg(REG_L, value, BYTE_LOW);
				break;
		}
	}

	// Set the stack pointer with two 8-bit values
	void SetSP(uint8_t high, uint8_t low) {
		sp = (high << 8) | low;
	}

	bool IsFlagSet(FlagTypes flag) {
		return (GetReg(REG_F) & flag);
	}

	void SetFlag(FlagTypes flag, bool on) {
		if (on)
			SetReg(REG_F, GetReg(REG_F) | flag);
		else
			SetReg(REG_F, GetReg(REG_F) & ~flag);
	}

	bool CheckHalfCarry(uint8_t a, uint8_t b, bool add = true) {
		if(add)
			return ((((a & 0x0F) + (b & 0x0F)) & 0x10) == 0x10);
		else
			return (((a & 0x0F) - (b & 0x0F)) < 0);

	}

	bool CheckHalfCarry(uint16_t a, uint16_t b, bool add = true) {
		if (add)
			return ((((a & 0x0F) + (b & 0x0F)) & 0x10) == 0x10);
		else
			return (((a & 0x0F) - (b & 0x0F)) < 0);

	}

	bool CheckCarry(uint8_t a, uint8_t b) {
		return ((a + b) > 0xFF);
	}

	bool CheckCarry(uint16_t a, uint16_t b) {
		return ((a + b) > 0xFFFF);
	}

private:
	struct CPUInstructions {
		const char *mnemonic;
		uint8_t length;
		uint8_t cycles;
		void(CPU::*execute)();
	};

	const CPUInstructions instructions[NUM_INSTRUCTIONS] = {
		{ "NOP", 1, 4, &CPU::NOP },
		{ "LD BC, d16", 3, 12, &CPU::LD_BC_NN},
		{ "LD (BC), A", 1, 8, &CPU::LD_BC_A},
		{ "INC BC", 1, 8, &CPU::INC_BC},
		{ "INC B", 1, 4, &CPU::INC_B},
		{ "DEC B", 1, 4, &CPU::DEC_B},
		{ "LD B, d8", 2, 8, &CPU::LD_B_N},
		{ "RLCA", 1, 4, &CPU::RLCA},
		{ "LD (a16), SP", 3, 20, &CPU::LD_NN_SP},
		{ "ADD HL, BC", 1, 8, &CPU::ADD_HL_BC },
		{ "LD A, (BC)", 1, 8, &CPU::LD_A_BC },
		{ "DEC BC", 1, 8, &CPU::DEC_BC },
		{ "INC C", 1, 4, &CPU::INC_C },
		{ "DEC C", 1, 4, &CPU::DEC_C },
		{ "LD C, d8", 2, 8, &CPU::LD_C_N },
		{ "RRCA", 1, 4, &CPU::RRCA }
	};

	void NOP();

	// 16-bit Loads
	void LD_BC_NN();
	void LD_DE_NN();
	void LD_HL_NN();
	void LD_SP_NN();
	void LD_NN_SP();

	// 8-bit loads
	void LD_BC_A();
	void LD_B_N();
	void LD_C_N();
	void LD_A_BC();

	// 8-bit arithmetic/logic commands
	void INC_B();
	void DEC_B();
	void INC_C();
	void DEC_C();

	// 16-bit arithmetic/logic commands
	void INC_BC();
	void DEC_BC();
	void ADD_HL_BC();

	// Rotate and shift commands
	void RLCA();
	void RRCA();
};