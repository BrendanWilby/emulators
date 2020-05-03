#pragma once

#include "MMU.h"
#include "CPU.h"
#include "Debug.h"

class GameBoy {
private:
	MMU *mmu;
	CPU *cpu;

	bool running;
	bool paused;
public:
	GameBoy();

	void Reset();
	void Start();
	void Pause();
	void UnPause();
	void Quit();
	void Run();
};