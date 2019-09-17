/*
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
											CHIP-8 EMULATOR - Written by Brendan Wilby 14-17/09/2019
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
																References
--------------------------------------------------------------------------------------------------------------------------------------------------------------
	https://en.wikipedia.org/wiki/CHIP-8							- for general information and descriptions of opcodes
	http://mattmik.com/files/chip8/mastering/chip8.html				- for excellent descriptions of opcodes and the system in general
	http://devernay.free.fr/hacks/chip8/C8TECH10.HTM				- Comments written below, above each opcodem are from here
	http://emulator101.com/											- Fantastic walkthrough of how CHIP-8 works
	https://github.com/kpmiller/emulator101/tree/master/Chip8		- Source code from above site, which helped enormously with some of the more difficult to implement opcodes!
	https://github.com/dmatlack/chip8/tree/master/roms				- Source of most of the ROMs used
	https://www.reddit.com/r/EmuDev/comments/8a4coz/how_do_you_test_your_emu_chip8/		-	Two test programs found in the comments here


--------------------------------------------------------------------------------------------------------------------------------------------------------------

--------------------------------------------------------------------------------------------------------------------------------------------------------------
															What is CHIP-8?
--------------------------------------------------------------------------------------------------------------------------------------------------------------
	From Wikipedia, "CHIP-8 is an interpreted programming language,
	developed by Joseph Weisbecker".

--------------------------------------------------------------------------------------------------------------------------------------------------------------


--------------------------------------------------------------------------------------------------------------------------------------------------------------
																SPECS
--------------------------------------------------------------------------------------------------------------------------------------------------------------
	Memory			- 4096B (4KB) of memory
					- The first 512 bytes are reserved for the interpreter and fontsets
					- Programs are loaded in from address 0x200 (512B) onwards
					- In this emulation, there is no need to store the interpreter in the first 512 bytes,
					  and the screen data is treated separately.

	Registers		-  There are 16 8-bit registers, labelled V0-VF
					-  VF is used as a carry flag, no borrow flag, and pixel collision flag,
					   and so its use should be avoided
					-  The 16-bit address register I is used to store memory addresses
	
	Non-registers	- PC is the program counter, it stores the address of the currently executed opcode
					- SP is the stack pointer, and points to the top of the stack

	Timers			- Delay timer: used for timing events, decremented at a rate of 60Hz.
					- Sound timer: a "beep" sound is made when this timer is non-zero, and is decremented at a rate of 60Hz

	Stack			- A size 16 array, used to store addresses to return to after a subroutine has been called
*/

#include "CHIP8.h"
#include "SDL.h"

CHIP8 chip;
SDL_Window *window = NULL;
SDL_Renderer *renderer = NULL;
SDL_Event e;
bool running = false;
bool advance = false;
bool paused = false;
const int FPS = 800;
const int SCREEN_WIDTH = 1024;
const int SCREEN_HEIGHT = 512;

// Keymapping that works more nicely with QWERTY keyboards

u8 KEYMAP_MODERN[16] = {
	SDLK_1,
	SDLK_2,
	SDLK_3,
	SDLK_4,
	SDLK_q,
	SDLK_w,
	SDLK_e,
	SDLK_r,
	SDLK_a,
	SDLK_s,
	SDLK_d,
	SDLK_f,
	SDLK_z,
	SDLK_x,
	SDLK_c,
	SDLK_v
};

// Keymapping that is faithful to the original keymapping
u8 KEYMAP_CLASSIC[16] = {
	SDLK_1,
	SDLK_2,
	SDLK_3,
	SDLK_c,
	SDLK_4,
	SDLK_5,
	SDLK_6,
	SDLK_d,
	SDLK_7,
	SDLK_8,
	SDLK_9,
	SDLK_e,
	SDLK_a,
	SDLK_0,
	SDLK_b,
	SDLK_f
};

// A pointer to an array, corresponding to the current keymap in use
u8 (*keymap)[16] = &KEYMAP_CLASSIC;

char * GAMES[10] = {
	"Roms/Lunar Lander (Udo Pernisz, 1979).ch8",
	"Roms/Space Invaders [David Winter].ch8",
	"Roms/Tapeworm [JDR, 1999].ch8",
	"Roms/Tetris [Fran Dachille, 1991].ch8",
	"Roms/Tic-Tac-Toe [David Winter].ch8",
	"Roms/UFO [Lutz V, 1992].ch8",
	"Roms/Wall [David Winter].ch8",
	"Roms/Wipe Off [Joseph Weisbecker].ch8",
	"Roms/Worm V4 [RB-Revival Studios, 2007].ch8",
	"Roms/X-Mirror.ch8"
};

char * TESTS[8] = {
	"Tests/Chip8 emulator Logo [Garstyciuks].ch8",
	"Tests/Chip8 Picture.ch8",
	"Tests/Delay Timer Test [Matthew Mikolay, 2010].ch8",
	"Tests/IBM Logo.ch8",
	"Tests/Keypad Test [Hap, 2006].ch8",
	"Tests/Life [GV Samways, 1980].ch8",
	"Tests/BC_test.ch8",
	"Tests/test_opcode.ch8"
};

// Initialize SDL window and renderer and load the ROM to memory
static void InitSDL() {
	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		printf("SDL failed to initialize. SDL Error: %s\n", SDL_GetError());
		exit(1);
	}

	window = SDL_CreateWindow("CHIP-8 Emulator", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);

	if (window == NULL) {
		printf("Could not initialize SDL Window. SDL error: %s\n", SDL_GetError());
		exit(1);
	}

	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

	if (renderer == NULL) {
		printf("Failed to create SDL Renderer. SDL Error: %s\n", SDL_GetError());
		exit(1);
	}

	SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0xFF);

	//LoadROMToMemory(&chip, TESTS[4]);
	LoadROMToMemory(&chip, GAMES[1]);

	//DisassembleROM(&chip, "Space Invaders [David Winter].ch8");    

	running = true;
}

static void HandleInput() {
	while (SDL_PollEvent(&e) != 0) {
		if (e.type == SDL_QUIT)
			running = false;
		else if (e.type == SDL_KEYDOWN) {
			for (int i = 0; i < 16; i++) {
				if (e.key.keysym.sym == (*keymap)[i])
					chip.keys[i] = 1;
			}

			if (e.key.keysym.sym == SDLK_SPACE)
				advance = true;

			if (e.key.keysym.sym == SDLK_p) 
				paused = !paused;
		}
		else if (e.type == SDL_KEYUP) {
			for (int i = 0; i < 16; i++) {
				if (e.key.keysym.sym == (*keymap)[i])
					chip.keys[i] = 0;
			}

			if (e.key.keysym.sym == SDLK_SPACE)
				advance = false;
		}
	}
}

static void Render() {
	SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0xFF);
	SDL_RenderClear(renderer);

	int pixelWidth = SCREEN_WIDTH / WIDTH;
	int pixelHeight = SCREEN_HEIGHT / HEIGHT;

	for (int y = 0; y < HEIGHT; y++) {
		for (int x = 0; x < WIDTH; x++) {
			if (chip.screen[y][x]) {
				SDL_Rect fillRect = { x * pixelWidth, y * pixelHeight, pixelWidth, pixelHeight };
				SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF);
				SDL_RenderFillRect(renderer, &fillRect);
			}
		}
	}
	SDL_RenderPresent(renderer);
}

static void Quit() {
	SDL_DestroyWindow(window);
	SDL_DestroyRenderer(renderer);
	window = NULL;
	renderer = NULL;
	SDL_Quit();
}

int main(){
	InitSDL();
	u32 startTime;

	while (running) {
		startTime = SDL_GetTicks();

		// Allows for skipping through opcodes one-by-one. Paused is true when the "p" key is called, and advance when space is pressed
		if (paused && advance) {
			EmulateCHIP8Cycle(&chip);
			advance = false;
		}
		else if (!paused) {
			EmulateCHIP8Cycle(&chip);
		}

		UpdateCHIP8Timers(&chip);

		if (chip.drawFlag) {
			Render();
			chip.drawFlag = false;
		}

		HandleInput();

		if (SDL_GetTicks() - startTime < (1000 / FPS))
			SDL_Delay((1000 / FPS) - (SDL_GetTicks() - startTime));

	}

	Quit();
    return 0;
}