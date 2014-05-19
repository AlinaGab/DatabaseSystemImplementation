#include <stdio.h>
#include <string.h>

#include "BF.h"
#include "HP.h"
#include "Record.h"
#include "Join_MergeSorted.h"

typedef enum {FALSE,TRUE} bool;
	
	

int Join_MergeSort(char *inputName1,char *inputName2,char *outputName){	
	struct Fileinfo {
		bool EOBF;							/* end of block file */
		int fileDesc;
		int Record_It,Block_It;
		int rno,bno;						/* record No,block No */
		Record *recp;						/* record pointer */
		void *Block;
	};
	struct Fileinfo Source = {FALSE,0,0,1,0,0,NULL,NULL};
	struct Fileinfo Target = {FALSE,0,0,1,0,0,NULL,NULL};
	struct Fileinfo Out    = {FALSE,0,0,1,0,0,NULL,NULL};
	
	
	/* anoigoume to prwto arxeio */
	if((Source.fileDesc = HP_OpenFile(inputName1)) < 0){                                         
		BF_PrintError("The file1 could not be opened..\n");
		return -1;
    }
	/* anoigoume to deutero arxeio */
	if((Target.fileDesc = HP_OpenFile(inputName2)) < 0){
		printf("The file2 \"%s\" could not be opened..\n",inputName2);
		if(HP_CloseFile(Source.fileDesc) < 0)
			printf("The file1 \"%s\" could not be closed..\n",inputName1);
		return -1;
	}

	/* dimiourgoume ena output arxeio */
	if(HP_CreateFile(outputName) < 0){
		printf("The output file \"%s\" could not be created..\n",outputName); 
		return -1;
	}
	/* anoigoume to output arxeio */
	if((Out.fileDesc = HP_OpenFile(outputName)) < 0){
		printf("The output file \"%s\" could not be opened..\n",outputName);
		if(HP_CloseFile(Source.fileDesc) < 0)
			printf("The file1 \"%s\" could not be closed..\n",inputName1);
		if(HP_CloseFile(Target.fileDesc) < 0)
			printf("The file2 \"%s\" could not be closed..\n",inputName2);
		return -1;
	}
	/* blocks pou periexei to file1 */
	if((Source.bno = BF_GetBlockCounter(Source.fileDesc)) < 0){
		BF_PrintError("Could not find the No of blocks in file1.."); 
		return -1;
	}
	/* blocks pou periexei to file2 */
	if((Target.bno = BF_GetBlockCounter(Target.fileDesc)) < 0){
		BF_PrintError("Could not find the No of blocks in file2.."); 
		return -1;
	}
	/* upologismos records pou xwraei 1 block tou output file */
	Out.rno = BF_BLOCK_SIZE/sizeof(Record);
	/* an den ftasame sto telos tou file1 i an den fataseme sto telos tou file2 */
	while(Source.EOBF == FALSE || Target.EOBF == FALSE){
		/* an ftasame sto teleutaio record tou block tou file1 k den eimaste sto telos tou file1 */
		if(Source.Record_It == Source.rno && Source.EOBF == FALSE){
			/* an eimaste sto teleutaio block you file1 */
			if(Source.Block_It == Source.bno){
				Source.EOBF = TRUE;
				if(HP_CloseFile(Source.fileDesc) < 0)
					return -1;
			}
			else {
				if(BF_ReadBlock(Source.fileDesc,Source.Block_It,&Source.Block) < 0){
					BF_PrintError("Could not read the block..");
					return -1;
				}
				/* records pou periexei to trexon block */
				Source.rno = *((int *) Source.Block);
				Source.Record_It = 0;
				Source.Block += 2*sizeof(int);
				Source.Block_It++;
			}
		}
		/* an ftasame sto teleutaio record tou block tou file2 k den eimaste sto telos tou file2 */
		if(Target.Record_It == Target.rno && Target.EOBF == FALSE){
            /* an exoume vgei apo ta oria tou arxeiou */
			if(Target.Block_It == Target.bno){
				Target.EOBF = TRUE;
				if(HP_CloseFile(Target.fileDesc) < 0)
					return -1;
			}
            /* an eimaste sto arxeio (mexri to teleutaio block) */
			else {
				if(BF_ReadBlock(Target.fileDesc,Target.Block_It,&Target.Block) < 0){
					BF_PrintError("Could not read the block..");
					return -1;
				}
                /* poses eggrafes exei to target */
				Target.rno = *((int *) Target.Block);
				Target.Record_It = 0;
				Target.Block += 2*sizeof(int);
				Target.Block_It++;
			}
		}
		/* an eimaste sto teleutaio record tou block i teleiwse to file1 k to file2 i eimaste sto prwto block (me records) tou output */
		if(Out.Record_It == Out.rno || (Source.EOBF == TRUE && Target.EOBF == TRUE) || (Out.Block_It == 1)){ 
			/* to den eimaste sto prwto block tou output */
			if(Out.Block_It > 1){				
				Out.Block -= 2*sizeof(int);
                /* pername to poses eggrafes tha exei to out file */
				*((int *) Out.Block) = Out.Record_It;
				if(BF_WriteBlock(Out.fileDesc,Out.Block_It-1) < 0){
					BF_PrintError("Could not write to this block ..");
					return -1;
				}
				if(HP_CloseFile(Out.fileDesc) < 0)
					return -1;
			}
			/* an ftasame sto telos tou file1 k tou file2 */
			if(Source.EOBF == TRUE && Target.EOBF == TRUE)
				break;
			if((Out.fileDesc = HP_OpenFile(outputName)) < 0){
				printf("The output file \"%s\" could not be opened..\n",outputName);
				return -1;
			}
			if(BF_AllocateBlock(Out.fileDesc) < 0){
				BF_PrintError("Could not allocate new block..");
				return -1;
			}
			if(BF_ReadBlock(Out.fileDesc,Out.Block_It,&Out.Block) < 0){
				BF_PrintError("Could not read the block..");
				return -1;
			}
			Out.Record_It = 0;
			Out.Block += 2*sizeof(int);
			Out.Block_It++;
		}
		/* kratame deikti sto input buffer1 */
		Source.recp = ((Record *) Source.Block + Source.Record_It);
		/* kratame deikti sto input buffer2 */
		Target.recp = ((Record *) Target.Block + Target.Record_It);
		/* kratame deikti sto output buffer */
		Out.recp = ((Record *) Out.Block + Out.Record_It);
		/* an den exei records to file1 i to file2 */
		if(!Source.rno || !Target.rno)
			continue;
		/* alliws an den teleiwse to file1 k teleiwse to file2 */
		else if(Source.EOBF == FALSE && Target.EOBF == TRUE){
			memcpy(Out.recp,Source.recp,sizeof(Record)); 
			Source.Record_It++;
		}
		/* alliws an teleiwse to file1 k den teleiwse to file2 */
		else if(Source.EOBF == TRUE && Target.EOBF == FALSE){
			memcpy(Out.recp,Target.recp,sizeof(Record)); 
			Target.Record_It++;
		}
		/* an to id pou deixnei o deiktis tou buffer1 mikrotero iso tou id pou deixnei o deiktis tou buffer2 */
		else if((Source.recp)->id < (Target.recp)->id){
			memcpy(Out.recp,Source.recp,sizeof(Record)); 
			Source.Record_It++;
		}
		else if((Source.recp)->id > (Target.recp)->id){
			memcpy(Out.recp,Target.recp,sizeof(Record)); 
			Target.Record_It++;
		}
		/* elegxos gia diplotupa i idia id's */
		/* an ta id tou file1 kai tou file2 einai idia */
		else if(((Source.recp)->id == (Target.recp)->id)){
			memcpy(Out.recp,Source.recp,sizeof(Record));
			Source.Record_It++;
            /* vrethike diplotupo */
			if(!(strcmp((Source.recp)->name,(Target.recp)->name)) && !(strcmp((Source.recp)->surname,(Target.recp)->surname)) && !(strcmp((Source.recp)->city,(Target.recp)->city)))
				Target.Record_It++;
		}
		/* prin guriseis stin arxiki while auksise ton iterator tou output file */
		Out.Record_It++;
	}
	return 0;
}
	   


