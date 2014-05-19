#ifndef _HP_
#define _HP_

#include "Record.h"
#include "BF.h"

int HP_CreateFile(char* fileName);
int HP_OpenFile(char* fileName);
int HP_CloseFile(int fileDesc);
int HP_InsertEntry(int fileDesc,Record record);
int HP_DeleteEntry(int fileDesc,char* fieldName,void* value);
void HP_GetAllEntries(int fileDesc,char* fieldName,void* value);

#endif
