#define _CRT_SECURE_NO_WARNINGS
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define MAXNUMLABELS 65536
#define MAXLABELLENGTH 7 /* includes the null character termination */
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



long long int readAndParse(FILE*, char*, char*, char*, char*, char*);
long long int translateSymbol(char labelArray[MAXNUMLABELS][MAXLABELLENGTH], long long int labelAddress[MAXNUMLABELS], long long int, char*);
int isNumber(char*);
void testRegArg(char*);
void testAddrArg(char*);

int main(int argc, char* argv[])
{
	char* inFileString, * outFileString;
	FILE* inFilePtr, * outFilePtr;
	long long int address;
	char label[MAXLINELENGTH], opcode[MAXLINELENGTH], arg0[MAXLINELENGTH],
		arg1[MAXLINELENGTH], arg2[MAXLINELENGTH], argTmp[MAXLINELENGTH];
	int i;
	long long int numLabels = 0;
	long long int num;
	long long int addressField;

	char labelArray[MAXNUMLABELS][MAXLABELLENGTH];
	long long int labelAddress[MAXNUMLABELS];

	if (argc != 3) {
		printf("error: usage: %s <assembly-code-file> <machine-code-file>\n",
			argv[0]);
		exit(1);
	}

	inFileString = argv[1];
	outFileString = argv[2];

	inFilePtr = fopen(inFileString, "r");
	if (inFilePtr == NULL) {
		printf("error in opening %s\n", inFileString);
		exit(1);
	}
	outFilePtr = fopen(outFileString, "w");
	if (outFilePtr == NULL) {
		printf("error in opening %s\n", outFileString);
		exit(1);
	}

	/* map symbols to addresses */

	/* assume address start at 0 */
	for (address = 0; readAndParse(inFilePtr, label, opcode, arg0, arg1, arg2); address++) {
		/*
printf("%d: label=%s, opcode=%s, arg0=%s, arg1=%s, arg2=%s\n",
		address, label, opcode, arg0, arg1, arg2);
		*/

		/* check for illegal opcode */
		if (strcmp(opcode, "add") && strcmp(opcode, "load") && strcmp(opcode, "store") && strcmp(opcode, "nand") && strcmp(opcode, "xsub") && strcmp(opcode, "xidiv") && strcmp(opcode, "xadd") && strcmp(opcode, "beq") &&
			strcmp(opcode, "jalr") && strcmp(opcode, "xor") && strcmp(opcode, "cmpe") &&
			strcmp(opcode, "sar") && strcmp(opcode, "rcr") && strcmp(opcode, "jma") && strcmp(opcode, "jml") && strcmp(opcode, "adc") && strcmp(opcode, "sbb") && strcmp(opcode, "rcr") &&
			strcmp(opcode, "lw") && strcmp(opcode, "sw") && strcmp(opcode, "halt") &&
			strcmp(opcode, "noop") && strcmp(opcode, ".fill")) {
			printf("error: unrecognized opcode %s at address %lli\n", opcode, address);
			exit(1);
		}

		/* check register fields */
		if (!strcmp(opcode, "add") || !strcmp(opcode, "adc") || !strcmp(opcode, "nand") ||
			!strcmp(opcode, "xidiv") || !strcmp(opcode, "sbb") || !strcmp(opcode, "xsub") ||
			!strcmp(opcode, "xor") || !strcmp(opcode, "and") || !strcmp(opcode, "xidiv") ||
			!strcmp(opcode, "cmpe") || !strcmp(opcode, "sar") || !strcmp(opcode, "rcr") ||
			!strcmp(opcode, "jma") || !strcmp(opcode, "jml") ||
			!strcmp(opcode, "beq") || !strcmp(opcode, "jalr") ||
			!strcmp(opcode, "lw") || !strcmp(opcode, "sw")) {
			testRegArg(arg0);
			testRegArg(arg1);
		}
		if (!strcmp(opcode, "load") || !strcmp(opcode, "store")) {
			testRegArg(arg0);
		}
		if (!strcmp(opcode, "add") || !strcmp(opcode, "adc") || !strcmp(opcode, "sbb") || !strcmp(opcode, "nand") || !strcmp(opcode, "xsub") ||
			!strcmp(opcode, "xadd") || !strcmp(opcode, "cmpe")) {
			testRegArg(arg2);
		}
		if (!strcmp(opcode, "beq") || !strcmp(opcode, "xor") ||
			!strcmp(opcode, "lw") || !strcmp(opcode, "sw") ||
			!strcmp(opcode, "jma") || !strcmp(opcode, "jml")) {
			testAddrArg(arg2);
		}
		if (!strcmp(opcode, ".fill")) {
			testAddrArg(arg0);
		}/* check for enough arguments */

		/* check for enough arguments */
		if ((strcmp(opcode, "halt") && strcmp(opcode, "noop") && strcmp(opcode, "load") && strcmp(opcode, "store") &&
			strcmp(opcode, ".fill") && strcmp(opcode, "jalr") && arg2[0] == '\0') ||
			(!strcmp(opcode, "jalr") && arg1[0] == '\0') ||
			(!strcmp(opcode, ".fill") && arg0[0] == '\0')) {
			printf("error at address %lli: not enough arguments\n", address);
			exit(2);
		}
		if (label[0] != '\0') {
			/* check for labels that are too long */
			if (strlen(label) >= MAXLABELLENGTH) {
				printf("label too long\n");
				exit(2);
			}
			/* make sure label starts with letter */
			if (!sscanf(label, "%[a-zA-Z]", argTmp)) {
				printf("label doesn't start with letter\n");
				exit(2);
			}
			/* make sure label consists of only letters and numbers */
			sscanf(label, "%[a-zA-Z0-9]", argTmp);
			if (strcmp(argTmp, label)) {
				printf("label has character other than letters and numbers\n");
				exit(2);
			}
			/* look for duplicate label */
			for (i = 0; i < numLabels; i++) {
				if (!strcmp(label, labelArray[i])) {
					printf("error: duplicate label %s at address %lli\n", label, address);
					exit(1);
				}
			}
			/* see if there are too many labels */
			if (numLabels >= MAXNUMLABELS) {
				printf("error: too many labels (label=%s)\n", label);
				exit(2);
			}
			strcpy(labelArray[numLabels], label);
			labelAddress[numLabels++] = address;
		}
	}

	for (i = 0; i < numLabels; i++) {
		/* printf("%s = %d\n", labelArray[i], labelAddress[i]); */
	}

	/* now do second pass (print machine code, with symbols filled in as
	addresses) */
	rewind(inFilePtr);
	for (address = 0; readAndParse(inFilePtr, label, opcode, arg0, arg1, arg2); address++) {
		if (!strcmp(opcode, "xsub")) {
			num = ((long long int)XSUB << 36) | (atoll(arg0) << 30) | (atoll(arg1) << 24)
				| atoll(arg2);//KOP(61-60),arg0(59-54),arg1(48-53),offset(0-47)/arg2(5-0)
		}
		else if (!strcmp(opcode, "xidiv")) {
			num = ((long long int)XIDIV << 36) | (atoll(arg0) << 30) | (atoll(arg1) << 24)\
				| atoll(arg2);
		}
		else if (!strcmp(opcode, "xadd")) {
			num = ((long long int)XADD << 36) | (atoll(arg0) << 30) | (atoll(arg1) << 24) | atoll(arg2);
		}
		else if (!strcmp(opcode, "add")) {
			num = ((long long int)ADD << 36) | (atoll(arg0) << 30) | (atoll(arg1) << 24) | atoll(arg2);
		}
		else if (!strcmp(opcode, "adc")) {
			num = ((long long int)ADC << 36) | (atoll(arg0) << 30) | (atoll(arg1) << 24) | atoll(arg2);
		}
		else if (!strcmp(opcode, "sbb")) {
			num = ((long long int)SBB << 36) | (atoll(arg0) << 30) | (atoll(arg1) << 24) | atoll(arg2);
		}
		else if (!strcmp(opcode, "nand")) {
			num = ((long long int)NAND << 36) | (atoll(arg0) << 30) | (atoll(arg1) << 24) | atoll(arg2);
		}
		else if (!strcmp(opcode, "jalr")) {
			num = ((long long int)JALR << 36) | (atoll(arg0) << 30) | (atoll(arg1) << 24);
		}
		else if (!strcmp(opcode, "xor")) {
			num = ((long long int)XOR << 36) | (atoll(arg0) << 30) | (atoll(arg1) << 24) | atoll(arg2);
		}
		else if (!strcmp(opcode, "cmpe")) {
			num = ((long long int)CMPE << 36) | (atoll(arg0) << 30) | (atoll(arg1) << 24) | atoll(arg2);
		}
		else if (!strcmp(opcode, "sar")) {
			num = ((long long int)SAR << 36) | (atoll(arg0) << 30) | (atoll(arg1) << 24) | atoll(arg2);
		}
		else if (!strcmp(opcode, "rcr")) {
			num = ((long long int)RCR << 36) | (atoll(arg0) << 30) | (atoll(arg1) << 24) | atoll(arg2);
		}
		else if (!strcmp(opcode, "halt")) {
			num = ((long long int)HALT << 36);
		}
		else if (!strcmp(opcode, "noop")) {
			num = ((long long int)NOOP << 36);
		}
		else if (!strcmp(opcode, "lw") || !strcmp(opcode, "sw") ||
			!strcmp(opcode, "beq") || !strcmp(opcode, "jma") || !strcmp(opcode, "jml")) {
			/* if arg2 is symbolic, then translate into an address */
			if (!isNumber(arg2)) {
				addressField = translateSymbol(labelArray, labelAddress, numLabels, arg2);
				if (!strcmp(opcode, "beq") || !strcmp(opcode, "jma") || !strcmp(opcode, "jml")) {
					addressField = addressField - address - 1;
				}
			}
			else {
				addressField = atoll(arg2);
			}
			if (addressField < -8388608 || addressField > 8388608) {
				printf("error: offset %lli out of range\n", addressField);
				exit(1);
			}/* truncate the offset field, in case it's negative */
			/* truncate the offset field, in case it's negative */
			addressField = addressField & 0xFFFF;
			if (!strcmp(opcode, "beq")) {
				num = ((long long int)BEQ << 36) | (atoll(arg0) << 30) | (atoll(arg1) << 24) | addressField;
			}
			else if (!strcmp(opcode, "lw")) {
				num = ((long long int) LW << 36) | (atoll(arg0) << 30) | (atoll(arg1) << 24) | addressField;
			}
			else if (!strcmp(opcode, "sw")) {
				num = ((long long int) SW << 36) | (atoll(arg0) << 30) | (atoll(arg1) << 24) | addressField;
			}
			else if (!strcmp(opcode, "jma")) {
				num = ((long long int) JMA << 36) | (atoll(arg0) << 30) | (atoll(arg1) << 24) | addressField;
			}
			else if (!strcmp(opcode, "jml")) {
				num = ((long long int) JML << 36) | (atoll(arg0) << 30) | (atoll(arg1) << 24) | addressField;
			}
		}
		else if (!strcmp(opcode, ".fill")) {
			if (!isNumber(arg0)) {
				num = translateSymbol(labelArray, labelAddress, numLabels, arg0);
			}
			else {
				num = atoll(arg0);
				if (num >= 0) {
					num = num & 0xFFFFFFFFFFFF;
				}
				else {

					num = num & 0xFFFFFFFFFFFF;
					num = num | 0xFFFF000000000000;

				}
			}
		}
		else if (!strcmp(opcode, "load") || !strcmp(opcode, "store")) {
			if (!isNumber(arg1)) {
				addressField = translateSymbol(labelArray, labelAddress, numLabels, arg1);
			}
			else {
				addressField = atoll(arg1);
			}
			if (addressField < -8388608 || addressField > 8388608) {
				printf("error: offset %lli out of range\n", addressField);
				exit(1);
			}
			addressField = addressField & 0xFFFF;
			if (!strcmp(opcode, "load")) {
				num = ((long long int) LOAD << 36) | (atoll(arg0) << 30) | addressField;
			}
			else if (!strcmp(opcode, "store")) {
				num = ((long long int) STORE << 36) | (atoll(arg0) << 30) | addressField;
			}
		}
		fprintf(outFilePtr, "%lli\n", num);
	}
	exit(0);
}
/*
 * Read and parse a line of the assembly-language file.  Fields are returned
 * in label, opcode, arg0, arg1, arg2 (these strings must have memory already
 * allocated to them).
 *
 * Return values:
 *     0 if reached end of file
 *     1 if all went well
 *
 * exit(1) if line is too long.
 */
long long int readAndParse(FILE* inFilePtr, char* label, char* opcode, char* arg0,
	char* arg1, char* arg2)
{
	char line[MAXLINELENGTH];
	char* ptr = line;
	/* delete prior values */
	label[0] = opcode[0] = arg0[0] = arg1[0] = arg2[0] = '\0';
	/* read the line from the assembly-language file */
	if (fgets(line, MAXLINELENGTH, inFilePtr) == NULL) {
		/* reached end of file */
		return(0);
	}

	/* check for line too long */
	if (strlen(line) == MAXLINELENGTH - 1) {
		printf("error: line too long\n");
		exit(1);
	}
	/* is there a label? */
	ptr = line;
	if (sscanf(ptr, "%[^\t\n ]", label)) {
		/* successfully read label; advance pointer over the label */
		ptr += strlen(label);
	}

	/*
	 * Parse the rest of the line.  Would be nice to have real regular
	 * expressions, but scanf will suffice.
	 */
	sscanf(ptr, "%*[\t\n\r ]%[^\t\n\r ]%*[\t\n\r ]%[^\t\n\r ]%*[\t\n\r ]%[^\t\n\r ]%*[\t\n\r ]%[^\t\n\r ]",
		opcode, arg0, arg1, arg2);
	return(1);
}

long long int translateSymbol(char labelArray[MAXNUMLABELS][MAXLABELLENGTH],
	long long int labelAddress[MAXNUMLABELS], long long int numLabels, char* symbol)
{
	int i;

	/* search through address label table */
	for (i = 0; i < numLabels && strcmp(symbol, labelArray[i]); i++) {
	}

	if (i >= numLabels) {
		printf("error: missing label %s\n", symbol);
		exit(1);
	}
	return(labelAddress[i]);
}
int isNumber(char* string)
{
	/* return 1 if string is a number */
	int i;
	return((sscanf(string, "%d", &i)) == 1);
}
void testRegArg(char* arg)
{
	long long int num;
	char c;
	if (atoll(arg) < 0 || atoll(arg) > 63) {
		printf("error: register out of range\n");
		exit(2);
	}
	if (sscanf(arg, "%lli%c", &num, &c) != 1) {
		printf("bad character in register argument\n");
		exit(2);
	}
}
void testAddrArg(char* arg)
{
	long long int num;
	char c;
	/* test numeric addressField */
	if (isNumber(arg)) {
		if (sscanf(arg, "%lli%c", &num, &c) != 1) {
			printf("bad character in addressField\n");
			exit(2);
		}
	}
}
