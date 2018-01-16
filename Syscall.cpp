#include "syscall_identification.hpp"
#include "syscall_lookup.hpp"

Syscall::Syscall(Function *func, Block *block, Instruction::Ptr &instr, uint64_t addr)
{
    this->function = func;
    this->instruction = instr;
    this->sc_address = addr;
    this->sc_block = block;
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
    vector<Block *> *visited = new vector<Block *>();
    vector<int> *possible_sc_nums = new vector<int>();
    get_value_in_register(this->sc_block, "ax", this->sc_address, visited, possible_sc_nums);
    //printf("Visited %zd blocks\n", visited->size());
    visited->~vector();
    return *possible_sc_nums;
}

void Syscall::get_value_in_register(Block *curr_block, string reg, Address start_from, vector<Block *> *visited, vector<int> *possible_sc_nums)
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
                if (instr->readsMemory())
                {
                    // printf("Memory read!\n");
                    // printf("%lx: %s\n", addr, instr->format().c_str());
                    possible_sc_nums->push_back(-1);
                    return;
                }
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
                    if (readset.size() != 1)
                    {
                        printf("%lx: %s\n", addr, instr->format().c_str());
                        cout << "Read set != 1\n";
                        possible_sc_nums->push_back(-2);
                        return;
                    }
                    string rname = get_size_agnostic_reg_name(*(readset.begin()));
                    get_value_in_register(curr_block, rname, addr, visited, possible_sc_nums);
                    return;
                }
            }

            else if (instruction_is_self_xor(instr, addr))
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

    /*  No assignment of interest has been made in the current block,
        so we traverse through all preceding blocks in a similar manner  */
    const Block::edgelist &incoming = curr_block->sources();
    for (auto j = incoming.begin(); j != incoming.end(); ++j)
    {
        Block *preceding_block = (*j)->src();
        vector<Function *> pbfuncs;
        preceding_block->getFuncs(pbfuncs);
        bool already_visited = any_of(visited->begin(), visited->end(), [preceding_block](Block *b) { return preceding_block == b; });
        bool same_func = any_of(pbfuncs.begin(), pbfuncs.end(), [this](Function *f) { return this->function == f; });
        bool empty_list = possible_sc_nums->empty();
        if (!already_visited && (same_func || empty_list))
        {
            get_value_in_register(preceding_block, reg, preceding_block->last(), visited, possible_sc_nums);
        }
    }
}

vector<int> Syscall::lookup_sc_numbers(void)
{
    auto res = syscall_map.find(this->function->name());

    if (res != syscall_map.end())
    {
        return res->second;
    }
    else
    {
        /* Since this is the last resort, if it doesn't work, we can simply exit. */
        printf("Syscall at %#lx cannot be determined\n", this->sc_address);
        exit(-1);
    }
}