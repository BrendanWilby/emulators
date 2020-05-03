#pragma once

#include <iostream>
#include <string>
#include <iomanip>

constexpr auto PREFIX_ERROR = "ERROR: ";
constexpr auto PREFIX_WARNING = "WARNING: ";

class Debug {
public:
	static void Log(std::string message);
	static void LogError(std::string errMsg, bool quit);
	static void LogWarning(std::string warnMsg);

	static void PrintInstruction(uint16_t pc, uint8_t opcode, std::string mnemonic);
private:
	Debug() {}
};
