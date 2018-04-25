DYNINST_INSTALL=$(HOME)/Desktop/dyninst/build/prefix/
SRC=instruction_parser.cpp main.cpp Syscall.cpp utils.cpp
PROG=identify_syscalls
CPPFLAGS=-I$(DYNINST_INSTALL)/include
LDFLAGS=-L$(DYNINST_INSTALL)/lib -lparseAPI -ldyninstAPI -linstructionAPI \
		-lsymtabAPI -lcommon -Wl,-R $(DYNINST_INSTALL)/lib

CC=g++

all: $(PROG)

$(PROG): $(SRC)
	$(CC) $(CPPFLAGS) $^ -o $@ $(LDFLAGS)

clean:
	rm -rf *.o $(PROG)
