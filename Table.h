/*
 * Jeffrey Bour
 * Table.h contains the definitions of those
 * functions implemented in Table.c and utilized
 * by the two-pass linker in Linker.c.
 */

#ifndef _TABLE_H_INCLUDE_
#define _TABLE_H_INCLUDE_

typedef struct TableObj* Table;

Table newTable(void);

void freeTable(Table *pT);

int isEmpty(Table T);

int size(Table T);

void insertSymbol(Table T, char* s, int m, int b, int o);

int getAddress(Table def, char* s);

void checkSize(Table T, int size, int module);

void resetFlags(Table T);

void mark(Table T, char* s);

void makeEmpty(Table T);

void printTable(Table T);

void printWarnings(Table T);

#endif