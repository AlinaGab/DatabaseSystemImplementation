#include <stdio.h>
#include <string.h>

#include "BF.h"
#include "Sorted.h"
#include "Record.h"


int Sorted_CreateFile(char* fileName){
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
	
	*((int *) head) = 2;
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


int Sorted_OpenFile(char* fileName){
	int fileDesc;
	void* head;
	if((fileDesc = BF_OpenFile(fileName)) < 0){	
		BF_PrintError("The file could not be opened..\n");
		return -1;
	} 
	/* diavazei apo to prwto block tin pliroforia pou afora to taksinomineno arxeio */
	if(BF_ReadBlock(fileDesc,0,&head) < 0){
		BF_PrintError("Could not read the block..\n");
		BF_CloseFile(fileDesc);
		return -1;
	}
	if(*((int *) head) != 2){
		printf("This is not a sorted file..\n");
		BF_CloseFile(fileDesc);
		return -1;
	}
	return fileDesc;
}


int Sorted_CloseFile(int fileDesc){
	if(BF_CloseFile(fileDesc) < 0){
		BF_PrintError("The file could not be closed..\n");
		return-1;
	}
	return 0;
}




int Sorted_InsertEntry(int fileDesc,Record record){
	void *block,*pblock;
	int low,high,middle,n,m,i,j,bno,full;
	Record tmp;	
	
	bno = BF_GetBlockCounter(fileDesc);
	
	/* elegxos gia epanalamvanonena records */
	for(i=1;i<bno;i++){
		/* metafora tou teleutaiou block tou file sti mnimi */
		if(BF_ReadBlock(fileDesc,i,&block))
			return -1;
		/* arithmos eggrafwn pou periexei */
		n = *((int *) block);
		/* gia ola ta records */
		for(j=0;j<n;j++){
			memcpy(&tmp,block + 2*sizeof(int) + j*sizeof(Record),sizeof(Record));
			if(tmp.id==record.id && !strcmp(tmp.name,record.name) && !strcmp(tmp.surname,record.surname) && !strcmp(tmp.city,record.city)){
				printf("The record already exists..\n");
				return -1;
			}
		}
	}
	
	/* binary search gia euresi thesis pros eisagwgi */
	low=1;
	middle=1;
	high=bno-1;
	i=0;
	/* an to file periexei panw apo 1 block */
	if(bno != 1){
		while(low <= high){
			/* vriskoume ti mesi */
			middle=low + (high-low)/2;
			if(BF_ReadBlock(fileDesc,middle,&block))
				return -1;
			n = *((int *) block);
			for(i=0;i<n;i++){
				memcpy(&tmp,block + 2*sizeof(int) + i*sizeof(Record),sizeof(Record));
				/* an vroume record megalutero apo auto pou theloume na eisagoume */
				if(tmp.id > record.id)
					break;
			}
			/* exoume elegsei ola ta blocks pou eprepe */
			if(low == high){
				/* an ola ta ids itan mikrotera k den eimaste sto teleutaio block */
				if(i == n && middle != bno-1){
					low = ++high;
					continue;
				}
				break;
			}
			/* to prwto megalutero pou vrikame einai mesa sto record */
			else if(i != 0 && i != n)
				break;
			/* to megalutero record einai stin arxi tou block */
			else if(i == 0)
				/* psakse sto deutero miso tou file */
				high = middle-1;
			/* ola ta records itan mikrotera */
			else if(i == n)
				/* psakse sto prwto miso tou file */
				low = middle+1;
			/* to megalutero record einai sto telos tou block */
			else
				break;
		}
	}
	/* an uparxei mono to block kefalida */
	if(bno-1 == 0){
		if(BF_AllocateBlock(fileDesc))
			return -1;
		//printf("\E[33mAllocate new block 1\E[0m\n");
		++bno;
		n = 0;
	}
	/* alliws diavase to teleutaio block */
	else {
		if(BF_ReadBlock(fileDesc,bno-1,&block))
			return -1;
		n = *((int *) block);
		/* xwraei parapanw eggrafi? */
		if(2*sizeof(int) + (n+1)*sizeof(Record) > BF_BLOCK_SIZE){
			//printf("\E[33mAllocate new block 2\E[0m\n");
			/* den xwraei -> ftiakse neo block */
			if(BF_AllocateBlock(fileDesc))
				return -1;
			/* auksise ton arithmo twn blocks */
			++bno;
		}
	}
	/* upologismos posa records xwraei ena block */
	full = (BF_BLOCK_SIZE-2*sizeof(int))/sizeof(Record);
	/* prepei na metakinithoun ola ta records mia thesi katw */
	for(j=bno-1;j > middle;j--){
		//printf("Moving records of block %d\n",j);
		if(BF_ReadBlock(fileDesc,j,&block))
			return -1;
		n = *((int *) block);
		block += 2*sizeof(int);
		/* metakinise ola ta records tou sugkekrimenou block mia thesi katw */
		memmove(block + sizeof(Record),block,n * sizeof(Record));
		/* diavase to proigoumeno block */
		if(BF_ReadBlock(fileDesc,j-1,&pblock))
			return -1;
		/* oi eggrafes pou uparxoun sto proigoumeno block */
		m = *((int *) pblock);
		pblock += 2*sizeof(int);
		/* an to proigoumeno block den einai gemato i einai adeio ---> pigaine stin arxi tis for (sumvainei stin eisagwgi meta tin diagrafi,pou menoun adeia blocksi)*/
		if(m != full || !m) continue;
		memcpy(block,pblock+(m-1)*sizeof(Record),sizeof(Record));
		block -= 2*sizeof(int);
		*((int *) block) += 1;
		if(BF_WriteBlock(fileDesc,j))
			return -1;
		pblock -= 2*sizeof(int);
		*((int *) pblock) -= 1;
		if(BF_WriteBlock(fileDesc,j-1))
			return -1;
	}
	/* metakinisi records mesa sto middle */
	//printf("Getting block %d/%d\n",middle,BF_GetBlockCounter(fileDesc)-1);
	if(BF_ReadBlock(fileDesc,middle,&block))
		return -1;
	n=*((int *)block);
	//printf("It has %d records!\n",n);
	block += 2*sizeof(int);
	//printf("Moving up %d records (n:%d - i:%d\n",n-i+1,n,i);
	/* metakinisi eggrafwn mesa sto middle */
	memmove(block +(i+1)*sizeof(Record),block +i*sizeof(Record),(n-i+1)*sizeof(Record));
	//printf("Inserting on position %d\n",i);
	memcpy(block +i*sizeof(Record),&record,sizeof(Record));
	block -= 2*sizeof(int);
	*((int *) block) += 1;
	//printf("Writing back block %d\n",middle);
	if(BF_WriteBlock(fileDesc,middle))
		return -1;
	printf("\n");
	return 0;
}



int Sorted_DeleteEntry(int fileDesc,char* fieldName,void* value){
	int bno,i,j,rno,pno,full;
	void *block,*pblock;
	Record tmp;
	int low,high,middle,search_prev=0,search_next=0,res,cont;
	
	if(value == NULL)
		return -1;
	if(strcmp(fieldName,"id") && strcmp(fieldName,"name") && strcmp(fieldName,"surname") && strcmp(fieldName,"city"))
		return -1;
	if(!strcmp(fieldName,"id")){
		bno = BF_GetBlockCounter(fileDesc);
		low=1;
		high=bno-1;
		// locate correct block
		// then do sequential deletion on previous && next block if any
		while(low <= high){
			//vriskoume ti mesi
			middle = low+(high-low)/2;
			if(BF_ReadBlock(fileDesc,middle,&block))
				return -1;
			//No of records
			rno = *((int *) block);
			block += 2*sizeof(int);
			for(i=0;i<rno;i++){
				//vriskoume ti diafora tis apothikeumenis record.id me tin value
				res = ((Record *)block+i)->id - *((int *)value);
				//an einai isa
				if(!res){
					//printf("%-d %-15s %-20s %-10s\n",((Record *)block+i)->id,((Record *)block+i)->name,((Record *)block+i)->surname,((Record *)block+i)->city);
					//an to i einai 0 tha psaksoume kai to proigoumeno block
					if(!i) search_prev = middle-1;
					//an to to i einai rno-1, to telos tou block, ara psaxnw to epomeno block
					if(!(rno-i-1)) search_next = middle+1;
					//olisthaineis ta epomena records meta apo to i
					memmove(block+i*sizeof(Record),block+(i+1)*sizeof(Record),(rno-i-1)*sizeof(Record));
					//meiwneis to No of records
					--rno;
					//pas pisw gia na elenkseis ta upoloipa records
					--i;
				}
			}
			//update to No of records
			*((int *) block -2) = rno;
			if(BF_WriteBlock(fileDesc,middle))
				return -1;
			//to katw kai to panw orio einai idia --> telos	
			if(high == low || search_prev || search_next)
				break;
			//psakse sto deutero miso tou file
			else if(res > 0)
				high = middle-1;
			//psakse sto prwto miso tou file
			else if(res < 0)
				low = middle+1;
		}
		// sequential delete from now on...
			
		//gia ola ta proigoumena blocks
		while(search_prev > 0){
			cont=0;
			if(BF_ReadBlock(fileDesc,search_prev,&block))
				return -1;
			rno = *((int *) block);
			block += 2*sizeof(int);
			//gia ola ta records
			for(i=0;i<rno;i++){
				res = ((Record *)block+i)->id - *((int *)value);
				if(!res){
					if(!i) cont=1;
					memmove(block+i*sizeof(Record),block+(i+1)*sizeof(Record),(rno-i-1)*sizeof(Record));
					--rno;
					--i;
				}
			}
			*((int *) block -2) = rno;
			if(BF_WriteBlock(fileDesc,search_prev))
				return -1;
			search_prev = cont ? search_prev-1 : 0;
		}
		//gia ola ta epomena blocks
		while(search_next < bno){
			cont=0;
			if(BF_ReadBlock(fileDesc,search_next,&block))
				return -1;
			rno = *((int *) block);
			block += 2*sizeof(int);
			for(i=0;i<rno;i++){
				res = ((Record *)block+i)->id - *((int *)value);
				if(!res){
					if(!(rno-i-1)) cont=1;
					memmove(block+i*sizeof(Record),block+(i+1)*sizeof(Record),(rno-i-1)*sizeof(Record));
					--rno;
					--i;
				}
			}
			*((int *) block -2) = rno;
			if(BF_WriteBlock(fileDesc,search_next))
				return -1;
			search_next = cont ? search_next+1 : bno;
		}
	}
	else if(!strcmp(fieldName,"name")){
		bno = BF_GetBlockCounter(fileDesc);
		//gia ola ta blocks
		for(i=1;i<bno;i++){
			if(BF_ReadBlock(fileDesc,i,&block))
				return -1;
			rno = *((int *) block);
			block += 2*sizeof(int);
			//gia ola ta records
			for(j=0;j<rno;++j){
				memcpy(&tmp,block + j*sizeof(Record),sizeof(Record));
				if(!strcmp(tmp.name,(char *)value)){
					memmove(block + j*sizeof(Record),block + (j+1)*sizeof(Record),(rno-j-1)*sizeof(Record));
					--rno;
					//check again-new element
					--j;	
				}
			}
			block -= 2*sizeof(int);
			//update record amount
			*((int *) block) = rno;
			if(BF_WriteBlock(fileDesc,i))
				return -1;
		}
	}
	else if(!strcmp(fieldName,"city")){
		bno = BF_GetBlockCounter(fileDesc);
		for(i=1;i<bno;i++){
			if(BF_ReadBlock(fileDesc,i,&block))
				return -1;
			rno = *((int *) block);
			block += 2*sizeof(int);
			for(j=0;j<rno;++j){
				memcpy(&tmp,block + j*sizeof(Record),sizeof(Record));
				if(!strcmp(tmp.city,(char *)value)){
					memmove(block + j*sizeof(Record),block + (j+1)*sizeof(Record),(rno-j-1)*sizeof(Record));
					--rno;
					//check again-new element
					--j;	
				}
			}
			block -= 2*sizeof(int);
			//update record amount
			*((int *) block) = rno;	
			if(BF_WriteBlock(fileDesc,i))
				return -1;
		}
	}
	else if(!strcmp(fieldName,"surname")){
		bno = BF_GetBlockCounter(fileDesc);
		for(i=1;i<bno;i++){
			if(BF_ReadBlock(fileDesc,i,&block))
				return -1;
			rno = *((int *) block);
			block += 2*sizeof(int);
			for(j=0;j<rno;++j){
				memcpy(&tmp,block + j*sizeof(Record),sizeof(Record));
				if(!strcmp(tmp.surname,(char *)value)){
					memmove(block + j*sizeof(Record),block + (j+1)*sizeof(Record),(rno-j-1)*sizeof(Record));
					--rno;
					//check again-new element
					--j;	
				}
			}
			block -= 2*sizeof(int);
			//update record amount
			*((int *) block) = rno;	
			if(BF_WriteBlock(fileDesc,i))
				return -1;
		}
	}

	// Roll up all records! general usage on both id,name,...
	full = (BF_BLOCK_SIZE-2*sizeof(int))/sizeof(Record);
	for(i=1;i<bno;i++){
		//printf("\nGetting block %d\n",i);
		if(BF_ReadBlock(fileDesc,i,&block))
			return -1;
		rno = *((int *) block);
		//printf("it has %d/%d records -->",rno,full);
		// block full, continue to next block 
		if(rno == full){
			//printf("full\n");
			continue;
		}
		//last block, nothing to do
		if(i+1 == bno){
			//printf("EOF\n");
			break;
		}
		block += 2*sizeof(int);
		//printf("need's filling! (missing %d records!)\n",full-rno);
		//get records from next block
		for(j=i+1;j<bno;j++){
			if(BF_ReadBlock(fileDesc,j,&pblock))
				return -1;
			pno = *((int *) pblock);
			pblock += 2*sizeof(int);
			//less-equal than needed -- needed = full-rno
			if(full-rno <= pno){
				//printf("fill[%d]:\tGetting %d records from block %d\n",i,(full-rno),j);
				//move records to 1st
				memmove(block+rno*sizeof(Record),pblock,(full-rno)*sizeof(Record));
				//move 2nd's records to beginning
				memmove(pblock,pblock + (full-rno)*sizeof(Record),(pno-(full-rno))*sizeof(Record));
				//update amount
				*((int *)pblock -2) -= (full-rno);
				*((int *)block  -2) = full;
				if(BF_WriteBlock(fileDesc,i))
					return -1;
				if(BF_WriteBlock(fileDesc,j))
					return -1;
				break;
			}
			//else == less than needed
			if(full-rno > pno){
				//printf("fill[%d]:\tblock %d has less records than needed...\n",i,j);
				memmove(block+rno*sizeof(Record),pblock,pno*sizeof(Record));
				*((int *)pblock -2) = 0;
				*((int *)block  -2) += pno;
				rno += pno;
				if(BF_WriteBlock(fileDesc,j))
					return -1;
				continue;
			}
		}
	}
	return 0;
}




void Sorted_GetAllEntries(int fileDesc,char* fieldName,void* value){
	int bno,i,rno,j,block_count;
	void *block;
	int low,high,middle,res;
	int next_rollback,search_previous,previous_rollback,search_next;

	bno = BF_GetBlockCounter(fileDesc);
	block_count = bno-1;
	/* me endeiksi NULL ektupwnei olo to file */
	if(value == NULL || fieldName == NULL){
		for(i=1;i<bno;i++){
			if(BF_ReadBlock(fileDesc,i,&block))
				return;
			rno = *((int *)block);
			printf("\E[31mBlock %d\E[0m (\E[34m%d records\E[0m)\n",i,rno);
			block += 2*sizeof(int);
			for(j=0;j<rno;j++){
				printf("%2d : ",j);
				printf("%-d %-15s %-20s %-10s\n",((Record *)block+j)->id,((Record *)block+j)->name,((Record *)block+j)->surname,((Record *)block+j)->city);
			}
		}
	}
	/* to fildName einai id */
	else if(!strcmp(fieldName,"id")){
		block_count=0;
		low=1;
		high=bno-1;
		next_rollback=0;
		search_previous=0;
		previous_rollback=0;
		search_next=0;

		int set_new_middle = 1,ignore_prev=0,ignore_next=0;	// right middle block found , stop doing bsearch! , ignore search_previous/next flags, 
		while(low <= high){
			// the found one is the first record --> search prev block
			if(search_previous)
				--middle;
			// the found one is the last record --> search next block
			else if(search_next)
				++middle;
			// continue searching
			else if(set_new_middle)
				middle=low +(high-low)/2;
			else
				//done -> stopped bsearching, no next/prev rollback
				break;	
			printf("\E[31mBlock %d\E[0m (\E[34m%d records\E[0m)\n",middle,rno);
			printf("Searching on block %d!\n",middle);
			if(BF_ReadBlock(fileDesc,middle,&block))
				return;
			++block_count;
			rno = *((int *)block);
			block += 2*sizeof(int);
			for(i=0;i<rno;i++){
				res = ((Record *)block+i)->id - *((int *) value);
				//printf("Comparing %d with %d -- result: %d\n", ((Record *)block+i)->id,*((int *) value),res);			
				if(res == 0){
					//printf("%-d %-15s %-20s %-10s\n",((Record *)block+i)->id,((Record *)block+i)->name,((Record *)block+i)->surname,((Record *)block+i)->city);
					// 1st record match && not first block
					if(!i && middle != 1){
						//printf("Will search previous!\n");
						//executed only once
						if(!ignore_prev)		// flag not set
							++search_previous;	// continue searching backwards
						set_new_middle=0;		// stop bsearching
					}
					// last record match && not last block
					else if(!(rno-i-1) && middle+1 != bno){
						//printf("Will search next!\n");
						if(!ignore_next)		// flag not set
							++search_next;		// continue searching frontwards
						set_new_middle=0;		// stop bsearching
					}
				}
				// current block == low end && checked all records or
				// first record , search previous flag set && no match
				if((middle == low && !(rno-i-1)) || (!i && search_previous && res != 0)){	//stop rollback
					middle += search_previous;	//rollback to middle point
					//printf("Will jump front to %d\n",middle);
					search_previous = 0;		//stop searching backwards
					ignore_prev=1;				//so set ignore flag
				}
				else if((middle == high && !(rno-i-1)) || (!(rno-i-1) && search_next &&  res != 0)){	//stop rollfront 
					middle -= search_next;		
					//printf("Will jump back to %d\n",middle);
					search_next = 0;		//stop searching forward
					ignore_next =1;			//so set ignore flag
				}
				
			}
			if(res > 0)
				high = middle-1;
			else if(res < 0)
				low = middle+1;
		}
	}
	
	//IDIA ME TIN HP
	
	/* to fildName einai name */
	else if(!strcmp(fieldName,"name")){
		for(i=1;i<bno;i++){
			if(BF_ReadBlock(fileDesc,i,&block))
				return;
			rno = *((int *)block);
			printf("\E[31mBlock %d\E[0m (\E[34m%d records\E[0m)\n",i,rno);
			block += 2*sizeof(int);
			for(j=0;j<rno;j++){
				if(!strcmp(((Record *)block+j)->name,(char *)value)){
					printf("%2d : ",j);
					printf("%-d %-15s %-20s %-10s\n",((Record *)block+j)->id,((Record *)block+j)->name,((Record *)block+j)->surname,((Record *)block+j)->city);
				}
			}
		}
	}
	/* to fildName einai surname */
	else if(!strcmp(fieldName,"surname")){
		for(i=1;i<bno;i++){
			if(BF_ReadBlock(fileDesc,i,&block))
				return;
			rno = *((int *)block);
			printf("\E[31mBlock %d\E[0m (\E[34m%d records\E[0m)\n",i,rno);
			block += 2*sizeof(int);
			for(j=0;j<rno;j++){
				if(!strcmp(((Record *)block+j)->surname,(char*)value)){
					printf("%2d : ",j);
					printf("%-d %-15s %-20s %-10s\n",((Record *)block+j)->id,((Record *)block+j)->name,((Record *)block+j)->surname,((Record *)block+j)->city);
				}
			}
		}
	}
	/* to fildName einai city */
	else if(!strcmp(fieldName,"city")){
		for(i=1;i<bno;i++){
			if(BF_ReadBlock(fileDesc,i,&block))
				return;
			rno = *((int *)block);
			printf("\E[31mBlock %d\E[0m (\E[34m%d records\E[0m)\n",i,rno);
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
	printf("Διαβάστηκαν %d blocks\n",block_count);
}

