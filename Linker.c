/*
 * Jeffrey Bour
 * Linker.c contains the main portion of my solution
 */

#include<stdio.h>
#include<stdlib.h>
#include<ctype.h>
#include<string.h>
#include"Table.h"

int baseaddress;
int modulecount;

int linenum;
int lineoffset;
int offsetbuffer;

char c;

int numbuffer;
char addressbuffer;
char symbuffer[20];

Table symtable;

char uselist[16][20];
int uselist_flags[16];
int uselist_size;

FILE* in;

void __parseerror(int errcode) {
	static char* errstr[] = {
		"NUM_EXPECTED",
		"SYM_EXPECTED",
		"ADDR_EXPECTED",
		"SYM_TOLONG",
		"TO_MANY_DEF_IN_MODULE",
		"TO_MANY_USE_IN_MODULE",
		"TO_MANY_INSTR",
	};
	printf("Parse Error line %d offset %d: %s\n",linenum,lineoffset,errstr[errcode]);
}

void clearUselist() {
	int i;
	for(i = 0; i<16; i++) {
		memset(uselist[i], 0, 32);
		uselist_flags[i] = 0;
	}
	uselist_size = 0;
}

void modWarnings() {
	int i;
	for(i = 0; i<uselist_size; i++) {
		if(uselist_flags[i]==0) printf("Warning: Module %d: %s appeared in the uselist but was not actually used\n", modulecount, uselist[i]);
	}
}

void advance() {
	lineoffset+=offsetbuffer;
	char prev;
	int lastlen = lineoffset;
	while(isspace(c)) {
		if(c==' ') {
			lineoffset++;
		}
		else if (c=='\t' || c=='\n') {
			linenum++;
			lineoffset = 1;
		}
		prev = c;
		c = fgetc(in);
		if(c==EOF) {
			if(prev==' ') lineoffset--;
			else if (prev=='\t' || prev=='\n') {
				linenum--;
				lineoffset = lastlen;
			}
		}
		lastlen = lineoffset;
	}
}

void readInt() {
	offsetbuffer = 0;
	numbuffer = 0;
	if(isdigit(c)) {
		numbuffer = (c-48);
		offsetbuffer++;
	} else {
		__parseerror(0);
		exit(EXIT_FAILURE);
	}
	while(!isspace(c) && !feof(in)) {
		c = fgetc(in);
		if(!isspace(c) && c!=EOF) {
			if(isdigit(c)) {
				numbuffer = (numbuffer*10) + (c-48);
				offsetbuffer++;
			} else {
				__parseerror(0);
				exit(EXIT_FAILURE);
			}
		}
	}
}

void readSym() {
	offsetbuffer = 0;
	memset(symbuffer, 0, strlen(symbuffer));
	if(isalpha(c)) {
		symbuffer[offsetbuffer] = c;
		offsetbuffer++;
	} else {
		__parseerror(1);
		exit(EXIT_FAILURE);
	}
	while(!isspace(c) && !feof(in)) {
		c = fgetc(in);
		if(!isspace(c) && c!=EOF) {
			if (isalnum(c)) {
				symbuffer[offsetbuffer] = c;
				offsetbuffer++;
			} else {
				__parseerror(1);
				exit(EXIT_FAILURE);
			}
		}
		if(strlen(symbuffer)>16) {
			__parseerror(3);
			exit(EXIT_FAILURE);
		}
	}
}

void readAddress() {
	offsetbuffer = 0;
	if(c=='A' || c=='I' || c=='E' || c=='R') {
		addressbuffer = c;
		c = fgetc(in);
		if(!isspace(c)) {
			__parseerror(2);
			exit(EXIT_FAILURE);
		} else offsetbuffer++;
	} else {
		__parseerror(2);
		exit(EXIT_FAILURE);
	}
}

void readDefList_1() {
	readInt();
	int numdefs = numbuffer;
	if(numdefs>16) {
		__parseerror(4);
		exit(EXIT_FAILURE);
	}
	advance();
	int i;
	for(i = 0; i<numdefs; i++) {
		readSym();
		advance();
		readInt();
		advance();
		insertSymbol(symtable, strdup(symbuffer), modulecount, baseaddress, numbuffer);
	}
}

void readUseList_1() {
	readInt();
	int numuse = numbuffer;
	if(numuse>16) {
		__parseerror(5);
		exit(EXIT_FAILURE);
	}
	advance();
	int i;
	for(i = 0; i<numuse; i++) {
		readSym();
		advance();
	}
}

void readInstList_1() {
	readInt();
	int numinst = numbuffer;
	baseaddress+=numinst;
	if(baseaddress>512) {
		__parseerror(6);
		exit(EXIT_FAILURE);
	}
	advance();
	int i;
	for(i = 0; i<numinst; i++) {
		readAddress();
		advance();
		readInt();
		advance();
	}
	checkSize(symtable, numinst, modulecount);
}

void readDefList_2() {
	readInt();
	int numdefs = numbuffer;
	advance();
	int i;
	for(i = 0; i<numdefs; i++) {
		readSym();
		advance();
		readInt();
		advance();
	}
}

void readUseList_2() {
	readInt();
	uselist_size = numbuffer;
	advance();
	int i;
	for(i = 0; i<uselist_size; i++) {
		readSym();
		strcpy(uselist[i], strdup(symbuffer));
		mark(symtable, uselist[i]);
		advance();
	}
}

void readInstList_2() {
	readInt();
	int numinst = numbuffer;
	advance();
	int i;
	for(i = baseaddress; i<baseaddress+numinst; i++) {
		readAddress();
		advance();
		readInt();
		advance();
		printf("%03d: ", i);
		if(numbuffer>9999) {
			if(addressbuffer=='I') printf("9999 Error: Illegal immediate value; treated as 9999\n");
			else printf("9999 Error: Illegal opcode; treated as 9999\n");
		}
		else {
			int operand = (numbuffer%1000);
			int opcode = (numbuffer/1000);
			if(addressbuffer=='A') {
				if(operand>512) printf("%04d Error: Absolute address exceeds machine size; zero used\n", (opcode*1000));
				else printf("%04d\n", numbuffer);
			} else if(addressbuffer=='E') {
				if(uselist_size<=operand) printf("%04d Error: External address exceeds length of uselist; treated as immediate\n", numbuffer);
				else {
					int addr = getAddress(symtable, uselist[operand]);
					uselist_flags[operand]++;
					if(addr==-1) printf("%04d Error: %s is not defined; zero used\n", (opcode*1000), uselist[operand]);
					else printf("%04d\n", (opcode*1000)+addr);
				}
			} else if(addressbuffer=='I') {
				printf("%04d\n", numbuffer);
			} else if(addressbuffer=='R') {
				if(operand>numinst) printf("%04d Error: Relative address exceeds module size; zero used\n", (opcode*1000)+baseaddress);
				else printf("%04d\n", numbuffer+baseaddress);
			}
		}
	}
	modWarnings();
	baseaddress+=numinst;
	clearUselist();
}

void createModule() {
	readDefList_1();
	readUseList_1();
	readInstList_1();
}

void resolveAddresses() {
	readDefList_2();
	readUseList_2();
	readInstList_2();
}

void readFile_1() {
	while(!feof(in)) {
		createModule();
		modulecount++;
	}
}

void readFile_2() {
	while(!feof(in)) {
		resolveAddresses();
		modulecount++;
	}
}

void pass_1_init(){
	symtable = newTable();
	modulecount = 1;
	baseaddress = 0;
	linenum = 1;
	lineoffset = 1;
	c = fgetc(in);
	advance();
	readFile_1();
	printf("Symbol Table\n");
	printTable(symtable);
}

void pass_2_init(){
	rewind(in);
	resetFlags(symtable);
	modulecount = 1;
	baseaddress = 0;
	linenum = 1;
	lineoffset = 1;
	c = fgetc(in);
	advance();
	printf("\nMemory Map\n");
	readFile_2();
	printWarnings(symtable);
}

int main(int argc, char* argv[]) {
	if(argc != 2) {
		printf("Usage: %s <input file>\n", argv[0]);
		exit(EXIT_FAILURE);
	}
	in = fopen(argv[1], "r");
	if(in==NULL) {
		printf("Unable to read from file \"%s\"", argv[1]);
		exit(EXIT_FAILURE);
	}
	pass_1_init();
	pass_2_init();
	freeTable(&symtable);
	fclose(in);
	return(EXIT_SUCCESS);
}