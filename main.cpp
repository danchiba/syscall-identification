#include "syscall_identification.hpp"

vector<Syscall *> *get_all_syscalls(CodeObject *codeObject);
void create_syscall_num_list(vector<Syscall *> *syscall_list);

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
	printf("%zd syscalls found\n\n", syscall_list->size());

	create_syscall_num_list(syscall_list);
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

				bool already_caught = any_of(syscall_list->begin(), syscall_list->end(),
											 [&](Syscall *s) { return addr == s->get_address(); });
				if (instruction_is_syscall(instr) and !already_caught)
				{
					Syscall *sc = new Syscall(f, bb, instr, addr);
					syscall_list->push_back(sc);
				}
			}
		}
	}
	return syscall_list;
}

void create_syscall_num_list(vector<Syscall *> *syscall_list)
{
	vector<int> sc_num_list;
	for (auto i = syscall_list->begin(); i != syscall_list->end(); ++i)
	{
		Syscall *sc = *i;
		vector<int> possible_scs = sc->get_possible_sc_nos();

		if (any_of(possible_scs.begin(), possible_scs.end(), [](int n) { return n < 0; }))
		{
			// printf("Using lookup for syscall at %#lx\n", sc->get_address());
			possible_scs = sc->lookup_sc_numbers();
		}

		//printf("For syscall at %#lx:\n", sc->get_address());
		for (auto j = possible_scs.begin(); j != possible_scs.end(); ++j)
		{
			int no = *j;
			// printf("Found syscall #%d\n", no);
			if (!any_of(sc_num_list.begin(), sc_num_list.end(), [no](int n) { return n == no; }))
			{
				auto pos = upper_bound(sc_num_list.begin(), sc_num_list.end(), no);
				sc_num_list.insert(pos, no);
			}
		}
	}

	printf("Final syscall list:\n");
	for (auto i = sc_num_list.begin(); i != sc_num_list.end(); ++i)
	{
		printf("%d\n", *i);
	}
}