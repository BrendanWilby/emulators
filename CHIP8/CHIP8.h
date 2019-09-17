///////
//http://devernay.free.fr/hacks/chip8/C8TECH10.HTM#8xy6

#pragma once
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;

#define WIDTH 64
#define HEIGHT 32

u8 fontset[80] = {
	0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
	0x20, 0x60, 0x20, 0x20, 0x70, // 1
	0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
	0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
	0x90, 0x90, 0xF0, 0x10, 0x10, // 4
	0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
	0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
	0xF0, 0x10, 0x20, 0x40, 0x40, // 7
	0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
	0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
	0xF0, 0x90, 0xF0, 0x90, 0x90, // A
	0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
	0xF0, 0x80, 0x80, 0x80, 0xF0, // C
	0xE0, 0x90, 0x90, 0x90, 0xE0, // D
	0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
	0xF0, 0x80, 0xF0, 0x80, 0x80  // F
};

typedef struct{
    u8 V[15];       // Registers
    u8 VF;          // VF Register, isn't used for normal registers. Used as carry flag in addition, no-borrow flag in subtraction and set upon pixel collision in draw instructions.
    u8 delayTimer;	// Decremented at a rate of 60Hz if non-zero.
    u8 soundTimer;	// Decremented at a rate of 60Hz if non-zero
    u16 I;          // Address register, used to store mmemory addresses
}CHIP8Registers;

typedef struct{
    u16 sp;						// Stack pointer - the memory address of the instruction at the top of the stack
    u16 pc;						// Program counter - the memory address of the current opcode being executed
	bool playSound;				// Called while the sound timer is > 0
	bool drawFlag;				// Called when the screen is cleared and when a sprite needs to be drawn
    int romSize;				// Size of the ROM in bytes
    u8 keys[16];				// Holds the states of each key - 1 for active, 0 for inactive
    u8 memory[4096];			// 4096 B (4KB) of RAM
    u8 screen[HEIGHT][WIDTH];	// Screen pixel buffer
    u16 stack[16];
    CHIP8Registers registers;
}CHIP8;

///////////////////////////////////////////////////////
//              HELPER FUNCTIONS
///////////////////////////////////////////////////////

void ClearScreen(CHIP8 *chip){
	for (int y = 0; y < HEIGHT; y++) {
		for (int x = 0; x < WIDTH; x++) {
			chip->screen[y][x] = 0;
		}
	}
}

void Write(CHIP8 *chip, u16 addr, u8 data){
    chip->memory[addr] = data;
}

u8 Read(CHIP8 *chip, u16 addr){
    return chip->memory[addr];
}

// Opens ROM at "path" and returns its contents.
// The argument "romSize" is used as a callback to store the size of the rom, which can then later be stored in the chip when written to memory
static unsigned char* LoadROM(CHIP8 *chip, char *path, int *romSize){
    FILE *rom = fopen(path, "rb");

    if(rom == NULL){
        printf("Error: Could not open file at %s.\n", path);
        exit(1);
    }

    fseek(rom, 0L, SEEK_END);
    *romSize = ftell(rom);
    fseek(rom, 0L, SEEK_SET);

    unsigned char* buffer = (unsigned char*)malloc(*romSize);
    fread(buffer, 1, *romSize, rom);
    fclose(rom);

    return buffer;
}

////////////////////////////////////////////////////////
//          DISASSEMBLER

// Translates the current opcode and prints to the console its Assembly like instruction

void DisassembleOpcode(u16 pc, u8 opcodeHigh, u8 opcodeLow) {
	printf("%04x \t %02x%02x", pc, opcodeHigh, opcodeLow);

	switch (opcodeHigh >> 4) {
		case 0x00:
			switch (opcodeLow) {
				case 0xE0:
					printf("\t CLS");
					break;
				case 0xEE:
					printf("\t RET");
					break;
				default:
					printf("\t SYS %01x%02x", (opcodeHigh & 0x0F), opcodeLow);
					break;
			}
			break;

			case 0x01:
				printf("\t JP %01x%02x", (opcodeHigh & 0x0F), opcodeLow);
				break;

			case 0x02:
				printf("\t CALL %01x%02x", (opcodeHigh & 0x0F), opcodeLow);
				break;

			case 0x03:
				printf("\t SE V%01x", (opcodeHigh & 0x0F));
				break;

			case 0x04:
				printf("\t SNE V%01x", (opcodeHigh & 0x0F));
				break;

			case 0x05:
				printf("\t SE V%01x, V%01x", (opcodeHigh & 0x0F), (opcodeLow & 0xF0) >> 4);
				break;

			case 0x06:
				printf("\t LD V%01x", (opcodeHigh & 0x0F));
				break;

			case 0x07:
				printf("\t ADD V%01x", (opcodeHigh & 0x0F));
				break;

			case 0x08:
				switch (opcodeLow & 0x0F) {
				case 0x00:
					printf("\t LD V%01x, V%01x", (opcodeHigh & 0x0F), (opcodeLow & 0xF0));
					break;
				case 0x01:
					printf("\t OR V%01x, V%01x", (opcodeHigh & 0x0F), (opcodeLow & 0xF0));
					break;
				case 0x02:
					printf("\t AND V%01x, V%01x", (opcodeHigh & 0x0F), (opcodeLow & 0xF0));
					break;
				case 0x03:
					printf("\t XOR V%01x, V%01x", (opcodeHigh & 0x0F), (opcodeLow & 0xF0));
					break;
				case 0x04:
					printf("\t ADD V%01x, V%01x", (opcodeHigh & 0x0F), (opcodeLow & 0xF0));
					break;
				case 0x05:
					printf("\t SUB V%01x, V%01x", (opcodeHigh & 0x0F), (opcodeLow & 0xF0));
					break;
				case 0x06:
					printf("\t SHR V%01x {, V%01x}", (opcodeHigh & 0x0F), (opcodeLow & 0xF0));
					break;
				case 0x07:
					printf("\t SUBN V%01x, V%01x", (opcodeHigh & 0x0F), (opcodeLow & 0xF0));
					break;
				case 0x0E:
					printf("\t SHL V%01x {, V%01x}", (opcodeHigh & 0x0F), (opcodeLow & 0xF0));
					break;
				}
				break;

			case 0x09:
				printf("\t SNE V%01x, V%01x", (opcodeHigh & 0x0F), (opcodeLow & 0xF0));
				break;

			case 0x0A:
				printf("\t LD I, %01x%02x", (opcodeHigh & 0x0F), opcodeLow);
				break;

			case 0x0B:
				printf("\t JP V0, %01x%02x", (opcodeHigh & 0x0F), opcodeLow);
				break;

			case 0x0C:
				printf("\t RND V%01x", (opcodeHigh & 0x0F));
				break;

			case 0x0D:
				printf("\t DRW V%01x, V%01x", (opcodeHigh & 0x0F), (opcodeLow & 0xF0) >> 4);
				break;

			case 0x0E:
				switch (opcodeLow) {
				case 0x9E:
					printf("\t SKP V%01x", (opcodeHigh & 0x0F));
					break;
				case 0xA1:
					printf("\t SKNP V%01x", (opcodeHigh & 0x0F));
					break;
				}
				break;

			case 0x0F:
				switch (opcodeLow) {
				case 0x07:
					printf("\t LD V%01x, DT", (opcodeHigh & 0x0F));
					break;

				case 0x0A:
					printf("\t LD V%01x, K", (opcodeHigh & 0x0F));
					break;

				case 0x15:
					printf("\t LD DT, V%01x", (opcodeHigh & 0x0F));
					break;

				case 0x18:
					printf("\t LD ST, V%01x", (opcodeHigh & 0x0F));
					break;

				case 0x1E:
					printf("\t ADD I, V%01x", (opcodeHigh & 0x0F));
					break;

				case 0x29:
					printf("\t LD F, V%01x", (opcodeHigh & 0x0F));
					break;

				case 0x33:
					printf("\t LD B, V%01x", (opcodeHigh & 0x0F));
					break;

				case 0x55:
					printf("\t LD [I], V%01x", (opcodeHigh & 0x0F));
					break;

				case 0x65:
					printf("\t LD V%01x, [I]", (opcodeHigh & 0x0F));
					break;
				}
				break;

		default:
			printf("\t Not implemented");
			break;
	}
	printf("\n");
}

// Used to translate every instruction in the ROM file located at "path"
// Prints the original file content, as well as its translation into the console
void DisassembleROM(CHIP8 *chip, char *path){
    int romSize;
    unsigned char* buffer = LoadROM(chip, path, &romSize);

    int pc = 0;

    printf("Addr \tOpcode \tInstruction\n");

	while (pc < romSize) {
		u8 opcodeHigh = buffer[pc];
		u8 opcodeLow = buffer[pc + 1];
		
		DisassembleOpcode(pc, opcodeHigh, opcodeLow);
		pc += 2;
	}

    free(buffer);
}

void ResetCHIP8(CHIP8 *chip){
    for(int i = 0; i < 4096; i++)
        chip->memory[i] = 0;
    
	// Programs are loaded into memory at 0x200 onwards
    chip->pc = 0x200;
    chip->sp = 0;
    chip->registers.I = 0;
    chip->registers.VF = 0;
    chip->romSize = 0;

    for (int i = 0; i < 16; i++){
        chip->registers.V[i] = 0;
        chip->keys[i] = 0;
        if(i < 15)
            chip->stack[i] = 0;
    }

    ClearScreen(chip);

	// Writes the fontset to memory. Some emulators start writing at 0x50 onwards, but this isn't compatible with some of the test roms.
	// Starting writing at 0x00 seems to work for both games and test roms, so I am writing at 0x00 here.
	for (int i = 0; i < 80; i++) {
		Write(chip, i, fontset[i]);
	}
}

// Loads the ROM at "path" into memory at memory address 0x200 onwards.
void LoadROMToMemory(CHIP8 *chip, char *path){
    int romSize;
    unsigned char *rom = LoadROM(chip, path, &romSize); 

    ResetCHIP8(chip);
    
    chip->romSize = romSize;

    for(int i = 0; i < romSize; i++){
        Write(chip, 0x200 + i, rom[i]);
    }

    free(rom);

    printf("Loaded game %s of size %dB.\n", path, romSize);
}

void EmulateCHIP8Cycle(CHIP8 *chip){
    u8 opcodeHigh = Read(chip, chip->pc);			// First byte of the opcode (leftmost)
    u8 opcodeLow = Read(chip, chip->pc + 1);		// Last byte of the opcode (rightmost)
    u16 opcode = (opcodeHigh << 8) | opcodeLow;		// Complete 2 bit opcode
	u8 x = opcodeHigh & 0x0F;						// Last nibble of the first byte of the opcode (or in other words, the last nibble of opcodeHigh)
	u8 y = (opcodeLow & 0xF0) >> 4;					// First nibble of the second byte of the opcode (first nibble of opcodeLow)

	DisassembleOpcode(chip->pc, opcodeHigh, opcodeLow);

    // Shift high opcode 4 bits to the right
    // 12 (0001 0010) would become -> 0000 0001 = 0x01
    // Can then compare with 0x01, 0x02, etc...
    switch(opcodeHigh >> 4){
        
        case 0x00:
            switch(opcodeLow){
                //  =======================================================================================
                //          00E0 - CLS
                //  =======================================================================================
                //  Clear the display
                //  =======================================================================================
                case 0xE0:
                    ClearScreen(chip);
                    chip->pc += 2;
					chip->drawFlag = true;
                    break;

                //  =======================================================================================
                //          00EE - RET
                //  =======================================================================================
                //  Return from a subroutine
                //  The interpreter sets the PC to the address at the top of the stack,
                //  then subtracts 1 from the stack pointer.
                //  =======================================================================================
                case 0xEE:
                    chip->pc = chip->stack[chip->sp];
                    chip->sp -= 1;
                    chip->pc += 2;

                    break;

                //  =======================================================================================
                //          0NNN - SYS addr
                //  =======================================================================================
                //  Jump to a machine code routine at NNN
                //  This instruction is only used on old computers on which CHIP-8 was originally implemented.
                //  IGNORED in this emulation.
                //  =======================================================================================
                default:

                    break;
            }
            break;

        //  =======================================================================================
        //          1NNN - JP addr
        //  =======================================================================================
        //  Jump to location addr
        //  Interpreter sets program counter to NNN
        //  =======================================================================================
        case 0x01:
            chip->pc = opcode & 0x0FFF;
            break;

        //  =======================================================================================
        //          2NNN - JP addr
        //  =======================================================================================
        //  Call subroutine at NNN
        //  Interpreter increments the stack pointer, then puts the current PC
        //  on the top of the stack. The PC is then set to NNN.
        //  =======================================================================================
        case 0x02:
			chip->sp++;
            chip->stack[chip->sp] = chip->pc;
            chip->pc = opcode & 0x0FFF;
            break;

        //  =======================================================================================
        //          3xkk - SE Vx byte
        //  =======================================================================================
        //  Skip next instruction if Vx = kk
        //  Interpreter compares register Vx to kk, and if they are equal,
        //  increments the PC by 2
        //  =======================================================================================
        case 0x03:
            if(chip->registers.V[x] == opcodeLow)
                chip->pc += 4;
            else
                chip->pc += 2;

            break;

        //  =======================================================================================
        //          4xkk - SNE Vx byte
        //  =======================================================================================
        //  Skip next instruction if Vx != kk
        //  Interpreter compares register Vx to kk, and if they are not equal,
        // increments the PC by 2
        //  =======================================================================================
        case 0x04:
            if(chip->registers.V[x] != opcodeLow)
                chip->pc += 4;
            else
                chip->pc += 2;
            
            break;

        //  =======================================================================================
        //          5xy0 - SE Vx, Vy
        //  =======================================================================================
        //  Skip next instruction if Vx = Vy
        //  The interpreter compares register Vx to register Vy, and if they are equal,
        //  increments the PC by 2
        //  =======================================================================================
        case 0x05:
            if(chip->registers.V[x] == chip->registers.V[y])
                chip->pc += 4;
            else
                chip->pc += 2;
            break;

        //  ==========================================================
        //          6xkk - LD Vx, byte
        //  ==========================================================
        //  Set Vx = kk
        //  The interpreter puts the value kk into the register Vx
        //  ==========================================================
        case 0x06:
            chip->registers.V[x] = opcodeLow;
            chip->pc += 2;
            break;

        //  =======================================================================================
        //          7xkk - ADD Vx, byte
        //  =======================================================================================
        //  Set Vx = Vx + kk
        //  Adds the value kk to the value of register Vx, then stores the result in Vx
        //  =======================================================================================
        case 0x07:
            chip->registers.V[x] += opcodeLow;
            chip->pc += 2;
            break;

        case 0x08:
            switch(opcodeLow & 0x0F){

                //  ==========================================================
                //          8xy0 - LD Vx, Vy
                //  ==========================================================
                //  Set Vx = Vy
                //  Stores the value of register Vy in register Vx
                //  ==========================================================
                case 0x00:
                    chip->registers.V[x] = chip->registers.V[y];
                    chip->pc += 2;
                    break;

                //  =======================================================================================
                //          8xy1 - OR Vx, Vy
                //  =======================================================================================
                //  Set Vx = Vy OR Vy
                //  Performs a bitwise OR on the values of Vx and Vy, then stores the result in Vx.
                //  A bitwise OR compares the corrseponding bits from two values, and if either bit is 1,
                //  then the same bit in the result is also 1.
                //  Otherwise, it is 0.
                //  =======================================================================================
                case 0x01:
                    chip->registers.V[x] |= chip->registers.V[y];
                    chip->pc += 2;
                    break;

                //  =======================================================================================
                //          8xy2 - AND Vx, Vy
                //  =======================================================================================
                //  Set Vx = Vy AND Vy
                //  Performs a bitwise AND on the values of Vx and Vy, then stores the result in Vx.
                //  A bitwise AND compares the corrseponding bits from two values, and if both bits are 1,
                //  then the same bit in the result is also 1.
                //  Otherwise, it is 0.
                //  =======================================================================================
                case 0x02:
                    chip->registers.V[x] &= chip->registers.V[y];
                    chip->pc += 2;
                    break;

                //  ====================================================================================================================
                //          8xy3 - XOR Vx, Vy
                //  ====================================================================================================================
                //  Set Vx = Vy XOR Vy
                //  Performs a bitwise exclusive OR on the values of Vx and Vy, then stores the result in Vx.
                //  An exclusive OR compares the corrseponding bits from two values, and if the bits are not both the same,
                //  then the corresponding bit in the result is set to 1.
                //  Otherwise, it is 0.
                //  ====================================================================================================================
                case 0x03:
                    chip->registers.V[x] ^= chip->registers.V[y];
                    chip->pc += 2;
                    break;

                //  =======================================================================================
                //          8xy4 - ADD Vx, Vy
                //  =======================================================================================
                //  Set Vx = Vx + Vy, set VF = carry
                //  The values of Vx and Vy are added together.
                //  If the result is greater than 8 bits (i.e., > 255,) VF is set to 1, otherwise 0.
                //  Only the lowest 8 bits of the result are kept, and stored in Vx.
                //  =======================================================================================
                case 0x04:
                    if ((chip->registers.V[x] + chip->registers.V[y]) > 0xFF)
                        chip->registers.VF = 1;
                    else
                        chip->registers.VF = 0;

					chip->registers.V[x] += chip->registers.V[y];
                    chip->pc += 2;
                    break;

                //  =======================================================================================
                //                  8xy5 - SUB Vx, Vy
                //  =======================================================================================
                //  Set Vx = Vx - Vy, set VF = NOT borrow
                //  If Vx > Vy, then VF is set to 1, otherwise 0.
                //  Then Vy is subtracted from Vx, and the results stored in Vx.
                //  =======================================================================================
                case 0x05:
                    if(chip->registers.V[x] > chip->registers.V[y])
                        chip->registers.VF = 1;
                    else
                        chip->registers.VF = 0;
                    
                    chip->registers.V[x] -= chip->registers.V[y];
					chip->pc += 2;
                    break;

                //  =======================================================================================
                //                  8xy6 - SHR Vx {, Vy}
                //  =======================================================================================
                //  Set Vx = Vx SHR 1
                //  If the least-significant bit of Vx is 1, then VF is set to 1, otherwise 0.
                //  Then Vx is divided by 2.
                //  =======================================================================================
                case 0x06:
                    chip->registers.VF = (chip->registers.V[x] & 0x01);
                    chip->registers.V[x] >>= 1;
                    chip->pc += 2;
                    break;

                //  =======================================================================================
                //                  8xy7 - SUBN Vx, Vy
                //  =======================================================================================
                //  Set Vx = Vy - Vx, set VF = NOT borrow
                //  If Vy > Vx, then VF is set to 1, otherwise 0.
                //  Then Vx is subtracted from Vy, and the results stored in Vx.
                //  =======================================================================================
                case 0x07:
                    if(chip->registers.V[y] > chip->registers.V[x])
                        chip->registers.VF = 1;
                    else
                        chip->registers.VF = 0;

                    chip->registers.V[x] = chip->registers.V[y] - chip->registers.V[x];
                    chip->pc += 2;

                    break;

                //  =======================================================================================
                //                  8xyE - SHL Vx {, Vy}
                //  =======================================================================================
                //  Set Vx = Vx SHL 1
                //  If the most-significant bit of Vx is 1, then VF is set to 1, otherwise to 0.
                //  Then Vx is multiplied by 2.
                //  =======================================================================================
                case 0x0E:
                    chip->registers.VF = (chip->registers.V[x] >> 7);
                    chip->registers.V[x] <<= 1;
                    chip->pc += 2;
                    break;
            }
            break;

        //  ==========================================================
        //                  9xy0 - SNE Vx, Vy
        //  ==========================================================
        //  Skip next instruction if Vx != Vy
        //  The values of Vx and Vy are compared, and if they are not equal,
        //  the PC is increased by 2
        //  ==========================================================
        case 0x09:
            if (chip->registers.V[x] != chip->registers.V[y])
                chip->pc += 4;
            else
                chip->pc += 2;
            break;

        //  ==========================================================
        //                  ANNN - LD I, addr
        //  ==========================================================
        //  Set I = NNN
        //  The value of register I is set to NNN
        //  ==========================================================
        case 0x0A:
            chip->registers.I = opcode & 0x0FFF;
            chip->pc += 2;
            break;

        //  ==========================================================
        //                  BNNN - JP V0, addr
        //  ==========================================================
        //  Jump to location NNN + V0
        //  The PC is set to NNN plus the value of V0
        //  ==========================================================
        case 0x0B:
            chip->pc = (opcode & 0x0FFF) + chip->registers.V[0];
            break;

        //  =======================================================================================
        //                  Cxkk - RND Vx, byte
        //  =======================================================================================
        //  Set Vx = random byte AND kk
        //  The interpreter generates a random number from 0 to 255, which is then ANDed with
        //  the value kk. The results are stored in Vx. See instruction 8xy2 for more info on AND.
        //  =======================================================================================
        case 0x0C:
            chip->registers.V[x] = (rand() % 256) & opcodeLow;
            chip->pc += 2;
            break;

        //  =======================================================================================
        //                  Dxyn - DRW Vx, Vy, nibble
        //  =======================================================================================
        //  Display n-byte sprite starting at memory location I at (Vx, Vy), set VF = collision
        //  The interpreter reads n bytes from memory, starting at the address stored in I.
        //  These bytes are then displayed as sprites on screen at coordinates (Vx, Vy).
        //  Sprites are XORed onto the existing screen. If this causes any pixels to be erased,
        //  VF is set to 1, otherwise it is set to 0.
        //  If the sprite is positioned so part of it is outside the coordinates of the display,
        //  it wraps around to the opposite side of the screen.
        //  See instruction 8xy3 for more information on XOR, and section 2.4, Display, for more information on the Chip-8 screen and sprites.
        //  =======================================================================================
        //  Each sprite is 8xN
        case 0x0D:
            chip->registers.VF = 0;
			u8 pixelOriginX = chip->registers.V[x];
			u8 pixelOriginY = chip->registers.V[y];

            for(int i = 0; i < (opcodeLow & 0x0F); i++){
                // get status of pixel, 1 = on, 0 = off
                u8 pixel = Read(chip, chip->registers.I + i);

				// go through every bit in the pixel
                for(int j = 0; j < 8; j++){

					int pixX = (pixelOriginX + j) % WIDTH;
					int pixY = (pixelOriginY + i) % HEIGHT;

                    if ((pixel & (0x80 >> j)) != 0){
						if (chip->screen[pixY][pixX] == 1)
							chip->registers.VF = 1;
						chip->screen[pixY][pixX] ^= 1;
                    }
                }
            }
			chip->drawFlag = true;
            chip->pc += 2;
            break;

        case 0x0E:
            switch(opcodeLow){

                //  =======================================================================================
                //          Ex9E - SKP Vx
                //  =======================================================================================
                //  Skip next instruction if key with the value of Vx is pressed
                //  Checks the keyboard, and if the key corresponding to the value of Vx is
                //  currently in the down position, PC is increased by 2
                //  =======================================================================================
                case 0x9E:
                    if(chip->keys[chip->registers.V[x]])
                        chip->pc += 4;
                    else
                        chip->pc += 2;
                    break;

                //  =======================================================================================
                //          ExA1 - SKNP Vx
                //  =======================================================================================
                //  Skip next instruction if key with the value of Vx is not pressed.
                //  Checks the keyboard, and if the key corresponding to the value of Vx is
                //  currently in the up position, the PC is increased by 2
                //  =======================================================================================
                case 0xA1:
                    if(!chip->keys[chip->registers.V[x]])
                        chip->pc += 4;
                    else
                        chip->pc += 2;
                    break;
            }
            break;

        case 0x0F:
            switch(opcodeLow){

                //  =======================================================================================
                //          Fx07 - LD Vx, DT
                //  =======================================================================================
                //  Set Vx = delay timer value
                //  The value of DT is placed into Vx
                //  =======================================================================================
                case 0x07:
                    chip->registers.V[x] = chip->registers.delayTimer;
                    chip->pc += 2;
                    break;

                //  =======================================================================================
                //          Fx0A - LD Vx, K
                //  =======================================================================================
                //  Wait for a key press, store the value of the key in Vx.
                //  All execution stops until a key is pressed, then the value of that key is stored in Vx.
                //  =======================================================================================
                case 0x0A:
				{
					bool keyPressed = false;

					for (int i = 0; i < 16; i++) {
						if (chip->keys[i]) {
							chip->registers.V[x] = i;
							keyPressed = true;
						}
					}

					if (!keyPressed)
						return;

					chip->pc += 2;
				}
                    break;

                //  =======================================================================================
                //          Fx15 - LD DT, Vx
                //  =======================================================================================
                //  Set delay timer = Vx
                //  DT is set equal to the value of Vx.
                //  =======================================================================================
                case 0x15:
                    chip->registers.delayTimer = chip->registers.V[x];
                    chip->pc += 2;
                    break;

                //  =======================================================================================
                //          Fx18 - LD ST, Vx
                //  =======================================================================================
                //  Set sound timer = Vx
                //  ST is set equal to the value of Vx
                //  =======================================================================================
                case 0x18:
                    chip->registers.soundTimer = chip->registers.V[x];
                    chip->pc += 2;
                    break;

                //  =======================================================================================
                //          Fx1E - Add I, Vx
                //  =======================================================================================
                //  Set I = I + Vx
                //  The values of I and Vx are added, and the results are stored in I.
                //  =======================================================================================
                case 0x1E:
					if (chip->registers.I + chip->registers.V[x] > 0xFFF)
						chip->registers.VF = 1;
					else
						chip->registers.VF = 0;


                    chip->registers.I += chip->registers.V[x];
                    chip->pc += 2;
                    break;

                //  =======================================================================================
                //          Fx29 - LD F, Vx
                //  =======================================================================================
                //  Set I = location of sprite for digit Vx.
                //  The value of I is set to the location for the hex sprite corresponding to the value
                //  of Vx.
                //  =======================================================================================
                case 0x29:
                    chip->registers.I = chip->registers.V[x] * 0x5;
                    chip->pc += 2;
                    break;

                //  =======================================================================================
                //          Fx33 - LD B, Vx
                //  =======================================================================================
                //  Store BCD representation of Vx in memory locations I, I+1, and I + 2
                //  The interpreter takes the decimal value of Vx, and places the hundreds digit
                //  in memory at location I, then tens digit at location I + 1, and the ones
                //  digit at location I + 2.
                //  =======================================================================================
                case 0x33:
					{
						u8 Vx = chip->registers.V[x];
						u8 hundreds = Vx / 100;
						u8 tens = ((Vx / 10) % 10);
						u8 ones = Vx % 10;

						Write(chip, chip->registers.I, hundreds);
						Write(chip, chip->registers.I + 1, tens);
						Write(chip, chip->registers.I + 2, ones);

						printf("Fx33: Vx (%02x) I (%02x), I + 1 (%02x), I + 2 (%02x)\n", Vx, Read(chip, chip->registers.I), Read(chip, chip->registers.I + 1), Read(chip, chip->registers.I + 2));

						chip->pc += 2;
					}
                    break;

                //  =======================================================================================
                //          Fx55 - LD [I], Vx
                //  =======================================================================================
                //  Store registers V0 through Vx in memory starting at location I.
                //  The interpreter copies the values of registers V0 through Vx into memory,
                //  starting at the address in I
                //  =======================================================================================
                case 0x55:
                    for(int i = 0; i <= x; i++){
                        Write(chip, chip->registers.I + i, chip->registers.V[i]);
                    }

                    chip->pc += 2;

                    break;

                //  =======================================================================================
                //          Fx65 - LD Vx, [I]
                //  =======================================================================================
                //  Reads registers V0 though Vx from memory starting at location I.
                //  The interpreter reads values from memory starting at location I into registers
                //  V0 through Vx.
                //  =======================================================================================
                case 0x65:
                    for(int i = 0; i <= x; i++){
                        chip->registers.V[i] = Read(chip, chip->registers.I + i);
                    }

                    chip->pc += 2;
                    break;
            }
            break;

        default:
            printf("Not implemented\n");
            break;
    }
}

void UpdateCHIP8Timers(CHIP8 *chip) {
	if (chip->registers.delayTimer > 0)
		chip->registers.delayTimer -= 1;

	if (chip->registers.soundTimer > 0) {
		chip->registers.soundTimer -= 1;
		chip->playSound = true;
	}
	else if (chip->registers.soundTimer == 0) {
		chip->playSound = false;
	}
}