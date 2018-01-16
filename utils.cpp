#include "syscall_identification.hpp"

void print_block(Block *block)
{
	Block::Insns instructions;
	block->getInsns(instructions);

	for (auto k = instructions.begin(); k != instructions.end(); ++k)
	{
		Instruction::Ptr instr = k->second;
		Address addr = k->first;

		cout << hex << addr << ": " << instr->format() << endl;
	}
	cout << dec;
}
