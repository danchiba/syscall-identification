# Syscall Identification

This program tries to identify all of the system calls that could be made by a statically compiled Linux binary on the x86_64 architecture. It does so by statically analysing the binary and determining which value will be present in the `RAX` register when each `syscall` instruction is executed.

## Requirements
- This program depends on [Dyninst](https://www.dyninst.org/) in order to obtain a control flow graph. Dyninst can obtained from [here](https://github.com/dyninst/dyninst). 
- It also requries [CMake](https://cmake.org/) 3.5 or later to build.
- The binaries whose system calls need to be identified must contain their symbol tables (they must not be stripped).

## Build
After Dyninst has been compiled and installed (follow instructions on Github link above):
```sh
$ cmake .
$ make -j`nproc`
```

## Run
```sh
$ ./identify_syscalls <path_to_binary> [cmake]
```
If the `cmake` option is specified, the output will be in the form of a semi-colon separated list containing only the unique system call numbers, which can be parsed into a list directly by CMake.

If the `cmake` option is not specified, the program will print, for each system call, its address along with the syscall number of the system call(s) that could be made by that `syscall` instruction.


## Notes
Not all of the system calls can be identified through binary analysis. In the case of glibc, there is one instance where we had to analyse the source code in order to determine the system call being made at that point. The program then reverts to a lookup table to identify a system call when analysis fails. While this is a hack, we only came across one instance where this was required, and since the standard library is not likely to change very often, the lookup table should be relatively stable.

## TODOs
- Write automated tests.
