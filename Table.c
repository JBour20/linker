/*
 * Jeffrey Bour
 * Table.c contains a linked-list data structure
 * used to create and store symbol tables.
 * Table.c lends its services to Linker.c.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "Table.h"

typedef struct SymbolObj {
	char* sym;
	int module;
	int baseAddress;
	int offset;
	int address;
	int flag;
	struct SymbolObj* next;
} SymbolObj;

typedef SymbolObj* Symbol;

Symbol newSymbol(char* s, int m, int b, int o) {
	Symbol S = malloc(sizeof(SymbolObj));
	assert(S!=NULL);
	S->sym = s;
	S->module = m;
	S->baseAddress = b;
	S->offset = o;
	S->address = b+o;
	S-> flag = 0;
	S->next = NULL;
	return S;
}

void freeSymbol(Symbol* pS) {
	if(pS!=NULL && *pS!=NULL){
		free(*pS);
		*pS = NULL;
	}
}

typedef struct TableObj {
	Symbol top;
	int count;
} TableObj;

Table newTable(void) {
	Table T = malloc(sizeof(TableObj));
	assert(T!=NULL);
	T->top = NULL;
	T->count = 0;
	return T;
}

void freeTable(Table *pT) {
	if(pT!=NULL && *pT!=NULL){
		if(!isEmpty(*pT)){
			makeEmpty(*pT);
		}
		free(*pT);
		*pT = NULL;
	}
}

int isEmpty(Table T) {
	if(T==NULL){
		fprintf(stderr, "Table Error: calling isEmpty() on NULL Table reference\n");
		exit(EXIT_FAILURE);
	}
	return (T->count==0);
}

int size(Table T) {
	if(T==NULL){
		fprintf(stderr, "Table Error: calling size() on NULL Table reference\n");
		exit(EXIT_FAILURE);
	}
	return T->count;
}

void insertSymbol(Table T, char* s, int m, int b, int o) {
	if(T==NULL){
		fprintf(stderr, "Table Error: calling insert() on NULL Table reference\n");
		exit(EXIT_FAILURE);
	}
	if(T->top==NULL) {
		T->top = newSymbol(s, m, b, o);
		T->count++;
	}
	else {
		Symbol S = T->top;
		while(S!=NULL) {
			if(strcmp(S->sym, s)==0) {
				S->flag = 1;
				S = NULL;
				break;
			} else {
				if(S->next==NULL) {
					S->next = newSymbol(s, m, b, o);
					S = NULL;
					T->count++;
					break;
				}
				else S = S->next;
			}
		}
	}
}

int contains(Table T, char* s) {
	if(T==NULL){
		fprintf(stderr, "Table Error: calling contains() on NULL Table reference\n");
		exit(EXIT_FAILURE);
	}
	Symbol S;
	for(S = T->top; S!=NULL; S = S->next) {
		if(strcmp(s, S->sym)==0) return 1;
	}
	return 0;
}

int getAddress(Table def, char* s) {
	if(def==NULL){
		fprintf(stderr, "Table Error: calling getAddress() on NULL Table reference\n");
		exit(EXIT_FAILURE);
	}
	Symbol S;
	for(S = def->top; S!=NULL; S = S->next) {
		if(strcmp(s, S->sym)==0) return S->address;
	}
	return -1;
}

void checkSize(Table T, int size, int module) {
	if(T==NULL){
		fprintf(stderr, "Table Error: calling checkModuleSize() on NULL Table reference\n");
		exit(EXIT_FAILURE);
	}
	Symbol S;
	for(S = T->top; S!=NULL; S = S->next) {
		if(S->module==module){
			if(S->offset>=size) {
				printf("Warning: Module %d: %s to big %d (max=%d) assume zero relative\n", module, S->sym, S->offset, size-1);
				S->offset = 0;
				S->address = (S->baseAddress + S->offset);
			}
		}
	}
}

void resetFlags(Table T) {
	if(T==NULL){
		fprintf(stderr, "Table Error: calling resetFlags() on NULL Table reference\n");
		exit(EXIT_FAILURE);
	}
	Symbol S;
	for(S = T->top; S!=NULL; S = S->next) {
		S->flag = 0;
	}
}

void mark(Table T, char* s) {
	if(T==NULL){
		fprintf(stderr, "Table Error: calling mark() on NULL Table reference\n");
		exit(EXIT_FAILURE);
	}
	Symbol S = T->top;
	for(S = T->top; S!=NULL; S = S->next) {
		if(strcmp(s, S->sym)==0) {
			S->flag++;
			break;
		}
	}
}

void makeEmpty(Table T) {
	if(T==NULL){
		fprintf(stderr, "Table Error: calling makeEmpty() on NULL Table reference\n");
		exit(EXIT_FAILURE);
	}
	if(T->count==0){
		fprintf(stderr, "Table Error: calling makeEmpty() on empty Table\n");
		exit(EXIT_FAILURE);
	}
	Symbol S;
	while(T->count>0){
		S = T->top;
		T->top = T->top->next;
		freeSymbol(&S);
		T->count--;
	}
	T->top = NULL;
}

void printTable(Table T) {
	if(T==NULL){
		fprintf(stderr, "Table Error: calling printTable() on NULL Table reference\n");
		exit(EXIT_FAILURE);
	}
	Symbol S;
	for(S = T->top; S!=NULL; S = S->next) {
		char* err = (S->flag==0) ? "" : "Error: This variable is multiple times defined; first value used";
		printf("%s=%d %s\n", S->sym, S->address, err);
	}
}

void printWarnings(Table T) {
	if(T==NULL){
		fprintf(stderr, "Table Error: calling printWarnings() on NULL Table reference\n");
		exit(EXIT_FAILURE);
	}
	Symbol S;
	int lineCount = 0;
	for(S = T->top; S!=NULL; S = S->next) {
		if(S->flag==0) {
			printf("\nWarning: Module %d: %s was defined but never used", S->module, S->sym);
			lineCount++;
		}
	}
	if(lineCount>0) printf("\n");
}