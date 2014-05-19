#ifndef _SORTED_H_
#define _SORTED_H_

#include "Record.h"
#include "BF.h"

int Sorted_CreateFile(char* fileName);
int Sorted_OpenFile(char* fileName);
int Sorted_CloseFile(int fileDesc);
int Sorted_InsertEntry(int fileDesc,Record record);
int Sorted_DeleteEntry(int fileDesc,char* fieldName,void* value);
void Sorted_GetAllEntries(int fileDesc,char* fieldName,void* value);

#endif
