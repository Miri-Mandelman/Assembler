#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "data.h"

/*Declaration on structer for the lables list:
	each node has 5 filds: 1.name -of the lable, 
			2.add - address to the value of the lable at the code Image 
			3.sign - d, for data type(.data/.struct/.string) .
				i, for instruction. x, for extern.
			4.ent - 1, if the lable defined as '.entery'. 0, if not(defolt).
			5.ext - the ic adress where used as operand.
			6.next -pointer to the next lable.*/
typedef struct node * lablePoint;
typedef struct node {
	char name[MAX_LABLE];
	int add;
	char sign;
	int ent;
	int ext;
	lablePoint next;
	}labelNode;

/*Fuctions declaration:*/
void spreadMacro (FILE *, char *);
int firstScan(FILE *, short (*)[], short (*)[], lablePoint *, int *, int *, int *);
void updateList(lablePoint *, int );
int secondScan(FILE *, short (*)[], lablePoint *, int *);
void freeListL(lablePoint *);
char getCoded(int );
void outFiles(short [], short [], int , int , lablePoint , char *,int ,int );

int main(int argc,char * argv[]){
	/*Variables:*/
	FILE *fd;
	int i;
	char fileName[MAX_FILE_NAME];
	int dc=0, ic=0;
	short ins[MAX_FILE_LENG];
	short data[MAX_FILE_LENG];
	lablePoint head = (lablePoint)malloc(sizeof(labelNode));
	int ext =0, ent =0; /*flugs for making the extern and entry files*/
	
	memset(head->name,'\0',MAX_LABLE);
	if(argc==1){
		printf("\nError, please enter file name in the command line \n");
		exit(0);
	}
	for(i=1;i<argc;i++){
		strcpy(fileName, argv[i]);
		strcat(fileName, ".as");
		if(!(fd = fopen(fileName, "r+"))){
			printf("\n Error, could not open file %s \n", fileName);
		}else{
			strcpy(fileName, argv[i]);
			spreadMacro(fd, fileName);/*call for function that spread the macros */
			memset(fileName,'\0',MAX_FILE_NAME-1);/*reset fileName*/
			strcpy(fileName, argv[i]);
			strcat(fileName, ".am");

			if(!(fd=fopen(fileName, "w+")))
				printf("\n Error, could not open file %s \n", fileName);
			else{
				if(! firstScan(fd, (&ins), (&data), &head, &ic, &dc, &ext)){
					updateList(&head, ic);/*call for function that update theaddress of data labels*/
					rewind(fd);
					if(! secondScan(fd, (&ins), &head, &ent)){
						outFiles(ins, data, ic, dc, head, argv[i], ext, ent);
					}/*close if*/
				}/*close if*/
			}/*close else*/
			freeListL(&head);
		}/*close else*/
	}/*close for loop*/
	return 0;

}/*close main*/
