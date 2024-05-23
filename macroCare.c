/*This file is the function "spreadMacro", the pre-asembler level.*/
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include "data.h"

/*Declaration on structer for the macros list. 
  Each node has 3 filds: the macro's name, the macro's text and a pointer for the next in list:*/
typedef struct node * macPoint;
typedef struct node {
	char  text[MAX_MAC_LENG];
	char  name[MAX_SENT];
	macPoint next;
	} macNode;

/*Fuctions declaration:*/
void checkAndAdd(macPoint *, FILE *, FILE *); 
void freeList(macPoint *);
char getWord(FILE *, char [MAX_SENT]);

/*-------------------------------------------------
The function 'spreadMacro' reads the given file, and writes it to a new file without the macros.
Returns: void.
Variables: fd - a pointer to the given file. name - the name of the given file (without the ending).*/

void spreadMacro (FILE *fd, char *name){

	macPoint h = NULL;
	macPoint p = NULL;
	char word[MAX_SENT];
	FILE *noMac;
	char *newEnding = ".am";
	char space;
	
printf("\nstart spreadMAcro:34	\n");
	
	strcat(name , newEnding);
	noMac = fopen(name, "w+");

	if(!noMac){ /*opening writing file and check.*/
		printf("/n Error, could not open file %s /n", name);
		return;
	}/*close if.*/
	while(feof(fd) ==0){ /*while we havn't reached the end of file.*/
		space= getWord(fd, word); /*call for function that reads a full word from the file.
					  and return the space/tab/enter that after*/
printf("\nword is: %s:48 \n", word);
		if(strcmp(word, "macro")==0)/*if the readen word is 'macro'.*/
			checkAndAdd(&h, fd, noMac); /*call for function that checkes if it's a valid macro and adds it to the list.*/
		else{ /*if the word isnot 'macro'.*/
			if(h){ /*if there allready exist a macro*/

				/*Check if the word is a call for a macro:*/
				p = h;
				while (p && strcmp(word, p->name)!=0){
					p = p->next;
				}
				if(p) /*there is a macro that the word is its name.*/
					fprintf(noMac,"%s", p->text);
				else /*the word is not 'macro' and not a call for one.*/
				fprintf(noMac,"%s%c", word, space);
			}/*close if*/
			else /*the word is not 'macro' and not a call for one.*/
				fprintf(noMac,"%s%c", word, space);
		}/*close else.*/
	}/*close while loop.*/

	fclose(fd);
	fclose(noMac);
	if(h)
		freeList(&h); /*call for a function that frees the macro list.*/
	return;

}/*end spreadMacro.*/

/*-----------------------------------------------------------------------------
 The function 'freeList' frees the list that the given pointer points at its first node.
 Returns: void.
 Variable: head -the pointer to the first node in the list.*/
void freeList(macPoint *head){
	macPoint p;
	while (*head){
		p= *head;
		*head = p->next;
		free(p);
	}/*close while loop.*/
}/*end freeList. */

/*-----------------------------------------------------------------------------
The function 'checkAndAdd' checks if the new macro has a valid and exclusive name.
	If it has, the function adds the macro and its values to the macro list.
	Otherwise, the function checks if there is a macro named 'macro', then it's a call for the macro,
				and the function writes the text of the macro to the new file.
		If there is no such macro,then the function identifies the word 'macro' as a coding word 
			and writes it, and the rest of the sentence, to the new file.
 Returns: void.
 Variables: head -pointer to the first node in the list.
			r -pointer to the file to read from.
			w -pointer to the file to write to.*/
void checkAndAdd(macPoint *head, FILE *r, FILE *w){
	
	int errName=0,errName2=0, i=0;
	char macName[MAX_SENT]; /*to keep the macro's name.*/
	char sen[MAX_SENT];        /*to keep a full sentence.*/
	char first[MAX_SENT];      /*to keep the first word of every line in the macro's text.*/
	macPoint pl= NULL; 
	macPoint new;
	char c;
	
	strcpy(macName , "");
	/*---Valid name check: 
	   getting the rest of the sentence from the text, then chacking if it includes only one word from letters or numbers only.*/
	fgets(sen, MAX_SENT, r);
	c=sen[0];
	while(c==' ' || c=='\t'){ /*finding the start of the name*/
		i++;
		c=sen[i];
	}/*close while loop.*/
	if(!isalpha(c))/*check if the first letter is alphabetic.*/
		errName = 1;
	while(c!= '\n'){ 
		if(isalnum(c)) /*check if the letter is alphanumeric.*/
			 strncat (macName, &c, 1);
		else { /*the name is invalid.*/
			errName2 = 1;
			break;
		}/*close else.*/
		i++;
		c=sen[i];
	}/*close while loop.*/
	if(errName2){/*check if after the name there are only white spaces*/
		while(c==' ' || c=='\t'){
			errName2 =0;
			i++;
			c=sen[i];
		}/*close while loop*/
		if(c!='\n')/*there is a not alphanumeric character after the name*/
			errName2 =1;
	}/*close if*/
	
	if(errName || errName2 || strcmp(macName, "")==0){ /*if the name isnot valid or missing.*/
		if(*head){/*check if there is a macro-node that named "macro".*/
			pl = *head;
			while (pl && strcmp(pl->name, "macro")!=0)
				pl=pl->next;		
		}/*close if*/
		if(pl) /*"macro" is a call for a macro named "macro".*/
			fprintf(w,"%s", pl->text);
		else  /*"macro" is a coding word.*/
			fprintf(w, "macro%s", sen);
		return;
	}/*close if.*/

	/*---Exclusive name check: scanning the macro list. for each node check if the name is equal to the new one.*/
	if(*head){
		pl =*head;
		while(pl && strcmp(macName, pl->name)) /*while there is a macro-node with a different name.*/
			pl = pl->next;
	}/*close if*/
	if(pl){ /*pl isn't null, so it points to a macro-node with the same name.*/
		fprintf(w, "macro%s", sen);
		return;
	}/*close if.*/
 	
/*---Building a new macro-node, setting the fields, and placing it in the macro-list:*/
	new = (macPoint)malloc(sizeof(macNode));
	strcpy(new->name , macName);/*setting the name field.*/
	strcpy(new->text ,"");
	if(!feof(r)){ /*setting the text field.*/
		getWord(r, first);
		while(strcmp(first, "endmacro")){
			fgets(sen, MAX_SENT, r);
			strcat(new->text , first);
			strcat(new->text , " ");
			strcat(new->text, sen); /*adding a sentence from the macro body to the text field.*/
			getWord(r, first);
		}/*close while loop.*/
	}/*close if*/
	new->next = *head; /*setting the next field, by placing at the head of the list.*/
	*head = new;
	return;
}/*end checkAndAdd.*/


