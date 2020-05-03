#include "GameBoy.h"

// GameBoy constructor. MMU and CPU are reset on initialization. GameBoy is automatically started.
GameBoy::GameBoy() {
	Debug::Log("Starting GameBoy");

	mmu = new MMU();
	cpu = new CPU(mmu);
}

// Resets the GameBoy - wipes all memory and registers and starts the GameBoy
void GameBoy::Reset() {
	Debug::Log("Resetting GameBoy");

	running = false;

	mmu->Reset();
	cpu->Reset();

	Start();
}

void GameBoy::Start() {
	running = true;
	Debug::Log("GameBoy Started Successfully");

	Run();
}

// Pauses exeuction of the CPU
void GameBoy::Pause() {
	paused = true;
	Debug::Log("GameBoy Paused");
}

// UnPauses an onoing pause in execution of the CPU
void GameBoy::UnPause() {
	paused = false;
	Debug::Log("GameBoy UnPaused");
}

// Quits the GameBoy
void GameBoy::Quit() {
	Debug::Log("Qutting...");
	running = false;

	Debug::Log("GameBoy Quitted Successfully");
	exit(0);
}

// Main GameBoy logic
void GameBoy::Run() {
	uint8_t opCycles = cpu->Execute(true);
	/*
	while (running) {
		if(!paused)
			uint8_t opCycles = cpu->Execute(true);
	}
	*/
}
