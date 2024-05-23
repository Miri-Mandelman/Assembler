/*This file makes the output files*/
#include <stdio.h>
#include <string.h>
#include "data.h"

#define UNIT 31
#define TEN (31<<5)

typedef struct node * lablePoint;
typedef struct node {
	char name[MAX_LABLE];
	int add;
	char sign;
	int ent;
	int ext;
	lablePoint next;
	}labelNode;

char getCoded(int );

void outFiles(short ins[], short data[], int ic, int dc, lablePoint head, char *fileName,int ext,int ent){

	char *object =".ob";
	char *entries = ".ent";
	char *externals= ".ext";
	char temp[MAX_FILE_NAME];
	lablePoint pl;
	char units, tens;
	int i;
	FILE *fd;
	
	if(ext){
		memset(temp,'\0',MAX_FILE_NAME-1);/*reset fileName*/
		strcpy(temp, fileName);
		strcat(temp, externals);

		if(!(fd=fopen(temp, "w+")))
			printf("\n Error, could not open file %s \n", temp);
		else{
			pl=head; 	
			while(pl){
				if(pl->sign == 'x'){
					fprintf(fd,"%s	",pl->name);
					units = getCoded((pl->ext)%32);
					tens = getCoded(((pl->ext)/32)%32);
					fprintf(fd,"%c%c\n",tens,units);
				}/*close if*/
			}/*close while*/
		}/*close else*/
	}/*close if - ext*/
	
	if(ent){
		memset(temp,'\0',MAX_FILE_NAME-1);/*reset fileName*/
		strcpy(temp, fileName);
		strcat(temp, entries);
		
		if(!(fd=fopen(temp, "w+")))
			printf("\n Error, could not open file %s \n", temp);
		else{
			pl=head; 	
			while(pl){
				if(pl->ent == 0){
					fprintf(fd,"%s	",pl->name);
					units = getCoded((pl->add)%32);
					tens = getCoded(((pl->add)/32)%32);
					fprintf(fd,"%c%c\n",tens,units);
					
				}/*close if*/
			}/*close while*/
		}/*close else*/
	}/*close if -ent*/
	
	memset(temp,'\0',MAX_FILE_NAME-1);/*reset fileName*/
	strcpy(temp, fileName);
	strcat(temp, object);
	
	if(!(fd=fopen(temp, "w+")))
		printf("\n Error, could not open file %s \n", temp);
	else{
	
		units = getCoded((ic)%32);
		tens = getCoded(((ic)/32)%32);
		fprintf(fd,"%c%c	",tens,units);
	
		units = getCoded((dc)%32);
		tens = getCoded(((dc)/32)%32);
		fprintf(fd,"%c%c\n",tens,units);
	
		for(i=0; i<= ic; i++){
			units = getCoded((i+100)%32);
			tens = getCoded(((i+100)/32)%32);
			fprintf(fd,"%c%c	",tens,units);
			
			units = getCoded(ins[i] & UNIT);
			tens = getCoded((ins[i] & TEN)>>5);
			fprintf(fd,"%c%c\n",tens,units);
	
		}/*close for loop -ic*/
		
		for(i=0; i<= dc; i++){
			units = getCoded((i+ic+100)%32);
			tens = getCoded(((i+ic+100)/32)%32);
			fprintf(fd,"%c%c	",tens,units);
			
			units = getCoded(data[i] & UNIT);
			tens = getCoded((data[i] & TEN)>>5);
			fprintf(fd,"%c%c\n",tens,units);
	
		}/*close for loop -ic*/
	}/*close else*/
	return;
}/*end outFiles*/

/*function that returns the coded character of a givvan number that is smaller than 32*/
char getCoded(int i){
	
	char code[32] = {'!','@','#','$','%','^','&','*','<','>','a','b','c','d','e','f'
					,'g','h','i','g','k','l','m','n','o','p','q','r','s','t','u','v'};
	
	return(code[i]);

}/*end getCoded*/
