#include <stdio.h>
#include <string.h>

#include "BF.h"
#include "HP.h"
#include "Record.h"


int HP_CreateFile(char* fileName){
	int fileDesc;
	void *head;
	if(BF_CreateFile(fileName) < 0){
		BF_PrintError("The file could not be created..\n");
		return -1;
	}
	if((fileDesc=BF_OpenFile(fileName)) < 0){
		BF_PrintError("The file could not be opened..\n");
		BF_CloseFile(fileDesc);
		return -1;
	}
	if(BF_AllocateBlock(fileDesc) < 0){
		BF_PrintError("The block could not be allocated..\n");
		BF_CloseFile(fileDesc);
		return -1;
	}
	if(BF_ReadBlock(fileDesc,0,&head) < 0){
		BF_PrintError("Could not read the block..\n");
		BF_CloseFile(fileDesc);
		return -1;
	}

	*((int *) head) = 1;
	*((int *) head + 1) = -1;
	
	if(BF_WriteBlock(fileDesc,0) < 0){
		BF_PrintError("Could not write to this block..\n");
		BF_CloseFile(fileDesc);
		return -1;
	}
	if(BF_CloseFile(fileDesc) < 0){
		BF_PrintError("The file could not be closed..\n");
		return -1;
	}
	return 0;	
}


int HP_OpenFile(char* fileName){
	int fileDesc;
	void *head;
	if((fileDesc=BF_OpenFile(fileName)) < 0){	
		BF_PrintError("The file could not be opened..\n");
		return -1;
	} 
	/* diavazei apo to prwto block tin pliroforia pou afora to arxeio swrou */
	if(BF_ReadBlock(fileDesc,0,&head) < 0){
		BF_PrintError("BF_ReadBlock could not read the block....\n");
		BF_CloseFile(fileDesc);
		return -1;
	}
	if(*((int*) head) != 1){	
		printf("This is not a heap file..\n");
		BF_CloseFile(fileDesc);
		return -1;
	}
	return fileDesc;
}	


int HP_CloseFile(int fileDesc){
	if(BF_CloseFile(fileDesc) < 0){
		BF_PrintError("The file could not be closed..\n");
		return -1;
	}
	return 0;
}


int HP_InsertEntry(int fileDesc,Record record){
	void *block, *head;
	int bno,rno,i,j;
	Record tmpRec;
	
	if((bno = BF_GetBlockCounter(fileDesc)) < 0)
		return -1;
	/* το heap αρχείο περιέχει μόνο το block κεφαλίδα */
	if(bno == 1){
		/* Δημιουργία του πρώτου block */
		if(BF_AllocateBlock(fileDesc) < 0)  
			return -1;
		if(BF_ReadBlock(fileDesc,1,&block))
			return -1;
		/* Δεν έχει επόμενο */
		*((int *) block + 1) = -1;
		/* εγγραφή του στο δίσκο */
		if(BF_WriteBlock(fileDesc,1) < 0)
			return -1;
		/* 2 block στο heap αρχείο */
		++bno;
		/* ανανέωση του block κεφαλίδας */
		if(BF_ReadBlock(fileDesc,0,&head))
			return -1;
		/* το πρώτο block του heap αρχείου */
		*((int *) head + 1) = 1;
		if(BF_WriteBlock(fileDesc,0))
			return -1;
	}
	
	/* έλεγχος για επαναλαμβανόμενα records */
	for(i=1;i<bno;i++){
		/* μεταφορά του τελευταίου block του heap file στη μνήμη */
		if(BF_ReadBlock(fileDesc,i,&block))
			return -1;
		/* αριθμός εγγραφών που αυτό περιέχει */
		rno = *((int *) block);
		for(j=0;j<rno;j++){
			memcpy(&tmpRec,block + 2*sizeof(int) + j*sizeof(Record),sizeof(Record));
			if(tmpRec.id==record.id && !strcmp(tmpRec.name,record.name) && !strcmp(tmpRec.surname,record.surname) && !strcmp(tmpRec.city,record.city)){
				printf("The record already exists..\n");
				return -1;
			}
		}
		/* φέρνω τον δείκτη block στη σωστή θέση */
		if(!rno && i!=1){
			/* διαβάζω το προηγούνενο block */
			if(BF_ReadBlock(fileDesc,i-1,&block))
				return -1;
			/* πόσα records περιέχει */
			rno = *((int *) block);
			/* βρίσκω τον αριθμό τω άδειων block */
			bno = bno - (bno - i);
			break;
		}
	}
	
	/* εάν η νέα εγγραφή δεν χωράει στο block αυτό */
	if(2 * sizeof(int) + (rno + 1) * sizeof(Record) > BF_BLOCK_SIZE){
		/* αν υπάρχει άδειο διαθέσημο block */
		if(bno != BF_GetBlockCounter(fileDesc)){
			/* διαβάζουμε το επόμενο υπάρχον άδειο block */
			if(BF_ReadBlock(fileDesc,bno,&block))
				return -1;
		}
		else{
			/* δεσμεύουμε ενα νέο */
			if(BF_AllocateBlock(fileDesc))				
				return -1;
		}
		/* ανανεώνουμε το δείκτη next του προηγούμενου */
		*((int *) block + 1) = bno;
		/* και το γράφουμε στο δίσκο */
		if(BF_WriteBlock(fileDesc,bno-1))
			return -1;
		/* μεταφέρουμε το πρόσφατα δεσμευμένο block στη μνήμη */
		if(BF_ReadBlock(fileDesc,bno,&block))
			return -1;
		/* 0 εγγραφές στο νέο block */
		*((int *) block) = 0;
		/* Δεν έχει επόμενο */
		*((int *) block + 1) = -1;
		/* 0 εγγραφές στο νέο block */
		rno = 0;
		/* το id του πρόσφατα δεσμευμένου block */
		++bno;
	}
	/* εισαγωγή της εγγραφής στην κατάλληλη θέση */
	memcpy(block + 2*sizeof(int) + rno * sizeof(Record),&record,sizeof(Record));
	/* αύξηση του αριθμού των εγγραφών */
	*((int *) block) += 1;
	/* εγγραφή του στο δίσκο */
	if(BF_WriteBlock(fileDesc,bno-1))
		return -1;
	return 0;
}


void rrprint(Record record){
	printf("{ %-8d | %-15s | %-20s | %15s }\n",record.id,record.name,record.surname,record.city);
}


int HP_DeleteEntry(int fileDesc,char* fieldName,void* value){
	int bno,i,j,k,rno,lastrno;
	void *block,*lastblock;
	
	if(fieldName == NULL || value == NULL)
		return -1;
	if((bno = BF_GetBlockCounter(fileDesc)) == 1){
		fprintf(stderr,"There is only a header block in this file\n");
		return -1;
	}
	for(i=1;i<bno;i++){
		if(BF_ReadBlock(fileDesc,i,&block) < 0)
			return -1;
		/* number of records */
		rno = *((int *)block);												
		if(!strcmp(fieldName,"id")){
			for(j=0;j<rno;j++){
				if(*((int *)value) == ((Record *)((char*)block+2*sizeof(int)) + j)->id){
					printf("Διαγραφή της %d εγγραφής του %d block!\n",j,i);
					k=0;
					/* vres to prwto block pou na exei mesa eggrafes */
					do { 											
						if(BF_ReadBlock(fileDesc,bno-(++k),&lastblock))
							return -1;
						/* posa records exei mesa to teleutaio block */
						lastrno = *((int *)lastblock);				
					} while(*((int *)lastblock) == 0 && k != bno-1);
					printf("Αντικατάσταση της με την %d εγγραφή του %d block!\n",lastrno-1,bno-k);
					printf("Διαγραφή απο το block %d!\n",bno-k);
					memcpy(block+2*sizeof(int)+j*sizeof(Record),lastblock+2*sizeof(int)+(lastrno-1)*sizeof(Record),sizeof(Record));
					if(BF_WriteBlock(fileDesc,i))
						return -1;
					memset(lastblock+2*sizeof(int)+(lastrno-1)*sizeof(Record),'\0',sizeof(Record));
					*((int *)lastblock) -= 1;
					if(BF_WriteBlock(fileDesc,bno-k))
						return -1;
					/*elenkse kai thn eggrafh pou metakinhses twra (sto epomeno loop to j tha einai idio)*/
					--j;	
				}
			}
		}
		else if(!strcmp(fieldName,"name")){
			for(j=0;j<rno;j++){
				if(!strcmp((char *)value,((Record *)((char*)block+2*sizeof(int)) + j)->name)){
					printf("Διαγραφή της %d εγγραφής του %d block!\n",j,i);
					k=0;
					//vres to prwto block 3ekinwntas apo to telos pou exei mesa mia eggrafh
					do {											
						if(BF_ReadBlock(fileDesc,bno-(++k),&lastblock))
							return -1;
						/* posa records exei mesa to teleutaio block */
						lastrno = *((int *)lastblock);				
					} while(*((int *)lastblock) == 0 && k != bno-1);
					printf("Αντικατάσταση της με την %d εγγραφή του %d block!\n",lastrno-1,bno-k);
					printf("Διαγραφή απο το block %d!\n",bno-k);
					memcpy(block+2*sizeof(int)+j*sizeof(Record),lastblock+2*sizeof(int)+(lastrno-1)*sizeof(Record),sizeof(Record));
					if(BF_WriteBlock(fileDesc,i))
						return -1;
					memset(lastblock+2*sizeof(int)+(lastrno-1)*sizeof(Record),'\0',sizeof(Record));
					*((int *)lastblock) -= 1;
					if(BF_WriteBlock(fileDesc,bno-k))
						return -1;
					/*elenkse kai thn eggrafh pou metakinhses twra (sto epomeno loop to j tha einai idio)*/
					--j;	
				}
			}
		}
		else if(!strcmp(fieldName,"surname")){
			for(j=0;j<rno;j++){
				if(!strcmp((char *)value,((Record *)((char*)block+2*sizeof(int)) + j)->surname)){
					printf("Διαγραφή της %d εγγραφής του %d block!\n",j,i);
					k=0;
					/* vres to prwto block 3ekinwntas apo to telos pou exei mesa mia eggrafh */
					do {											
						if(BF_ReadBlock(fileDesc,bno-(++k),&lastblock))
							return -1;
						/* posa records exei mesa to teleutaio block */
						lastrno = *((int *)lastblock);				
					} while(*((int *)lastblock) == 0 && k != bno-1);
					printf("Αντικατάσταση της με την %d εγγραφή του %d block!\n",lastrno-1,bno-k);
					printf("Διαγραφή απο το block %d!\n",bno-k);
					memcpy(block+2*sizeof(int)+j*sizeof(Record),lastblock+2*sizeof(int)+(lastrno-1)*sizeof(Record),sizeof(Record));
					if(BF_WriteBlock(fileDesc,i))
						return -1;
					memset(lastblock+2*sizeof(int)+(lastrno-1)*sizeof(Record),'\0',sizeof(Record));
					*((int *)lastblock) -= 1;
					if(BF_WriteBlock(fileDesc,bno-k))
						return -1;
					/*elenkse kai thn eggrafh pou metakinhses twra (sto epomeno loop to j tha einai idio)*/
					--j;	
				}
			}
		}
		else if(!strcmp(fieldName,"city")){
			for(j=0;j<rno;j++){
				if(!strcmp((char *)value,((Record *)((char*)block+2*sizeof(int)) + j)->city)){
					printf("Διαγραφή της %d εγγραφής του %d block!\n",j,i);
					k=0;
					/* vres to prwto block 3ekinwntas apo to telos pou exei mesa mia eggrafh */
					do {											
						if(BF_ReadBlock(fileDesc,bno-(++k),&lastblock))
							return -1;
						/* posa records exei mesa to teleutaio block */
						lastrno = *((int *)lastblock);				
					} while(*((int *)lastblock) == 0 && k != bno-1);
					printf("Αντικατάσταση της με την %d εγγραφή του %d block!\n",lastrno-1,bno-k);
					printf("Διαγραφή απο το block %d!\n",bno-k);
					memcpy(block+2*sizeof(int)+j*sizeof(Record),lastblock+2*sizeof(int)+(lastrno-1)*sizeof(Record),sizeof(Record));
					if(BF_WriteBlock(fileDesc,i))
						return -1;
					memset(lastblock+2*sizeof(int)+(lastrno-1)*sizeof(Record),'\0',sizeof(Record));
					*((int *)lastblock) -= 1;
					if(BF_WriteBlock(fileDesc,bno-k))
						return -1;
					/* elenkse kai thn eggrafh pou metakinhses twra (sto epomeno loop to j tha einai idio) */
					--j;	
				}
			}
		}
		else {
			printf("invalid!\n");
			return -1;
		}
	}
	return 0;
}


void HP_GetAllEntries(int fileDesc,char* fieldName,void* value){
	int bno,i,rno,j;
	void *block;

	bno = BF_GetBlockCounter(fileDesc);
	if(value == NULL || fieldName == NULL){
		/* gia ola ta block */
		for(i=1;i<bno;i++){
			if(BF_ReadBlock(fileDesc,i,&block))
				return;
			/* posa records exei mesa */
			rno = *((int *)block);
			printf("\E[31mBlock %d\E[0m (\E[34m%d records\E[0m)\n",i,rno);
			/* prosperna ta 2 int me plirofories */
			block += 2*sizeof(int);
			for(j=0;j<rno;j++){
				printf("%2d : ",j);
				printf("%-d %-15s %-20s %-10s\n",((Record *)block+j)->id,((Record *)block+j)->name,((Record *)block+j)->surname,((Record *)block+j)->city);
			}
		}
	}
	else if(!strcmp(fieldName,"id")){
		for(i=1;i<bno;i++){
			if(BF_ReadBlock(fileDesc,i,&block))
				return;
			rno = *((int *)block);
			printf("\E[31mBlock %d\E[0m (\E[34m%d records\E[0m)\n",i,rno);
			/* prosperna ta 2 int me plirofories */
			block += 2*sizeof(int);
			for(j=0;j<rno;j++){
				if(*((int *)value) == ((Record *)block+j)->id){
					printf("%2d : ",j);
					printf("%-d %-15s %-20s %-10s\n",((Record *)block+j)->id,((Record *)block+j)->name,((Record *)block+j)->surname,((Record *)block+j)->city);
				}
			}
		}	
	}
	else if(!strcmp(fieldName,"name")){
		for(i=1;i<bno;i++){
			if(BF_ReadBlock(fileDesc,i,&block))
				return;
			rno = *((int *)block);
			printf("\E[31mBlock %d\E[0m (\E[34m%d records\E[0m)\n",i,rno);
			/* prosperna ta 2 int me plirofories */
			block += 2*sizeof(int);
			for(j=0;j<rno;j++){
				if(!strcmp(((Record *)block+j)->name,(char *)value)){
					printf("%2d : ",j);
					printf("%-d %-15s %-20s %-10s\n",((Record *)block+j)->id,((Record *)block+j)->name,((Record *)block+j)->surname,((Record *)block+j)->city);
				}
			}
		}
	}
	else if(!strcmp(fieldName,"surname")){
		for(i=1;i<bno;i++){
			if(BF_ReadBlock(fileDesc,i,&block))
				return;
			rno = *((int *)block);
			printf("\E[31mBlock %d\E[0m (\E[34m%d records\E[0m)\n",i,rno);
			/* prosperna ta 2 int me plirofories */
			block += 2*sizeof(int);
			for(j=0;j<rno;j++){
				if(!strcmp(((Record *)block+j)->surname,(char*)value)){
					printf("%2d : ",j);
					printf("%-d %-15s %-20s %-10s\n",((Record *)block+j)->id,((Record *)block+j)->name,((Record *)block+j)->surname,((Record *)block+j)->city);
				}
			}
		}
	}
	else if(!strcmp(fieldName,"city")){
		for(i=1;i<bno;i++){
			if(BF_ReadBlock(fileDesc,i,&block))
				return;
			rno = *((int *)block);
			printf("\E[31mBlock %d\E[0m (\E[34m%d records\E[0m)\n",i,rno);
			/* prosperna ta 2 int me plirofories */
			block += 2*sizeof(int);
			for(j=0;j<rno;j++){
				if(!strcmp(((Record *)block+j)->city,(char*)value)){
					printf("%2d : ",j);
					printf("%-d %-15s %-20s %-10s\n",((Record *)block+j)->id,((Record *)block+j)->name,((Record *)block+j)->surname,((Record *)block+j)->city);
				}
			}
		}
	}
	else{
		printf("Invalid fieldname\n");
	}
	printf("Διαβάστηκαν %d blocks\n",bno-1);
}
