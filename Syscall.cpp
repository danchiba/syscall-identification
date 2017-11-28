#include "syscall_identification.h"

Syscall::Syscall(Function *func, Block *block, Instruction::Ptr &instr, uint64_t addr)
{
    this->function = func;
    this->instruction = instr;
    this->sc_address = addr;
    this->sc_block = block;
    this->set_eax_assignment();
}

void Syscall::set_eax_assignment()
{
    Block::Insns insns;
    this->sc_block->getInsns(insns);
    AssignmentConverter *aconv = new AssignmentConverter(false, false);

    /* Start searching for asssignments in instructions in reverse order from the syscall */
    for (auto i = insns.rbegin(); i != insns.rend(); ++i)
    {
        Instruction::Ptr instr = i->second;
        Address addr = i->first;
        vector<Assignment::Ptr> assignments;

        /*  Convert each instruction into assignments. 
         *  If the output region of any of these assignments is the rax/eax register,
         *  then that's the assignment we're looking for.  */
        aconv->convert(instr, addr, this->function, this->sc_block, assignments);
        for (auto j = assignments.begin(); j != assignments.end(); ++j)
        {
            Assignment::Ptr asmnt = *j;
            AbsRegion outregn = asmnt->out();

            if (!outregn.isImprecise() && outregn.containsOfType(Absloc::Type::Register))
            {
                //cout << outregn.format() << endl;
                Absloc aloc = outregn.absloc();
                MachRegister reg = aloc.reg();
                if (reg.name().find("ax") != string::npos)
                {
                    this->eax_assmnt = asmnt;
                    this->eax_address = addr;
                    //cout << hex << addr << endl;
                    return;
                }
            }
        }
    }
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
    return this->sc_address;
}

Instruction::Ptr Syscall::get_instruction()
{
    return this->instruction;
}

uint32_t Syscall::get_syscall_number()
{
    if (this->eax_assmnt == nullptr)
    {
        cout << "No assignment for " << hex << this->sc_address << dec << endl;
        return 0;
    }
    Slicer s(this->eax_assmnt, this->sc_block, this->function, false, false);
    Slicer::Predicates pred;

    GraphPtr graph = s.backwardSlice(pred);

    NodeIterator begin, end;
    graph->entryNodes(begin, end);

    //int count = 0;
    printf("Syscall at %lx:\n", this->sc_address);
    for (auto i = begin; i != end; ++i)
    {
        Node::Ptr node = *i;
        auto snode = dynamic_pointer_cast<SliceNode>(node);
        // cout << snode->format() << endl;
        Instruction::Ptr instr = snode->assign()->insn();
        printf("%#lx: ", snode->addr());
        cout << instr->format() << endl;
    }
    cout << endl;
    // if (count != 1)
    // {
    //     cout << hex << this->sc_address << dec << endl;
    //     cout << count << " entry nodes\n";
    // }
    return 0;
}

bool SyscallNumberPred::endAtPoint(Assignment::Ptr asmnt)
{
    return false;
}