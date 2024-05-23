/*this file is the auxiliary functions.*/
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
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

int isOrder(char *);/*check if word is an instruction name.*/
int isSavedWord(char *);/*check if word is a saved word*/
int checkLable(char [],lablePoint);/*check if lable name is valid*/
void addLable(lablePoint *, int, char, char *);/*adds new lable to the list*/
void updateList(lablePoint *, int );/*update theaddress of data labels*/
void freeListL(lablePoint *);

/*The function checks if the given word is a name of an instruction.
 if it is, returns the opcode number of the instruction. otherwise, returns -1.*/
int isOrder(char * word){

	char *orders[17] =  {"mov", "cmp", "add", "sub", "not", "clr", "lea", "inc", "dec",
						 "jmp", "bne", "get", "prn", "jsr", "rts", "hlt", NULL};
	int i;
	for(i=0; orders[i]!=NULL; i++)
		if(strcmp(word, orders[i])==0)
			return i;
	return -1;
}

/*The function checks if the given word is a saved word.
 if it is, returns 1. otherwise, returns 0.*/
int isSavedWord(char *word){
	int i=0;
	char *saved[16] ={"entry", "extern", "data", "strung", "struct",
					 "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7", "r8", "r9", NULL};
	for(; saved[i]!=NULL; i++){
		if(strcmp(word, saved[i])==0)
			return 1;
	}
	if(isOrder(word)+1)
		return 1;
	return 0;
}

/*The function checks if the given word is a valid and exclusive lable name.
 if it is, returns 0. otherwise, returns 1.
Variables: word - the word to check. head - pointer to the first node at the lable-list*/
int checkLable(char word[],lablePoint head){
	lablePoint pl=NULL;
	int i;
	if(!isalpha(word[0])) /*check if the first char isnot an alphabetiic*/
		return 1;
	for(i=1; word[i]!='\0'; i++)
		if(!isalnum(word[i])) /*check for not alphanumeric chars*/
			return 1;
	if(i>MAX_LABLE) /*check if the word is over 30 chars.*/
		return 1;
	if(isSavedWord(word))/*check if the word is a saved word.*/
		return 1;
	if(strcmp(head->name, ""))/*check if the lable-list isnot empty*/
		{
		pl = head;			/*check if the lable-name is exclusive*/
		while(pl && strcmp(pl->name, word))
			pl = pl->next;
		}
	if(pl) /*the lable-name isnot exclusive*/
		return 1;
	return 0;
}

/*The function adds a new lable to the list. returns void.
Variables: head - pointer to the first node on the list.
			address - the address to put on the add value of the lable node.
			c - the chracter sign to put on the sign value of the lable node.
			word - the name of the lable .*/
void addLable(lablePoint *head, int address, char c, char * word)
{
	lablePoint new, pl =NULL;
	
printf("\n AL word to add to list: '%s'\n",word);

	new=(lablePoint)malloc(sizeof(labelNode));/*space allocation*/
	strcpy(new->name , word);/*update name*/
		
	new->add = address;		/*update address*/
	new->sign = c;			/*update sign*/
	new->ent=0;				/*update entery flag*/
	new->ext=0;
	new->next = NULL;		/*update next*/
	
	/*Placement on the list:*/
	if(strcmp((*head)->name, "")==0){/*the list is empty*/
		*head = new;
		return;}
	pl=*head; 	/*the list isnot empty*/
	while(pl->next){ /*finding the last node on the list*/
		pl=pl->next;}
	pl->next = new;
	return;
}

/*The function updates the address of lables of type data, by adding ic.
 Returns: void.
 Variables: head - pointer to the first node on the label list.
 			ic - the number to update the address with.*/
void updateList(lablePoint *head, int ic){
printf("\nUL start  updatelist: 108\n");
	lablePoint pl=NULL;
	
	if(strcmp((*head)->name, "")==0)/*the list is empty*/
		return;
	pl=*head; 	/*the list isnot empty*/
	while(pl){
		if(pl->sign == 'd')
			pl->add+= ic;
		pl= pl->next;
	}/*close while loop*/
	return;
}/*end 	updateList*/

/*-----------------------------------------------------------------------------
 The function 'freeList' frees the list that the given pointer points at its first node.
 Returns: void.
 Variable: head -the pointer to the first node in the list.*/
void freeListL(lablePoint *head){
	lablePoint p;
	while (*head){
		p= *head;
		*head = p->next;
		free(p);
	}/*close while loop.*/
}/*end freeList. */

