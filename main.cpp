#include "syscall_identification.h"

vector<Syscall *> *get_all_syscalls(CodeObject *codeObject);
void print_block(Block *block);

int main(int argc, char *argv[])
{
	if (argc != 2)
	{
		printf("Usage: %s <path-to-application-binary>\n", argv[0]);
		return -1;
	}
	char *progName = argv[1];
	string progNameStr(progName);

	SymtabCodeSource *sts;
	CodeObject *co;
	PatchObject *po, *clonedpo;
	Instruction::Ptr instr;
	SymtabAPI::Symtab *symTab;
	vector<Syscall *> *syscall_list;

	if (!SymtabAPI::Symtab::openFile(symTab, progNameStr))
	{
		cout << "File can not be parsed\n";
		return -1;
	}

	sts = new SymtabCodeSource(progName);
	co = new CodeObject(sts);
	co->parse();

	syscall_list = get_all_syscalls(co);
	cout << dec << syscall_list->size() << endl
		 << endl;

	
	return 0;
}

vector<Syscall *> *get_all_syscalls(CodeObject *codeObject)
{
	vector<Syscall *> *syscall_list = new vector<Syscall *>;

	const CodeObject::funclist &funcs = codeObject->funcs();
	if (funcs.size() == 0)
	{
		cout << "No functions in file\n";
		exit(1);
	}
	cout << funcs.size() << " functions found\n";

	for (auto f1 = funcs.begin(); f1 != funcs.end(); ++f1)
	{
		Function *f = *f1;
		const Function::blocklist &bblocks = f->blocks();

		for (auto j = bblocks.begin(); j != bblocks.end(); ++j)
		{
			Block *bb = *j;
			Block::Insns instructions;
			bb->getInsns(instructions);

			if (instructions.size() == 0)
			{
				cout << "No instructions";
				continue;
			}

			for (auto k = instructions.begin(); k != instructions.end(); ++k)
			{
				Instruction::Ptr instr = k->second;
				uint64_t addr = k->first;
				Operation op = instr->getOperation();

				string mnemonic = op.format();
				bool already_caught = any_of(syscall_list->begin(), syscall_list->end(),
											 [&](Syscall *s) { return addr == s->get_address(); });
				if (!mnemonic.compare("syscall") and !already_caught)
				{
					Syscall *sc = new Syscall(f, bb, instr, addr);
					syscall_list->push_back(sc);
				}
			}
		}
	}
	return syscall_list;
}

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

