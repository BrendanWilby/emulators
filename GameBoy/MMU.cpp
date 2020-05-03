#include "MMU.h"

void MMU::Reset() {
	Debug::Log("Resetting MMU");

	// Wipe all memory
	for (int i = 0; i < MEMORY_SIZE_BYTES; i++)
		memory[i] = 0;

	memory[0] = 0x01;

	// Load ROMs here
}

void MMU::Write(uint16_t address, uint8_t value) {
	memory[address] = value;
}

uint8_t MMU::Read(uint16_t address) {
	return memory[address];
}