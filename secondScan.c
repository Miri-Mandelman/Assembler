/*This file is for the second-scan function*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "data.h"

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
char getWord(FILE *, char [MAX_SENT]);
int isOrder(char *);/*check if word is an instruction name.*/
int caseEntry(char, lablePoint *, FILE *, int);/*function for entry instruction.*/
int caseOrderSec(char, FILE *, short (*)[], lablePoint *, int *, int, int);/*function for an order sentence.*/
int operand(char [], int *, short (*)[], lablePoint *, int *, int);/*function for a operand of an order sentence.*/

/*The function operate step "second scan" on the file - finishing coding of labels.
Returns: 1-if there are errors. or 0 - if there aren't.
Variables:  fd= the file to read from.
			ins= pointer on the array of the instruction image.
			head= points to the first node of the lables-list.
			ent= flag to update to 1 if there is a valid entry instruction.*/
int secondScan(FILE *fd, short (*ins)[], lablePoint *head, int *ent)
{
	/*Variables:*/
	int ic=0;
	int lineC =0; /*line counter*/
	char word[MAX_SENT];
	char sen[MAX_SENT];
	int errors =0;/*incase of an error, changed to 1*/
	char c;
	int order;/*the opcode number of an order name*/
	int flag=0;		
	while(feof(fd)==0){
		c=getWord(fd,word);
		lineC++;
		/*Comment sentence, empty sentence, or data/string/struct/extern instruction:------*/
		if(word[0] ==';' || strcmp(word, "")==0 || strcmp(word, ".extern")==0
			|| strcmp(word, ".data")==0 || strcmp(word, ".string")==0 || strcmp(word, ".struct")==0)
		{
			if(c!='\n'){/*Skipping the sentence*/
				fgets(sen, MAX_SENT, fd);
				memset(sen,'\0',MAX_SENT-1);
			}
		}/*end if, skip sentence*/
		
		/*entry instruction:-------------------------*/
		else if(strcmp(word, ".entry")==0){
		
			if(!caseEntry(c, head, fd, lineC))
				errors=1;
			else
				(*ent)=1;
		}/*close if, entry*/
		
		/*order sentence:*--------------------------*/
		else if((order= isOrder(word))+1){ /*call for function that returns the opcode number of an order name, 
											or -1,if it's not an order*/		
			flag= caseOrderSec(c, fd, ins, head, &ic, lineC, order);/*call for a function for an order sentence.
							return -the number of memory words , or 0 for an error.*/
			if(flag)/*there were errors*/
				errors= 1;
		}/*close if,order*/
		
	}/*close while loop*/
		
	return errors;
}/*end secondScan*/

/*The function takes care of an entry instruction:
	finds the label on the label- list, and updating the 'ent' field to 1.
Returns: 0 - incase of any error, otherwise -1.
Variables:  c= the character that was readen from the text after '.entry'.
			head= points to the first node of the lables-list.
			fd= the file to read from.
			lineC= the number of the line in the file.*/
int caseEntry(char c, lablePoint *head, FILE *fd, int lineC){
	
	char word[MAX_SENT];
	lablePoint pl =NULL;
	
	if(c=='\n'){
		printf ("\nERROR, line %d- missing lable name after .entry\n", lineC);
		return 0;
	}/*end if*/
	c= getWord(fd,word);
	if (strcmp(word, "")==0){/*only white characters after '.entry'*/
		printf ("\nERROR, line %d- missing lable name after .entry\n", lineC);
		return 0;
	}/*end if*/
	while(c!='\n'){/*check  for extra in sentence:*/
		if(c!=' ' && c!='\t'){
			printf ("\nERROR, line %d- extra texst in sentence\n", lineC);
			fgets(word, MAX_SENT, fd);
			return 0;
		}/*end if*/
		fgetc(fd);
	}/*end while*/
	if(strcmp((*head)->name, "")==0){/*the list is empty*/
		printf ("\nERROR, line %d- undefined label after '.entry'\n", lineC);
		fgets(word, MAX_SENT, fd);
		return 0;
	}/*close if*/
	pl= (*head);
	while(pl && strcmp(pl->name, word)!=0){
		pl=pl->next;}
	if(!pl){/*there is no label in list with name = word*/
		printf ("\nERROR, line %d- undefined label after '.entry'\n", lineC);
		fgets(word, MAX_SENT, fd);
		return 0;
	}/*close if*/
	if(pl->sign =='x'){/*label of extern instruction*/
		printf ("\nERROR, line %d- invalid double declare of label as extern and entry\n", lineC);
		fgets(word, MAX_SENT, fd);
		return 0;
	}/*close if*/
	
	pl->ent=1;
	return 1;
	
}/*end caseEntry*/

/*The function takes care of an order sentence:
	updating ic and coding the label type- operands with their address.
Return: 1 if there is an error. otherwise -0.
Variables: c= the character that was readen from the text after the order name.
			fd= the file to read from.
			ins= pointer on the array of the instruction image.
			head - pointer to the first node on the list.
			ic= the address of the first empty node in the ins array.
			lineC= the number of the line in the file.
			order= the opcode number of the order name.*/
int caseOrderSec(char c, FILE *fd, short (*ins)[], lablePoint *head, int *ic, int lineC, int order){
	
	char sen[MAX_SENT];
	int i=0, first=0, second=0;
	fgets(sen, MAX_SENT, fd);
	switch(order)
	{	/*first group*/
		case 0:	/*mov*/
		case 1:	/*cmp*/
		case 2:	/*add*/
		case 3:	/*sub*/
		case 6:	/*lea*/
		{
			first= operand(sen, &i, ins, head, ic, lineC);/*call for function for one operand.
										counts memory words and if the operand is label -codes its addreaa*/
			for(; sen[i] ==' ' || sen[i] =='\t'; i++){}/*Move over white characters*/
			i++; /*Assume there is a comma and skip it - first scan performs a check*/
			second= operand(sen, &i, ins, head, ic, lineC);/*call for function for one operand.
										counts memory words and if the operand is label -codes its addreaa*/
			return (first||second);
		}/*close first group*/
			break;
		
		/*third group*/
		case 14:	/*rts*/
		case 15:	/*hlt*/
			return 0;
			break;/*close third group*/
			
		/*second group: not, clr, inc, dec, jmp, bne, get, prn, jsr*/
		default:
		{
			second= operand(sen, &i, ins, head,  ic, lineC);/*call for function for one operand.
										counts memory words and if the operand is label -codes its addreaa*/
			return second;
		}/*close second group*/
		
	}/*close swhitch*/
	return 0;
}/*end caseOrderSec*/

/*The function checks for valid operand and coding it into the instruction image.
Return -1 if there is an error. otherwise -0.
Variables: sen = the rest of the order sentence
			i = a pointer to the values of sen. (updating it to continue from the last char)
			ins= pointer on the array of the instruction image.
			head - pointer to the first node on the list.
			ic = the address of the first empty node in the ins array.
			lineC= the number of the line in the file.*/
int operand(char sen[], int *i, short (*ins)[], lablePoint *head, int *ic, int lineC){
	
	lablePoint pl;
	int j=0;
	char word [MAX_SENT];
	memset(word,'\0',MAX_SENT-1);
	for(; sen[*i]==' ' || sen[*i]=='\t'; (*i)++){} /*Move over white characters*/
	
	if(sen[*i] =='#'){/*integer operand*/
		for(;sen[*i]!=' ' && sen[*i]!='\t' && sen[*i]==',' && sen[*i]=='\n';(*i)++){}
		(*ic)++;
		return 0;
	}/*close if*/
	if(sen[*i] =='r'){
		if(sen[(*i)+1]>='0' && sen[(*i)+1]<='9'){
			if(sen[(*i)+2]==' ' || sen[(*i)+2]=='\t' || sen[(*i)+2]==','|| sen[(*i)+2]=='\n'){/*is a register*/
				(*ic)++;
				(*i)+=2;
				return 0;
			}/*close if-register*/
		}/*close if-0-9*/
	}/*close if-r*/	
	/*lable or struct oprand----------------*/
	for(; isalnum(sen[*i]);(*i)++){
		word[j]= sen[*i];
		j++;
	}/*close for*/
	if(sen[*i]=='.'){/*struct*/
		(*ic)+=2;
		(*i)+=2;}
	else /*label*/
		(*ic)++;
	if(strcmp((*head)->name, "")==0){/*the list is empty*/
		printf ("\nERROR, line %d- undefined label as operand\n", lineC);
		return 1;
	}/*close if*/
	pl= (*head);
	while(pl && strcmp(pl->name, word)!=0){
		pl=pl->next;}
	if(!pl){/*there is no label in list with name = word*/
		printf ("\nERROR, line %d- undefined label as operand\n", lineC);
		return 1;
	}/*close if*/
	(*ins)[*ic]=pl->add;
	if(pl->sign =='x')
		pl->ext = (*ic)+100;
	
	return 0;
	
}/*end operand*/
