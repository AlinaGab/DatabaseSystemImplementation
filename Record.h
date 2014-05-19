#ifndef RECORD_H_
#define RECORD_H_

#define BF_BLOCK_SIZE 512

typedef struct{
	int id;
	char name[15];
	char surname[20];
	char city[10];
}Record;

#endif /* RECORD_H_ */
