#pragma once

#include <stdint.h>
#include "Debug.h"

constexpr auto MEMORY_SIZE_BYTES	= 65536;	// Size of memory (64KB)

constexpr auto LOC_BOOT_ROM			= 0x0000;	// Location in memory where the boot ROM is held
constexpr auto LOC_ROM_BANK_00		= 0x0000;	// Location in memory of ROM bank 00
constexpr auto LOC_ROM_BANK_NN		= 0x4000;	// Location in memory of ROM bank 01~NN
constexpr auto LOC_VRAM				= 0x8000;	// Location in memory of VRAM
constexpr auto LOC_CART_RAM			= 0xA000;	// Location in memory of cartridge RAM
constexpr auto LOC_WRAM_0			= 0xC000;	// Location in memory of working RAM bank 0
constexpr auto LOC_WRAM_N			= 0xD000;	// Location in memory of WRAM for bank 1~N
constexpr auto LOC_ECHO_RAM			= 0xE000;	// Location in memory of echo RAM
constexpr auto LOC_OAM				= 0xFE00;	// Location in memory of OAM (Object Attribute Memory)
constexpr auto LOC_UNUSED			= 0xFEA0;	// Location in memory of unused memory
constexpr auto LOC_IO_REG			= 0xFF00;	// Location in memory of IO registers
constexpr auto LOC_HRAM				= 0xFF80;	// Location in memory of high RAM
constexpr auto LOC_INT_EN_REG		= 0xFFFF;	// Location in memory of the "interrupt enabled" register


class MMU {
public:
	MMU() {
		Reset();
	}

	void Reset();

	// Writes 8-bit value to location at 16-bit address in memory
	void Write(uint16_t address, uint8_t value);

	// Reads value from 16-bit address in memory
	uint8_t Read(uint16_t address);

private:
	uint8_t memory[MEMORY_SIZE_BYTES];
};