#include "syscall_identification.hpp"
#include "syscall_lookup.hpp"

Syscall::Syscall(Function *func, Block *block, Instruction::Ptr &instr, uint64_t addr)
    : function(func), sc_block(block), instruction(instr), sc_address(addr)
{
}

Function *Syscall::get_function(void)
{
    return this->function;
}

Block *Syscall::get_sc_block(void)
{
    return this->sc_block;
}

Address Syscall::get_address(void)
{
    return this->sc_address;
}

Instruction::Ptr Syscall::get_instruction(void)
{
    return this->instruction;
}

vector<int> Syscall::get_possible_sc_nos(void)
{
    /* Prepare empty vectors for the visited and possible_sc_nums parameters for the initial 
       call to get_value_in_register */
    vector<Block *> *visited = new vector<Block *>();
    vector<int> *possible_sc_nums = new vector<int>();

    get_value_in_register(this->sc_block, "ax", this->sc_address, visited, possible_sc_nums);
    delete visited;

    /* If any of the possible system call numbers could not be determined through 
	   binary analysis (indicated by -1), we have to use the lookup table */
    if (any_of(possible_sc_nums->begin(), possible_sc_nums->end(), [](int n) { return n < 0; }))
    {
        possible_sc_nums = this->lookup_sc_numbers();
    }

    return *possible_sc_nums;
}

/** 
 * Recursive function to find the value in a register at a given address. We go backwards from
 * start_from to find the instruction where reg is assigned to. Recursion occurs when we have
 * to backctrace through a different block or if the value comes from another register. 
 * 
 * @param curr_block The block through which we want to backtrace.
 * @param reg The register whose value we want to determine. Should be the lowercase, 
 *            two-character size-agnostic version (ax instead of eax or rax).
 * @param start_from The address within the block from which we should start backtracing.
 * @param visited All the blocks that have been visited in the search for this particular
 *                syscall number. Needed to prevent an infinite loop.
 * @param possible_sc_nums All possible syscall numbers for this particular system call.
 * */
void Syscall::get_value_in_register(Block *curr_block, string reg, Address start_from,
                                    vector<Block *> *visited, vector<int> *possible_sc_nums)
{
    visited->push_back(curr_block);

    Block::Insns insns;
    curr_block->getInsns(insns);

    /* Start searching for asssignments in instructions in reverse order from the syscall */
    for (auto i = insns.rbegin(); i != insns.rend(); ++i)
    {
        Instruction::Ptr instr = i->second;
        Address addr = i->first;

        if (addr > start_from)
        {
            continue;
        }

        if (instruction_assigns_to_register(instr, reg))
        {
            if (instruction_is_mov(instr))
            {
                /* If the value comes from memory, there is no way to determine it statically. */
                if (instr->readsMemory())
                {
                    possible_sc_nums->push_back(-1);
                    return;
                }
                /* If the source operand is an immediate value, success! */
                Operand sourceop = instr->getOperand(1);
                if (operand_is_immediate(sourceop))
                {
                    possible_sc_nums->push_back(get_immediate_value(sourceop));
                    return;
                }
                else
                {
                    set<RegisterAST::Ptr> readset;
                    instr->getReadSet(readset);
                    /* Multiple registers are being read for some reason. Can't decide what to do. */
                    if (readset.size() != 1)
                    {
                        printf("%lx: %s\n", addr, instr->format().c_str());
                        cout << "Read set != 1\n";
                        possible_sc_nums->push_back(-2);
                        return;
                    }
                    /* The value is coming from a different register. Recurse with this register
                       as the destination register, and the assignment address as the start address
                       printf("%lx: getting from different register\n", addr); */
                    string rname = get_size_agnostic_reg_name(*(readset.begin()));
                    get_value_in_register(curr_block, rname, addr, visited, possible_sc_nums);
                    return;
                }
            }

            /* r XOR r = 0. Success! */
            else if (instruction_is_self_xor(instr))
            {
                possible_sc_nums->push_back(0);
                return;
            }

            else
            {
                printf("Unknown operation affecting value\n");
                printf("%lx: %s\n", addr, instr->format().c_str());
                possible_sc_nums->push_back(-3);
                return;
            }
        }
    }

    /* No assignment of interest has been made in the current block,
       so we traverse through all incoming blocks in a similar manner.
       Recursion occurs with a different block. start_from takes the last
       address of that block, and we update visited. */
    const Block::edgelist &incoming = curr_block->sources();
    for (auto j = incoming.begin(); j != incoming.end(); ++j)
    {
        Block *preceding_block = (*j)->src();
        vector<Function *> pbfuncs;
        preceding_block->getFuncs(pbfuncs);
        bool already_visited = any_of(visited->begin(), visited->end(), [preceding_block](Block *b) { return preceding_block == b; });
        /* We only want to visit blocks in other functions if we haven't found any possibilities in the current function */
        bool same_func = any_of(pbfuncs.begin(), pbfuncs.end(), [this](Function *f) { return this->function == f; });
        bool empty_list = possible_sc_nums->empty();
        if (!already_visited && (same_func || empty_list))
        {
            get_value_in_register(preceding_block, reg, preceding_block->last(), visited, possible_sc_nums);
        }
    }
}

/* Returns a list of possible syscall numbers using a lookup table.
   This function should only be used as a last resort if the syscall
   number cannot be determined via binary analysis. */
vector<int> *Syscall::lookup_sc_numbers(void)
{
    auto res = syscall_map.find(this->function->name());

    if (res != syscall_map.end())
    {
        auto r = new vector<int>(res->second);
        return r;
    }
    else
    {
        /* Since this is the last resort, if it doesn't work, we can simply exit. */
        printf("Syscall at %#lx cannot be determined\n", this->sc_address);
        exit(-1);
    }
}