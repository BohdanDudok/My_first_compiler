#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <bitset>
#include <math.h>
using namespace std;
#define NUMMEMORY 65536 /* maximum number of words in memory */
#define NUMREGS 64 /* number of machine registers */
#define MAXLINELENGTH 1000
// Додані команди
#define XSUB 0
#define XIDIV 1
#define XADD 2
#define XOR 3
#define CMPE 4
#define SAR 5
#define JMA 6
#define JML 7
#define ADC 8
#define SBB 9
#define RCR 10
#define LOAD 11
#define STORE 12
// Стандартні команди
#define LW 13
#define SW 14
#define HALT 15
#define NOOP 16
#define ADD 17
#define NAND 18
#define BEQ 19
#define JALR 20

typedef struct stateStruct {
	long long int pc;
	long long int mem[NUMMEMORY];
	long long int reg[NUMREGS];
	long long int numMemory;
	long long int CF;
} stateType;
void printState(stateType*);
void run(stateType*);
int  convertNum(int);
int main(int argc, char* argv[])
{
	int i;
	char line[MAXLINELENGTH];
	stateType* state = new stateType;
	FILE* filePtr;
	if (argc != 2) {
		printf("error: usage: %s <machine-code file>\n", argv[0]);
		exit(1);
	}/* initialize memories and registers */
	for (i = 0; i < NUMMEMORY; i++) {
		state->mem[i] = 0;
	}
	for (i = 0; i < NUMREGS; i++) {
		state->reg[i] = 0;
	}
	state->pc = 0;
	state->CF = 0;
	/* read machine-code file into instruction/data memory (starting ataddress 0) */
	filePtr = fopen(argv[1], "r");
	if (filePtr == NULL) {
		printf("error: can't open file %s\n", argv[1]);
		perror("fopen");
		exit(1);
	}
	for (state->numMemory = 0; fgets(line, MAXLINELENGTH, filePtr) != NULL;
		state->numMemory++) {
		if (state->numMemory >= NUMMEMORY) {
			printf("exceeded memory size\n");
			exit(1);
		}
		if (sscanf(line, "%lli", state->mem + state->numMemory) != 1) {
			printf("error in reading address %lli\n", state->numMemory);
			exit(1);
		}
		printf("memory[%lli]=%lld\n", state->numMemory, state->mem[state->numMemory]);
	}
	printf("\n");
	/* run never returns */
	run(state);
	return(0);
}
void run(stateType* state)
{
	long long int arg0, arg1, arg2, addressField, tmp;
	long long int max_value_reg = 8388608;
	long long int min_value_reg = -8388608;
	long long int instructions = 0;
	long long int opcode;
	long long int maxMem = -1;	/* highest memory address touched during run */
	for (; 1; instructions++) { /* infinite loop, exits when it executes halt */
		printState(state);
		if (state->pc < 0 || state->pc >= NUMMEMORY) {
			printf("pc went out of the memory range\n");
			exit(1);
		}
		maxMem = (state->pc > maxMem) ? state->pc : maxMem;
		/* this is to make the following code easier to read */
		opcode = state->mem[state->pc] >> 36;
		arg0 = (state->mem[state->pc] >> 30) & 0x3F;
		arg1 = (state->mem[state->pc] >> 24) & 0x3F;
		arg2 = state->mem[state->pc] & 0x3F; /* only for add, nand */
		addressField = convertNum(state->mem[state->pc] & 0xFFFF); /*for beq,lw, sw */
		state->pc++;
		if (opcode == ADD) {
			state->reg[arg2] = state->reg[arg0] + state->reg[arg1];
		}
		else if (opcode == ADC) {
			tmp = state->reg[arg0] + state->reg[arg1];
			if (tmp > max_value_reg) {
				state->reg[arg2] = max_value_reg;
				state->CF = tmp - max_value_reg;
			}
			else {
				state->reg[arg2] = tmp + state->CF;
			}
		}
		else if (opcode == SBB) {
			tmp = state->reg[arg0] - state->reg[arg1];
			if (tmp < min_value_reg) {
				state->reg[arg2] = min_value_reg;
				state->CF = abs(tmp - min_value_reg);
			}
			else {
				state->reg[arg2] = tmp - state->CF;
			}
		}
		else if (opcode == NAND) {
			state->reg[arg2] = ~(state->reg[arg0] & state->reg[arg1]);
		}
		else if (opcode == BEQ) {
			if (state->reg[arg0] == state->reg[arg1])
				state->pc += addressField;
		}
		else if (opcode == XSUB) {
			long long int temp = 0;
			temp = state->reg[arg0];
			state->reg[arg0] = state->reg[arg1];
			state->reg[arg1] = temp;
			state->reg[arg2] = state->reg[arg0] - state->reg[arg1];
		}
		else if (opcode == XIDIV) {
			long long int temp;
			temp = state->reg[arg1];
			state->reg[arg1] = state->reg[arg0];
			state->reg[arg0] = temp;
			state->reg[arg2] = state->reg[arg0] / state->reg[arg1];
		}
		else if (opcode == XADD) {
			long long int temp;
			temp = state->reg[arg1];
			state->reg[arg1] = state->reg[arg0];
			state->reg[arg0] = temp;
			state->reg[arg2] = state->reg[arg0] + state->reg[arg1];
		}
		else if (opcode == XOR) {
			state->reg[arg2] = state->reg[arg0] ^ state->reg[arg1];
		}
		else if (opcode == CMPE) {
			if (state->reg[arg0] == state->reg[arg1]) {
				state->reg[arg2] = true;
			}
			else {
				state->reg[arg2] = false;
			}
		}
		else if (opcode == SAR) {
			state->reg[arg2] = state->reg[arg0] >> state->reg[arg1];
		}
		else if (opcode == RCR) {
			state->CF = state->reg[arg0] >> state->reg[arg1];
			state->reg[arg2] = state->CF;
		}
		else if (opcode == JALR) {
			state->reg[arg1] = state->pc;
			if (arg0 != 0)
				state->pc = state->reg[arg0];
			else
				state->pc = 0;
		}
		else if (opcode == LW) {
			if (state->reg[arg0] + addressField < 0 ||
				state->reg[arg0] + addressField >= NUMMEMORY) {
				printf("address out of bounds\n");
				exit(1);
			}
			state->reg[arg1] = state->mem[state->reg[arg0] + addressField];
			if (state->reg[arg0] + addressField > maxMem) {
				maxMem = state->reg[arg0] + addressField;
			}
		}
		else if (opcode == LOAD) {
			if (addressField < 0 || addressField >= NUMMEMORY) {
				printf("address out of bounds\n");
				exit(1);
			}
			state->reg[arg0] = state->mem[addressField];
			if (addressField > maxMem) {
				maxMem = addressField;
			}
		}
		else if (opcode == STORE) {
			if (addressField < 0 || addressField >= NUMMEMORY) {
				printf("address out of bounds\n");
				exit(1);
			}
			state->mem[addressField] = state->reg[arg0];
			if (addressField > maxMem) {
				maxMem = addressField;
			}
		}
		else if (opcode == SW) {
			if (state->reg[arg0] + addressField < 0 ||
				state->reg[arg0] + addressField >= NUMMEMORY) {
				printf("address out of bounds\n");
				exit(1);
			}
			state->mem[state->reg[arg0] + addressField] = state->reg[arg1];
			if (state->reg[arg0] + addressField > maxMem) {
				maxMem = state->reg[arg0] + addressField;
			}
		}
		else if (opcode == JMA) {
			if ((unsigned)state->reg[arg0] > (unsigned)state->reg[arg1]) {
				state->pc += addressField;
			}

		}
		else if (opcode == JML) {
			if ((unsigned)state->reg[arg0] < (unsigned)state->reg[arg1]) {
				state->pc += addressField;
			}
		}
		else if (opcode == NOOP) {
		}
		else if (opcode == HALT) {
			printf("machine halted\n");
			printf("total of %lli instructions executed\n", instructions + 1);
			printf("final state of machine:\n");
			printState(state);
			exit(0);
		}
		else {
			printf("error: illegal opcode 0x%lli\n", opcode);
			exit(1);
		}
		state->reg[0] = 0;
	}
}
void printState(stateType* statePtr)
{
	int i;
	printf("\n@@@\nstate:\n");
	printf("\tpc %lli\n", statePtr->pc);
	printf("\tmemory:\n");
	for (i = 0; i < statePtr->numMemory; i++) {
		printf("\t\tmem[ %d ] %lld\n", i, statePtr->mem[i]);
	}
	printf("\tregisters:\n");
	for (i = 0; i < NUMREGS; i++) {
		printf("\t\treg[ %d ] %lli\n", i, statePtr->reg[i]);
	}
	printf("CF = %lli\n", statePtr->CF);
	printf("end state\n");
}
int convertNum(int num)
{
	if (num & (1 << 15)) {
		num -= (1 << 16);
	}
	return(num);
}
