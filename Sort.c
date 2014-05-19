#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include "Record.h"
#include "BF.h"
#include "HP.h"
#include "Sort.h"

/* Voithitiki sunartisi tis quicksort */
void swap(Record *x,Record *y){
    Record temp;
    memcpy(&temp,x,sizeof(Record));
    memcpy(x,y,sizeof(Record));
    memcpy(y,&temp,sizeof(Record));
}

void Quicksort(int m,int n,void *bp){
    int i,j,mid,key;
    void *y;
    Record *x;
    if(m < n){
        mid=(n-m)/2+m;
        y=bp+2*sizeof(int);       //skip 2 ints
        x=(Record *)y+mid;
        swap((Record *)y+m,x);
        key = ((Record *)y+m)->id;
        i=m+1;
        j=n;
        while(i<=j){
            while((i <= n) && ((Record *)y+i)->id <= key) ++i;
            while((j >= m) && ((Record *)y+j)->id > key) --j;
            if(i < j)
                swap((Record *)y+i,(Record *)y+j);
        }
        swap((Record *)y+m,(Record *)y+j);
        Quicksort(m,j-1,bp);
        Quicksort(j+1,n,bp);
    }
}


typedef struct File_t {
	char buf[5+sizeof(int)];
	int fd;					/* file decriptor */
	int record,nrecords;	/* iterator,amount (unused in source) */
	int block,nblocks;		/* iterator,amount (unused in target) */
	void *bp;				/* block pointer */
} File_t;


typedef struct block_t {
	int record,nrecords;	/* iterator,amount */
	void *bp;				/* block pointer */
} block_t;

/* extra sunartisi */
int insert_at_end(int fd,Record record){
	void *block;
	int bno,rno,i,j;

	bno = BF_GetBlockCounter(fd);
	if(bno == 1){
		BF_AllocateBlock(fd);
		bno += 1;
	}
	BF_ReadBlock(fd,bno-1,&block);
	rno = *((int *) block);
	if(2 * sizeof(int) + (rno + 1) * sizeof(Record) > BF_BLOCK_SIZE){
		BF_AllocateBlock(fd);
		bno += 1;
	}
	BF_ReadBlock(fd,bno-1,&block);
	rno = *((int *) block);
	memcpy(block + 2*sizeof(int) + rno * sizeof(Record),&record,sizeof(Record));
	*((int *) block) += 1;
	BF_WriteBlock(fd,bno-1);

}


/* voithitiki sunartisi gia tin epilogi mikroteris eggrafis kai eisagwgi tis sto kainourio arxeio */
int getMin(File_t *mmap,int low,int high,char *file){
	int min,m,fd;
	if((fd = HP_OpenFile(file)) < 0)
        return -1;
	for(m=0;m<high-low;++m){
		if((mmap+m)->fd == -1)
			continue;
		if((mmap+m)->block == (mmap+m)->nblocks && (mmap+m)->record == (mmap+m)->nrecords){
			if(HP_CloseFile((mmap+m)->fd) < 0)
                return -1;
			(mmap+m)->fd = -1;
			continue;
		}
		if((mmap+m)->bp == NULL || (mmap+m)->record == (mmap+m)->nrecords){
			if(BF_ReadBlock((mmap+m)->fd,(mmap+m)->block,&(mmap+m)->bp) < 0){
                BF_PrintError("Could not read the block..\n");
                return -1;
            }
			(mmap+m)->block += 1;
			(mmap+m)->record=0;
			(mmap+m)->nrecords=*((int *)(mmap+m)->bp);
			(mmap+m)->bp += 2*sizeof(int);
		}
	}
	for(m=0,min=-1;m<high-low;++m)
		if((mmap+m)->fd != -1 && (min == -1 || ((Record *)(mmap+m)->bp+(mmap+m)->record)->id < ((Record *)(mmap+min)->bp+(mmap+min)->record)->id))
			min = m;
	if(min == -1)
		return -1;
//	insert_at_end(fd,*((Record *)(mmap+min)->bp+(mmap+min)->record));
	if(HP_InsertEntry(fd,*((Record *)(mmap+min)->bp+(mmap+min)->record)) < 0)
        return -1;
	++(mmap+min)->record;	
	if(HP_CloseFile(fd) < 0)
        return -1;
	return 0;
}



int Sort(char *fileName,int bufferSize){
	File_t source,target,*mmap;
	block_t *map;
	char buf[5+sizeof(int)],tbuf[5+sizeof(int)];
	int run,nruns,low,high,pass=0,depth,quantity;
	int i,j,k,sum,max_records,min;
	void *outbuf;
	
    /* min kaneis tipota an exeis mono to arxeio eisodou kai arxeio eksodou */
	if(bufferSize < 3)											
		return -1;
    /* ypologismos twn records poy xwraei to arxeio */
	max_records = BF_BLOCK_SIZE/sizeof(Record);
    /* arxikopoisi twn metavlitwn twn structs */
	memset(&source,0,sizeof(struct File_t));
	memset(&target,0,sizeof(struct File_t));
    /* prosperna to header */
	source.block=1;												
	target.block=1;
	if((source.fd = HP_OpenFile(fileName)) < 0)
        return -1;
	if((source.nblocks = BF_GetBlockCounter(source.fd)) < 0){
		BF_PrintError("The No of blocks could not be returned..\n");
        return -1;
    }
    
	/* SORTING PHASE */
	
    /* Ypologismos twn runs */
	nruns = (source.nblocks-1)/(bufferSize-1);
	/* Dimiourgia enos parapanw run se periptwsi perritou apotelesmatos */
    if((source.nblocks-1)%(bufferSize-1) != 0)
		++nruns;
	/* Desmeusi pinaka domwn block_t */
	map = malloc((bufferSize-1)*sizeof(struct block_t));
	if(!map) 
        return -1;
	
	printf("\e[41;30mPass %d\e[0m\n",pass++);
    /* gia kathe run */
	for(run=0;run<nruns;++run){
        /* Aponomi onomatwn gia tn dimiourgia proswrinwn arxeiwn */
		sprintf(buf,"run.%d",run);
		if(HP_CreateFile(buf) < 0)
            return -1;
		if((target.fd = HP_OpenFile(buf)) < 0)
            return -1;
		printf("Opening %d\n",target.fd);
        /* Prospername to prwto block pliroforias */
		target.block = 1;
        /* Ypologimos tou panw kai katw oriou */
		low = (run+0)*(bufferSize-1)+1;
		high= (run+1)*(bufferSize-1)+1;
        /* An to high ginei megalutero tou plithous twn block tou arxeiou */
		if(high > source.nblocks)
            /* Dwstou to pragmatiko plithos twn blocks tou arxeiou */
			high = source.nblocks;
		
		printf("\e[34mrun %d\e[0m\n",run);
        /* Gia kathe run diavase to kathe block tou kai metra ta records */
		for(i=0,sum=0;i<high-low;++i){
			if(BF_ReadBlock(source.fd,low+i,&(map+i)->bp) < 0){
                BF_PrintError("Could not read the block..\n");
                return -1;
            }
            /* Arxikopoiisi tou record iterator tou map */
			(map+i)->record = 0;
            /* plithous twn records tou kathe block tou map */
			(map+i)->nrecords = *((int *)(map+i)->bp);
			sum += (map+i)->nrecords;
		}
		/* Gia kathe run,taksinomise kathe block me quicksort */
		for(i=0;i<high-low;++i){
			printf("QuickSort %d\n",i);
			Quicksort(0,(map+i)->nrecords-1,(map+i)->bp);		/* quicksort */
			(map+i)->bp += 2*sizeof(int);						/* skip 2 int's */
		}

		printf("Sort done.\n");
		/* apo ti prwti eggrafi tou run mexri tin teleutaia */
		for(i=0;i<=sum;++i){
            /* An exei gemisei to output buffer h den exoume alles eggrafes na diavasoume */ 
			if(i % max_records == 0 || i == sum){				
                /* An den eimaste sto prwto block */
				if(i != 0 || i == sum){							
					*((int *) outbuf) = target.record;
					if(BF_WriteBlock(target.fd,target.block-1) < 0){
                        BF_PrintError("Could not write to this block..\n");
                        return -1;
                    }
				}
				/* an exoume perasei apo ola ta records tou run */
				if(i == sum)
					break;
                /* Mhdenise ton record iterator tou target */
				target.record=0;
                /* kai desmeuse kainourgio block */
                if(BF_AllocateBlock(target.fd) < 0){
                    BF_PrintError("The block could not be allocated..\n");
                    return -1;
                }
				if(BF_ReadBlock(target.fd,target.block,&outbuf) < 0){
                        BF_PrintError("Could not read the block..\n");
                        return -1;
                }
				/* pigaine sto epomeno block */
				++target.block;
			}
			/* to min einai -1 (pou den uparxei) */
			for(min=-1,j=0;j<high-low;++j)
                /* An exoume koitaksei oles tis eggrafes pou deixnei o bp */
				if((map+j)->record == (map+j)->nrecords)		
					continue;
                /* Sugkrine tin ekastote eggrafi me tin  min */
				else if(min == -1 || ((Record *)(map+j)->bp+(map+j)->record)->id < ((Record *)(map+min)->bp+(map+min)->record)->id)
                    /* An einai mikroteri thewrise autin min */
					min = j;
            /* kai valti sto output buffer */
			memcpy(outbuf+2*sizeof(int)+target.record*sizeof(Record),(map+min)->bp+(map+min)->record*sizeof(Record),sizeof(Record));
			++target.record;
			++(map+min)->record;
		}	
		HP_GetAllEntries(target.fd,"id",NULL);
		printf("Closing fd %d\n",target.fd);
		if(HP_CloseFile(target.fd) < 0)
            return -1;
	}
	if(HP_CloseFile(source.fd) < 0)
        return -1;
	free(map);
	
	printf("nr === %d\n",nruns);
	
    
	/* MERGE PHASE */
    /* An exoume ena run den xreiazetai i merge */
	if(nruns-1 == 0)		
		return 0;

	int m,n=0,step,fd;
    /* Ypologismos tou vathous tou dentrou */
	depth = (int)((log(nruns)/log(bufferSize-1))+1);
    printf("--> runs = %d\n",nruns);
	printf("--> depth = %d\n",depth);
    /* Gia olo to depth kai mexri na teleiwsoun ta runs */
	for(i=0;i<depth && nruns!=1;++i){
        /* Se posa runs tha sunenothoun ta arxeia */
		quantity = (nruns-1)/(bufferSize-1)+1;				
		printf("-> q = %d\n",quantity);
		/* Gia ola ta runs ana step */
		for(j=0;j<nruns;j+=step){
			/* lathos tis BF! xwris auto den douleue */
			BF_Init();
			/* kainouria prosorina arxeia */
			sprintf(target.buf,"tmp.%d",j/(bufferSize-1));
			printf("\e[41;30mPass %d File tmp.%d\e[0m\n",pass,j/(bufferSize-1));

            /* Deiktis sto katw akro */
			low = j;
            /* An kseperasoume to plithos twn runs proxwrame ana enapomeinonta blocks */
            if(j+(bufferSize-1) > nruns)
                step = nruns-j;
            else
                step = (bufferSize-1);
            high = j+step;
			mmap = calloc((high-low),sizeof(struct File_t));
			
            /* Gia to ekastote run */
			for(m=0;m<high-low;++m){
				sprintf(buf,"run.%d",low+m);
				(mmap+m)->fd = HP_OpenFile(buf);
				(mmap+m)->nblocks=BF_GetBlockCounter((mmap+m)->fd);
				(mmap+m)->block=1;
				(mmap+m)->bp=NULL;
			}
					
			/* Dimiourgia telikou ouput arxeiou */
			if(HP_CreateFile(target.buf) < 0)
                return -1;
            /* Eisagwgi mikroteris eggrafis sto output arxeio auto*/
			for(n=0;getMin(mmap,low,high,target.buf) != -1;++n);
			
            /* Gia kathe run pou exoume grapsei,diegrapse to proswrino arxeio */
			for(m=0;m<high-low;++m){
				sprintf(buf,"run.%d",low+m);
				remove(buf);
			}
            /* Kai metonomase to */
			sprintf(buf,"run.%d",j/(bufferSize-1));
			if(rename(target.buf,buf) == -1)
				perror("rename");
			system("ls");
			
		}
		free(mmap);
		nruns = quantity;
		pass++;		
	}
	rename("run.0",fileName);
	printf("in %d\n",n);
	return 0;
}


