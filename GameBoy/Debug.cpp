#include "Debug.h"

void Debug::Log(std::string message) {
	std::cout << message << std::endl;
}

void Debug::LogError(std::string errMsg, bool quit) {
	std::cout << PREFIX_ERROR << errMsg << std::endl;

	if (quit)
		exit(1);
}

void Debug::LogWarning(std::string warnMsg) {
	std::cout << PREFIX_WARNING << warnMsg << std::endl;
}

void Debug::PrintInstruction(uint16_t pc, uint8_t opcode, std::string mnemonic) {
	char output[100];
	sprintf_s(output, "PC = 0x%.2x, OPCODE = 0x%.2x (%s)\n", pc, opcode, mnemonic.c_str());
	std::cout << output;

}