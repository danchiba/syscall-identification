#include "syscall_identification.h"


Syscall::Syscall(Function *func, Block *block, Instruction::Ptr &instr, uint64_t addr)
{
    this->function = func;
    this->instruction = instr;
    this->address = addr;
    this->sc_block = block;
}

Function *Syscall::get_function()
{
    return this->function;
}

Block *Syscall::get_sc_block()
{
    return this->sc_block;
}

Address Syscall::get_address()
{
    return this->address;
}

Instruction::Ptr Syscall::get_instruction()
{
    return this->instruction;
}

