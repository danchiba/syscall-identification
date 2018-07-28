/**
 * utils.cpp
 * Contains random utility functions. Currently mostly for printing.
 * 
 * @author Daniel Chiba
 */

#include "syscall_identification.hpp"

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

/* Prints the address and syscall number(s) for each syscall instruction */
void verbose_print(map<Address, vector<int>> syscall_list)
{
	for (auto s = syscall_list.begin(); s != syscall_list.end(); ++s)
	{
		uint64_t addr = s->first;
		vector<int> scnums = s->second;
		printf("%#lx: ", addr);

		for (auto scn = scnums.begin(); scn != scnums.end(); ++scn)
		{
			if (scn != scnums.begin())
			{
				printf(" ,%d", *scn);
			}
			else
			{
				printf("%d", *scn);
			}
		}
		printf("\n");
	}
}

/* Returns a list of only the unique integers from the various vectors in the map */
vector<int> get_unique_syscalls(map<Address, vector<int>> syscall_list)
{
	vector<int> unique_syscalls;

	for (auto s = syscall_list.begin(); s != syscall_list.end(); ++s)
	{
		vector<int> scnums = s->second;

		for (auto scn = scnums.begin(); scn != scnums.end(); ++scn)
		{
			int no = *scn;
			if (!any_of(unique_syscalls.begin(), unique_syscalls.end(), [no](int n) { return n == no; }))
			{
				/* Make sure the list is in numerical order */
				auto pos = upper_bound(unique_syscalls.begin(), unique_syscalls.end(), no);
				unique_syscalls.insert(pos, no);
			}
		}
	}
	return unique_syscalls;
}

/* Prints only the unique system call numbers */
void unique_print(map<Address, vector<int>> syscall_list)
{
	vector<int> unique_syscalls = get_unique_syscalls(syscall_list);

	printf("%zd unique system calls were present:\n", unique_syscalls.size());
	for (auto scn = unique_syscalls.begin(); scn != unique_syscalls.end(); ++scn)
	{
		if (scn != unique_syscalls.begin())
		{
			printf(", %d", *scn);
		}
		else
		{
			printf("%d", *scn);
		}
	}
	printf("\n");
}

/* Prints in a format fhat can directly be parsed into a list by CMake */
void cmake_print(map<Address, vector<int>> syscall_list)
{
	vector<int> unique_syscalls = get_unique_syscalls(syscall_list);

	for (auto scn = unique_syscalls.begin(); scn != unique_syscalls.end(); ++scn)
	{
		if (scn != unique_syscalls.begin())
		{
			printf(";%d", *scn);
		}
		else
		{
			printf("%d", *scn);
		}
	}
	printf("\n");
}
