#ifndef SYSCALL_IDENTIFICATION_H
#define SYSCALL_IDENTIFICATION_H

#include <stdio.h>
#include <typeinfo>
#include "CodeObject.h"
#include "InstructionDecoder.h"
#include "CFG.h"
#include "PatchCFG.h"
#include "PatchObject.h"
#include "PatchModifier.h"

using namespace std;
using namespace Dyninst;
using namespace ParseAPI;
using namespace PatchAPI;
using namespace InstructionAPI;

class Syscall
{
	Function *function;
	Instruction::Ptr instruction;
	Address address;
	Block *sc_block;

  public:
	Syscall(Function *func, Block *block, Instruction::Ptr &instr, uint64_t addr);
	Function *get_function();
	Block *get_sc_block();
	Address get_address();
	Instruction::Ptr get_instruction();
};

#endif /* SYSCALL_IDENTIFICATION_H */
