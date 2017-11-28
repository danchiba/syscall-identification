#ifndef SYSCALL_IDENTIFICATION_H
#define SYSCALL_IDENTIFICATION_H

#include <stdio.h>
#include <typeinfo>
#include "CodeObject.h"
#include "InstructionDecoder.h"
#include "CFG.h"
#include "PatchCFG.h"
#include "Absloc.h"
#include "AbslocInterface.h"
#include "dyn_regs.h"
#include "slicing.h"
#include "Graph.h"
#include "Node.h"

using namespace std;
using namespace Dyninst;
using namespace ParseAPI;
using namespace PatchAPI;
using namespace InstructionAPI;
using namespace DataflowAPI;

class Syscall
{
	Function *function;
	Instruction::Ptr instruction;
	Address sc_address;
	Address eax_address;
	Block *sc_block;
	Assignment::Ptr eax_assmnt;

	void set_eax_assignment();

  public:
	Syscall(Function *func, Block *block, Instruction::Ptr &instr, uint64_t addr);
	Function *get_function();
	Block *get_sc_block();
	Address get_address();
	Instruction::Ptr get_instruction();
	uint32_t get_syscall_number();
};

class SyscallNumberPred : public Slicer::Predicates
{
  public:
	virtual bool endAtPoint(Assignment::Ptr ap);
};

// void print_block(Block *block)
// {
// 	Block::Insns instructions;
// 	block->getInsns(instructions);

// 	for (auto k = instructions.begin(); k != instructions.end(); ++k)
// 	{
// 		Instruction::Ptr instr = k->second;
// 		Address addr = k->first;

// 		cout << hex << addr << ": " << instr->format() << endl;
// 	}
// 	cout << dec;
// }

#endif /* SYSCALL_IDENTIFICATION_H */
