#ifndef SYSCALL_IDENTIFICATION_H
#define SYSCALL_IDENTIFICATION_H

#include <stdio.h>
#include <typeinfo>
#include <boost/algorithm/string/predicate.hpp>
#include "CodeObject.h"
#include "Instruction.h"
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
using namespace boost::algorithm;

class Syscall
{
	Function *function;
	Instruction::Ptr instruction;
	Address sc_address;
	Address eax_address;
	Block *sc_block;
	void get_value_in_register(Block *curr_block, string reg, Address start_from, vector<Block *> *visited, vector<int> *possible_sc_nums);

  public:
	Syscall(Function *func, Block *block, Instruction::Ptr &instr, uint64_t addr);
	Function *get_function(void);
	Block *get_sc_block(void);
	Address get_address(void);
	Instruction::Ptr get_instruction(void);
	vector<int> get_possible_sc_nos(void);
	vector<int> lookup_sc_numbers(void);
};

bool instruction_assigns_to_register(Instruction::Ptr instr, string reg);
string get_size_agnostic_reg_name(RegisterAST::Ptr reg);
bool operand_is_immediate(Operand operand);
bool instruction_is_syscall(Instruction::Ptr instr);
bool instruction_is_mov(Instruction::Ptr instr);
bool instruction_is_self_xor(Instruction::Ptr instr);
uint32_t get_immediate_value(Operand op);

void print_block(Block *block);

enum class Registers
{
	rax,
	rbx,
	rcx,
	rdx,
	rdi,
	rsi,
	rbp,
	r8,
	r9,
	r10,
	r11,
	r12,
	r13,
	r14,
	r15
};

#endif /* SYSCALL_IDENTIFICATION_H */
