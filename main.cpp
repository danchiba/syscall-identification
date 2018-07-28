#include "syscall_identification.hpp"

vector<Syscall *> *get_all_syscalls(CodeObject *codeObject);
void print_syscalls(map<Address, vector<int>> syscall_list, bool cmake_output);

int main(int argc, char *argv[])
{
	bool cmake_output;

	if (argc < 2)
	{
		printf("Usage: %s <path-to-application-binary> [cmake]\n", argv[0]);
		return -1;
	}
	char *progName = argv[1];

	if (argc >= 3 && !strcmp(argv[2], "cmake"))
	{
		cmake_output = true;
	}
	else
	{
		cmake_output = false;
	}

	/* Set up Dyninst data structures */
	SymtabCodeSource *sts;
	CodeObject *co;
	SymtabAPI::Symtab *symTab;

	if (!SymtabAPI::Symtab::openFile(symTab, progName))
	{
		cout << "File can not be parsed\n";
		return -1;
	}

	sts = new SymtabCodeSource(progName);
	co = new CodeObject(sts);
	co->parse();

	vector<Syscall *> *syscalls = get_all_syscalls(co);
	if (!cmake_output)
	{
		printf("%zd syscalls found\n", syscalls->size());
	}

	map<Address, vector<int>> syscall_nos;
	for (auto s = syscalls->begin(); s != syscalls->end(); ++s)
	{
		Syscall *sc = *s;
		syscall_nos[sc->get_address()] = sc->get_possible_sc_nos();
	}

	print_syscalls(syscall_nos, cmake_output);

	delete syscalls, symTab, co, sts;
	return 0;
}

/* Creates a Syscall object for each syscall that's found 
   and returns a list of the same */
vector<Syscall *> *get_all_syscalls(CodeObject *codeObject)
{
	vector<Syscall *> *syscalls = new vector<Syscall *>;

	const CodeObject::funclist &funcs = codeObject->funcs();
	if (funcs.size() == 0)
	{
		cout << "No functions in file\n";
		exit(-1);
	}

	/* Iterate over every instruction within every basic block within 
	   every function to find "syscall" instructions */
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
				cout << "No instructions in block";
				continue;
			}

			for (auto k = instructions.begin(); k != instructions.end(); ++k)
			{
				Instruction::Ptr instr = k->second;
				uint64_t addr = k->first;

				/* Some blocks overlap, so there might be an instruction that's part of mulitiple blocks */
				bool already_caught = any_of(syscalls->begin(), syscalls->end(),
											 [&](Syscall *s) { return addr == s->get_address(); });
				if (instruction_is_syscall(instr) and !already_caught)
				{
					Syscall *sc = new Syscall(f, bb, instr, addr);
					syscalls->push_back(sc);
				}
			}
		}
	}
	return syscalls;
}

void print_syscalls(map<Address, vector<int>> syscall_list, bool cmake_output)
{
	if (cmake_output)
	{
		cmake_print(syscall_list);
	}
	else
	{
		verbose_print(syscall_list);
		unique_print(syscall_list);
	}
}
