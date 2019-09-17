# emulators

CHIP-8: An emulator for the CHIP-8 interpreter. Uses SDL2 to draw graphics.

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