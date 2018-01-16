#ifndef SYSCALL_LOOKUP_H
#define SYSCALL_LOOKUP_H

#include <map>
#include <vector>

using namespace std;

const map<string,vector<int>> syscall_map = 
{
    {"_int_free", {2, 0, 3}}
};


#endif /* SYSCALL_LOOKUP_H */