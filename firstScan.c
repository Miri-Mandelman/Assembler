/*This file is for the first-scan function*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
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
int isSavedWord(char *);/*check if word is a saved word*/
int checkLable(char [],lablePoint);/*check if lable name is valid*/
void addLable(lablePoint *, int, char, char *);/*adds new lable to the list*/

int caseExtern(char, FILE *,lablePoint *, int);/*function for extern instruction.*/
int caseData(char, FILE *, short (*)[], int *, int);/*function for data instruction.*/
int caseString(char, FILE *, short (*)[], int *, int);/*function for string instruction.*/
int caseStruct(char, FILE *, short (*)[], int *, int);/*function for struct instruction.*/
int caseOrder(char, FILE *, short (*)[], int *, int, int);/*function for an order sentence.*/

int firstOper(char [], int *, short (*)[], int *, int *, int);/*function for the source operand of an order sentence.*/
int firstOperTwo(char [], int *, short (*)[], int *, int *, int);/*function for the source operand of 'lea' order sentence.*/
int secondOper(char [], int *, short (*)[], int *, int , int);/*function for the destination operand of an order sentence.*/
int secondOperTwo(char [], int *, short (*)[], int *, int , int);/*function for the destination operand of 'prn' or 'cmp' order sentence.*/

/*The function operate step "first scan" on the file - checking for errors , saving lables and start the coding.
Returns: 1-if there are errors. or 0 - if there aren't.
Variables:  fd= the file to read from.
			ins= pointer on the array of the instruction image.
			data= pointer on the array of the data image.
			head= points to the first node of the lables-list.
			ic= the address of the first empty node in the ins array.
			dc= the address of the first empty node in the data array.
			ext= flag to update to 1 if there is a valid extern instruction.*/
int firstScan(FILE *fd, short (*ins)[], short (*data)[], lablePoint *head, int *ic, int *dc, int *ext)
{
	/*Variables:*/
	int i;
	int lineC =0; /*line counter*/
	char word[MAX_SENT];
	char secWord[MAX_SENT];
	char sen[MAX_SENT];
	int errors =0;/*incase of an error, changed to 1*/
	char c;
	int valC;/*counter of values*/
	int order;/*the opcode number of an order name*/

	while(feof(fd) ==0){
		c=getWord(fd,word);
		lineC++;
		/*Comment sentence, empty sentence, or entry instruction:------*/
		if(word[0] ==';' || strcmp(word, "")==0 || strcmp(word, ".entry")==0)
		{	
			if(c!='\n'){/*Skipping the sentence*/
				fgets(sen, MAX_SENT, fd);
				memset(sen,'\0',MAX_SENT-1);
			}
		}/*end if, comment, empty or entry*/

		/*extern instruction:-------------------------*/
		else if(strcmp(word, ".extern")==0)
		{
			if(!caseExtern(c,fd,head,lineC))/*call for a function for extern instruction.*/
				errors=1;
			else
				(*ext)=1;
		}/*end if, extern.*/
		
		/*data instruction:-------------------------*/
		else if(strcmp(word, ".data")==0)
		{
			valC= caseData(c, fd, data, dc, lineC);/*call for a function for data instruction.
								return -the number of values , or 0 for an error.*/
			if(valC)/*there was no error*/
				(*dc)+=valC;
			else
				errors= 1;
		}/*end if, data*/

		/*string instruction:-------------------------*/
		else if(strcmp(word, ".string")==0)
		{
			valC= caseString(c, fd, data, dc, lineC);/*call for a function for string instruction.
								return -the number of characters+1 , or 0 for an error.*/
			if(valC)/*there was no error*/
				(*dc)+=valC;
			else
				errors= 1;
		}/*end if, string*/
		
		/*struct instruction:-------------------------*/
		else if(strcmp(word, ".struct")==0)
		{
			valC= caseStruct(c, fd, data, dc, lineC);/*call for a function for struct instruction.
				return -the number of values (number + number of characters+1), or 0 for an error.*/
			if(valC)/*there was no error*/
				(*dc)+=valC;
			else
				errors= 1;
		}/*end if, struct*/

		/*order sentence----------------------------*/
		else if((order= isOrder(word))+1) /*call for function that returns the opcode number of an order name, 
											or -1,if it's not an order*/
		{
			valC= caseOrder(c, fd, ins, ic, lineC, order);/*call for a function for an order sentence.
							return -the number of memory words , or 0 for an error.*/
			if(valC)/*there was no error*/
				(*ic)+=valC;
			else
				errors= 1;
		}/*end if, order*/
		
		else{

			for(i=0; word[i]!='\0'; i++){}/*finding the last char*/
/*-----------definition on label--------------------*/
			if(word[i-1]==':'){
				if(c=='\n'){
					printf ("\nERROR, line %d- missing value for label\n", lineC);
					errors=1;
				}/*end if*/
				else{
					c= getWord(fd, secWord);
					if(strcmp(secWord, ".entry")==0){ /*---entry---*/
						printf ("\nWARNING, line %d- label before .entry ,code ignore label \n", lineC);
						if(c!='\n'){
							fgets(sen, MAX_SENT, fd);
							memset(sen,'\0',MAX_SENT-1);
						}/*close if*/
					}/*close if, entry*/
					else if (strcmp(secWord, ".extern")==0)/*---extern---*/
					{
						printf ("\nWARNING, line %d- label before .extern ,code ignore label \n", lineC);
						if(!caseExtern(c,fd,head,lineC))/*call for a function for extern instruction.*/
							errors=1;
					}/*end if, extern.*/
					else{ /*label to put in the lable list*/
						word[i-1]='\0';/*delete the :*/
						if(checkLable(word,*head)){/*call for function that checks if the lable name is valid: 
										if it's not -returns 1.*/
							printf ("\nERROR, line %d- invalid label name.\n", lineC);
							errors=1;
						}/*close if*/
						else if(strcmp(secWord, ".data")==0){/*----data---*/
							valC= caseData(c, fd, data, dc, lineC);/*call for a function for data instruction.
										return -the number of values , or 0 for an error.*/
							if(valC){/*there was no error*/
								addLable(head, *dc, 'd', word);/*call for a function that add the lable
										to the lable-list, with address value=dc,and sign='d'*/
								(*dc)+=valC;
							}/*end if- no error*/
							else
								errors= 1;
						}/*end if, data*/
						else if(strcmp(secWord, ".string")==0){ /*---string---*/
							valC= caseString(c, fd, data, dc, lineC);/*call for a function for string 
							instruction.return -the number of characters+1 , or 0 for an error.*/
							if(valC){/*there was no error*/
								addLable(head, *dc, 'd', word);/*call for a function that add the lable 
										to the lable-list, with address value=dc,and sign='d'*/
								(*dc)+=valC;
							}/*end if- no error*/
							else
								errors= 1;
						}/*end if, string*/
						else if(strcmp(secWord, ".struct")==0){ /*---struct---*/
							valC= caseStruct(c, fd, data, dc, lineC);/*call for a function for struct 
				instruction.return -the number of values (number + number of characters+1), or 0 for an error.*/
							if(valC){/*there was no error*/
								addLable(head, *dc, 'd', word);/*call for a function that add the lable 
									to the lable-list, with address value=dc,and sign='d'*/
								(*dc)+=valC;
							}/*end if- no error*/
							else
								errors= 1;
						}/*end if, struct*/
						else{
							order= isOrder(secWord);/*call for function that returns the opcode number of 
											an order name,or -1, if it's not an order*/
							if(order+1){ 
								valC= caseOrder(c, fd, ins, ic, lineC, order);/*call for a function for an order 
								sentence. return -the number of memory words , or 0 for an error.*/
								if(valC){/*there was no error*/
									addLable(head, *ic, 'i', word);/*call for a function that add the lable 
										to the lable-list, with address value= ic,and sign='i'*/
									(*ic)+=valC;
								}/*end if- no error*/
								else
									errors= 1;
							}/*end if, order*/
							else{ /*invalid label value*/
								printf ("\nERROR, line %d- invalid value for label.\n", lineC);
								errors=1;
							}/*close else*/
						}/*end else*/
					}/*close else- label to put in the lable list*/	
				}/*close else*/
			}/*close if, label*/

			else{ /*word is not label*/
				printf ("\nERROR, line %d- unidentified word.\n", lineC);
				errors=1;
				if(c!='\n'){/*Skipping the sentence*/
				fgets(sen, MAX_SENT, fd);
				memset(sen,'\0',MAX_SENT-1);
				}
			}/*close else- not label*/
		}/*close else*/
	
	}/*end while loop*/
	return errors;

}/*end firstScan*/

/*The function takes care of an extern instruction:
	checks for valid lable after '/extern' and adds it to the lables-list.
Returns: 0 - incase of any error, otherwise -1.
Variables:  c= the character that was readen from the text after '.extern'.
			fd= the file to read from.
			head= points to the first node of the lables-list.
			lineC= the number of the line in the file.*/
int caseExtern(char c, FILE * fd, lablePoint *head, int lineC){

	char word[MAX_SENT];

	if (c=='\n'){
		printf ("\nERROR, line %d- missing lable name after .extern\n", lineC);
		return 0;
	}/*end if*/
	c= getWord(fd,word);
	if (strcmp(word, "")==0){/*only white characters after '.extern'*/
		printf ("\nERROR, line %d- missing lable name after .extern\n", lineC);
		return 0;
	}/*end if*/
	if(checkLable(word, *head)){/*call for function that checks if the lable name is valid: if it's not -returns 1.*/
		printf ("\nERROR, line %d- invalid lable name after .extern\n", lineC);
		return 0;
	}/*end if*/
	addLable( head, -1, 'x', word);/*call for a function that add the lable to the lable-list, with address value=-1,and sign='x'*/
	while(c!='\n'){/*check for extra in sentence:*/
		if(c!=' ' && c!='\t'){
			printf ("\nERROR, line %d- extra texst in sentence\n", lineC);
			fgets(word, MAX_SENT, fd);
			return 0;
		}/*end if*/
		fgetc(fd);
	}/*end while*/
	return 1;
}/*end caseExtern*/

/*The function takes care of an data instruction:
	checks for valid values and coding them into the data image.
Return -0 if there is an error. otherwise -the number of values.
Variables: c= the character that was readen from the text after '.data'.
			fd= the file to read from.
			data= pointer on the array of the data image.
			dc= the address of the first empty node in the data array.
			lineC= the number of the line in the file.*/
int caseData(char c, FILE * fd, short (*data)[], int *dc, int lineC){

	char sen[MAX_SENT];
	char word[MAX_SENT];
	int i=0, j=0, pos=1,openComma=0, valC;

	if(c=='\n'){
		printf ("\nERROR, line %d- missing values after .data\n", lineC);
		return 0;
	}/*end if*/
	fgets(sen, MAX_SENT, fd);
	for(; sen[i] ==' ' || sen[i] =='\t'; i++){}/*Move over white characters*/
	if(sen[i]=='+')
		i++;
	else if(sen[i]=='-'){
		i++;
		pos=0;
	}/*end else-if*/
	memset(word, '\0', MAX_SENT-1);
	for(; isdigit(sen[i]);i++){/*reading first number*/
		valC=1;
		word[j]=sen[i];
		j++;
	}/*end for loop*/
	if(!valC){/*invalid value*/
		printf ("\nERROR, line %d- invalid values after .data\n", lineC);
		return 0;
	}/*end if*/
	if(pos)/*first number is positive*/
		(*data)[*dc] = atoi(word);/*coding positive value into data image*/
	else/*first number is negative*/
		(*data)[*dc] =pow(2,10) -atoi(word);/*coding negative value into data image, using the method of complements to 2*/

	while(sen[i] !='\n' && !openComma){/*coding the rest of the values*/
		if(sen[i]!=' ' && 	sen[i]!='\t' && sen[i]!=',' ){/*check for invalid char*/
			printf ("\nERROR, line %d- invalid value after .data\n", lineC);
			return 0;
		}/*end if*/
		for(; sen[i] ==' ' || sen[i] =='\t'; i++){}/*Move over white characters*/
		if(sen[i]!='\n' && sen[i]!=','){/*Comma check between values*/
			printf ("\nERROR, line %d- missing comma between values\n", lineC);
			return 0;
		}/*end if*/
		if(sen[i] !='\n'){
			openComma =1;/*there is a comma*/
			pos =1;
			i++;
			for(; sen[i] ==' ' || sen[i] =='\t'; i++){}/*Move over white characters*/
			if(sen[i]=='+')
				i++;
			else if(sen[i]=='-'){
				i++;
				pos=0;
			}/*end else-if*/
			memset(word, '\0', MAX_SENT-1);
			j=0;
			for(; isdigit(sen[i]);i++){/*reading next number*/
				openComma =0;
				word[j]=sen[i];
				j++;
			}/*close for loop*/
			if(!openComma){/*there is another number*/
				valC++;
				if(pos)/* number is positive*/
					(*data)[(*dc)+valC] = atoi(word);/*coding positive value into data image*/
				else/* number is negative*/
				(*data)[(*dc)+valC] =pow(2,10) -atoi(word);/*coding negative value into data image, using the method of complements to 2*/
				}/*close if- openComma*/
		}/*close if*/				
	}/*close while loop*/
	if(openComma){/*extra comma*/
		printf ("\nERROR, line %d- extra comma in .data\n", lineC);
		return 0;
	}/*close if*/
	return valC;
	
}/*end caseData*/

/*The function takes care of an string instruction:
	checks for valid text and coding it into the data image.
Return -0 if there is an error. otherwise -the number of characters in text +1(end of text).
Variables: c= the character that was readen from the text after '.string'.
			fd= the file to read from.
			data= pointer on the array of the data image.
			dc= the address of the first empty node in the data array.
			lineC= the number of the line in the file.*/
int caseString(char c, FILE * fd, short (*data)[], int *dc, int lineC){
	
	char sen[MAX_SENT];
	int i=0, valC=0;

	if(c=='\n'){
		printf ("\nERROR, line %d- missing values , expecting type string\n", lineC);
		return 0;
	}/*end if*/
	while(c==' ' || c=='\t')/*Move over white characters*/
		c=fgetc(fd);
	if(c!='"'){ /*check openning quotation marks*/
		printf ("\nERROR, line %d- missing openning quotation marks in type string\n", lineC);
		fgets(sen, MAX_SENT, fd);
		return 0;
	}/*end if*/
	fgets(sen, MAX_SENT, fd);
	for(; sen[i]!='\n'; i++){}/*finding the end of sentence*/
	for(; sen[i]=='\n' || sen[i]==' ' || sen[i]=='\t'; i--)/*delete white characters from the end*/
		sen[i] ='\0';
	if(c!='"'){ /*check closing quotation marks*/
		printf ("\nERROR, line %d- missing closing quotation marks in type string\n", lineC);
		return 0;
	}/*end if*/
	sen[i] = '\0';/*delete the closing quotation marks*/
	
	for(i=0; sen[i]!='\0'; i++){/*coding text to the data image*/
		(*data)[i] =(int)sen[i];
		valC++;
	}/*close for loop*/
	(*data)[i] =0;/*coding 'end of text'*/
	valC++;
	return valC;

}/*end caseString*/

/*The function takes care of an struct instruction:
	checks for valid text and coding it into the data image.
Return -0 if there is an error. otherwise -the number of characters in text +2(end of text + the numbar filed).
Variables: c= the character that was readen from the text after '.struct'.
			fd= the file to read from.
			data= pointer on the array of the data image.
			dc= the address of the first empty node in the data array.
			lineC= the number of the line in the file.*/
int caseStruct(char c, FILE * fd, short (*data)[], int *dc, int lineC){

	char word[MAX_SENT];
	int pos=1, j=0, valC=0;

	if(c=='\n'){
		printf ("\nERROR, line %d- missing values after .struct\n", lineC);
		return 0;
	}/*end if*/
	while(c==' ' || c=='\t')/*Move over white characters*/
		c=fgetc(fd);
	if(c=='+')
		c=fgetc(fd);
	else if(c=='-'){
		pos=0;
		c=fgetc(fd);
	}/*close if*/
	if(!isdigit(c)){
		printf ("\nERROR, line %d- invalid value after .struct- expecting integer\n", lineC);
		fgets(word, MAX_SENT, fd);
		return 0;
	}/*end if*/
	for(; isdigit(c); j++){/*reading the integer value*/
		word[j] =c;
		c=fgetc(fd);
	}/*close for loop*/
	while(c==' ' || c=='\t')/*Move over white characters*/
		c=fgetc(fd);
	if(c!=','){/*check for comma between values*/
		printf ("\nERROR, line %d- missing comma between values\n", lineC);
		fgets(word, MAX_SENT, fd);
		return 0;
	}/*end if*/
	c=fgetc(fd);
	if(pos)/*the number is positive*/
		(*data)[(*dc)] = atoi(word);/*coding positive value into data image*/
	else/*the number is negative*/
		(*data)[(*dc)] =pow(2,10) -atoi(word);/*coding negative value into data image, using the method of complements to 2*/
	
	valC= caseString(c, fd, data, dc, lineC);/*call for a function for string type.
													return -the number of characters+1 , or 0 for an error.*/
	if(!valC)/*there was no error*/
		return 0;
	return (valC+1);
}/*end caseStruct*/

/*The function takes care of an order sentence:
	checks for valid operands and coding them, and the order sentence, into the instruction image.
Return -0 if there is an error. otherwise -the number of memory words that was coded.
Variables: c= the character that was readen from the text after the order name.
			fd= the file to read from.
			ins= pointer on the array of the instruction image.
			ic= the address of the first empty node in the ins array.
			lineC= the number of the line in the file.
			order= the opcode number of the order name.*/
int caseOrder(char c, FILE * fd, short (*ins)[], int *ic, int lineC, int order){

	char sen[MAX_SENT];
	int memWs=1;/*counter of the memory words*/
	int i=0;
	int reg=0;/*flug for register source operands*/
	int first, second; /*flugs for valid first/second operands*/

	(*ins)[(*ic)] = (order << 6); /*coding the opcode of the order name to the instruction image*/
	if(c!='\n')	
		fgets(sen, MAX_SENT, fd);
	switch(order)
	{	/*first group*/
		case 0:	/*mov*/
		case 1:	/*cmp*/
		case 2:	/*add*/
		case 3:	/*sub*/
		case 6:	/*lea*/
		{
			if(c=='\n'){
				printf ("\nERROR, line %d- missing operands\n", lineC);
				return 0;
			}/*end if*/
			if(order!=3)
				first = firstOper(sen, &i, ins, &memWs, &reg, *ic);/*call for function that checks and add the first operand.
																returns 0 if there is an error, otherwise- 1.*/
			else /*lea*/
				first = firstOperTwo(sen, &i, ins, &memWs, &reg, *ic);/*call for function that checks and add the first operand.
																			for 'lea' order 
																returns 0 if there is an error, otherwise- 1.*/
			if(!first){
				printf ("\nERROR, line %d- invalid source operand\n", lineC);
				return 0;
			}/*end if*/
			for(; sen[i] ==' ' || sen[i] =='\t'; i++){}/*Move over white characters*/
			if(sen[i]!=','){
				printf ("\nERROR, line %d- missing comma between operands\n", lineC);
				return 0;
			}/*end if*/
			i++;	/*move over comma*/
			second = secondOper(sen, &i, ins, &memWs, reg, *ic);/*call for function that checks and add the second operand.
																returns 0 if there is an error, otherwise- 1.*/
			if(!second){
				printf ("\nERROR, line %d- invalid destination operand\n", lineC);
				return 0;
			}/*end if*/
			for(; sen[i] ==' ' || sen[i] =='\t'; i++){}/*Move over white characters*/
			if(sen[i]!='\n'){
				printf ("\nERROR, line %d- extra text in sentence\n", lineC);
				return 0;
			}/*end if*/
			return memWs;/*updating in first/second functions*/
		}/*close first group*/
			break;
		/*third group*/
		case 14:	/*rts*/
		case 15:	/*hlt*/
		{
			if(c!='\n'){
				for(; sen[i] ==' ' || sen[i] =='\t'; i++){}/*Move over white characters*/
				if(sen[i]!='\n'){
					printf ("\nERROR, line %d- extra text in sentence\n", lineC);
					return 0;
				}/*end if*/			
			}/*close if*/
			return memWs;
		}/*close third group*/
			break;
		/*second group: not, clr, inc, dec, jmp, bne, get, prn, jsr*/
		default:
		{
			if(c=='\n'){
				printf ("\nERROR, line %d- missing operand\n", lineC);
				return 0;
			}/*end if*/
			if(order!= 1 && order!= 12)
				second = secondOper(sen, &i, ins, &memWs, reg, *ic);/*call for function that checks and add the destination operand.
																returns 0 if there is an error, otherwise- 1.*/
			else/*prn , cmp*/
				second = secondOperTwo(sen, &i, ins, &memWs, reg, *ic);/*call for function that checks and add the destination operand.
																			for 'prn', or 'cmp' order 
																returns 0 if there is an error, otherwise- 1.*/
			if(!second){
				printf ("\nERROR, line %d- invalid destination operand\n", lineC);
				return 0;
			}/*end if*/
			for(; sen[i] ==' ' || sen[i] =='\t'; i++){}/*Move over white characters*/
			if(sen[i]!='\n'){
				printf ("\nERROR, line %d- extra text in sentence\n", lineC);
				return 0;
			}/*end if*/
			return memWs;/*updating in secondOper functions*/
		}/*close second group*/
		
	}/*close swhitch*/
	return 0;
}/*end caseOrder*/

/*The function checks for valid source operand and coding it into the instruction image.
Return -0 if there is an error. otherwise -1.
Variables: sen = the rest of the order sentence
			i = a pointer to the values of sen. (updating it to continue from the last char)
			ins= pointer on the array of the instruction image.
			memWords= counter of the memory words that coded (updating it to update ic if there is no error).
			regin= flag for source register(update to 1 if the source operand is register).
			ic = the address of the first empty node in the ins array.*/
int firstOper(char sen[], int *i, short (*ins)[], int *memWords, int *regis, int ic){

	int regNum, j=0, pos=1;
	char word [MAX_SENT];

	memset(word,'\0',MAX_SENT-1);
	for(; sen[*i]==' ' || sen[*i]=='\t'; (*i)++){} /*Move over white characters*/
	/*integer operand------------------------*/
	if(sen[*i] =='#'){
		(*i)++;
		if(sen[*i] =='+')
			(*i)++;
		else if(sen[*i] =='-'){
			pos=0;
			(*i)++;
		}/*close if*/
		if(!isdigit(sen[*i]))/*there is no a number after #*/
			return 0;
		for(; isdigit(sen[*i]); (*i)++) {
			word[j] = sen[*i];
			j++;
		}/*close for loop*/
		if(pos)
			(*ins)[ic+1] = (atoi(word)<<2);/*coding positive number + A ending(00)*/
		else
			(*ins)[ic+1] = (((int)(pow(2,10) - atoi(word)))<<2);/*coding positive number + A ending(00)*/
		(*memWords)++; 
		return 1; /* the addressing methode of the source operand is by difault 0*/
	}/*close if- integer */
	/*register operand-------------------*/
	if(sen[*i] =='r'){
		if(sen[(*i)+1]>='0' && sen[(*i)+1]<='9'){
			regNum = (sen[(*i)+1] -'0');
			if(sen[(*i)+2]==' ' || sen[(*i)+2]=='\t' || sen[(*i)+2]==','){/*is a register*/
				(*ins)[ic+1]= (regNum<<6); /*coding source register + A ending(00)*/
				(*ins)[ic]+=(3<<4); /*coding the addressing methode of the source operand -register*/
				(*memWords)++;
				(*regis)=1;
				(*i)+=2;
				return 1;
			}/*close if-register*/
		}/*close if-0-9*/
	}/*close if-r*/	
	/*lable or struct oprand----------------*/
	for(; isalnum(sen[*i]);(*i)++){}
	if(sen[*i]=='.'){
		if(sen[(*i)+1]=='1' || sen[(*i)+1] =='2'){ /*----struct---*/
			if(sen[(*i)+2]==' ' || sen[(*i)+2]=='\t' || sen[(*i)+2]==','){
				(*ins)[ic]+=(2<<4); /*coding the addressing methode of the source operand -struct*/
				(*ins)[ic+2] = ((sen[(*i)+1]-'0')<<2); /*coding the number of field + A ending(00)*/
				(*memWords)+=2;
				(*i)+=2;
				return 1;
			}/*close if*/
		}/*end if-struct*/	
	}/*end if -'.'*/
	if(sen[*i]==' ' || sen[*i]=='\t'|| sen[*i]==','){ /*---lable----*/
		(*ins)[ic]+=(1<<4); /*coding the addressing methode of the source  operand -label*/
		(*memWords)++;
		return 1;
	}/*close if- label*/
	return 0;

}/*end firstOper*/

/*The function checks for valid source operand for 'lea' order and coding it into the instruction image.
Return -0 if there is an error. otherwise -1.
Variables: sen = the rest of the order sentence
			i = a pointer to the values of sen. (updating it to continue from the last char)
			ins= pointer on the array of the instruction image.
			memWords= counter of the memory words that coded (updating it to update ic if there is no error).
			regin= flag for source register(update to 1 if the source operand is register).
			ic = the address of the first empty node in the ins array.*/
int firstOperTwo(char sen[], int *i, short (*ins)[], int *memWords, int *regis, int ic){

	int regNum;

	for(; sen[*i]==' ' || sen[*i]=='\t'; (*i)++){} /*Move over white characters*/
	/*register operand-------------------*/
	if(sen[*i] =='r'){
		if(sen[(*i)+1]>='0' && sen[(*i)+1]<='9'){
			regNum = (sen[(*i)+1] -'0');
			if(sen[(*i)+2]==' ' || sen[(*i)+2]=='\t' || sen[(*i)+2]==','){/*is a register*/
				(*ins)[ic+1]= (regNum<<6); /*coding source register + A ending(00)*/
				(*ins)[ic]+=(3<<4); /*coding the addressing methode of the source operand -register*/
				(*memWords)++;
				(*regis)=1;
				(*i)+=2;
				return 1;
			}/*close if-register*/
		}/*close if-0-9*/
	}/*close if-r*/	
	/* struct oprand---------------------*/
	for(; isalnum(sen[*i]);(*i)++){}
	if(sen[*i]=='.'){
		if(sen[(*i)+1]=='1' || sen[(*i)+1] =='2'){ /*----struct---*/
			(*ins)[ic]+=(2<<4); /*coding the addressing methode of the source operand -struct*/
			(*ins)[ic+2] = ((sen[(*i)+1]-'0')<<2); /*coding the number of field + A ending(00)*/
			(*memWords)+=2;
			(*i)+=2;
			return 1;
		}/*end if-struct*/	
	}/*end if -'.'*/
	return 0;

}/*end firstOperTwo*/


/*The function checks for valid destination operand and coding it into the instruction image.
Return -0 if there is an error. otherwise -1.
Variables: sen = the rest of the order sentence
			i = a pointer to the values of sen. (updating it to continue from the last char)
			ins= pointer on the array of the instruction image.
			memWords= counter of the memory words that coded (updating it to update ic if there is no error).
			regin= flag for source register.
			ic = the address of the first empty node in the ins array.*/
int secondOper(char sen[], int *i, short (*ins)[], int *memWords, int regis, int ic){
	
	int regNum;
	int lable=0;

	for(; sen[*i]==' ' || sen[*i]=='\t'; (*i)++){} /*Move over white characters*/
	/*register operand-------------------*/
	if(sen[*i] =='r'){
		if(sen[(*i)+1]>='0' && sen[(*i)+1]<='9'){
			regNum = (sen[(*i)+1] -'0');
			if(sen[(*i)+2]==' ' || sen[(*i)+2]=='\t' || sen[(*i)+2]==','){/*is a register*/
				if(regis) /*there is a source operand of type register*/
					(*ins)[ic+1]= (regNum<<2); /*coding destination register + A ending(00)*/
				else{ /*no source operand of type register*/
					(*ins)[ic+(*memWords)]= (regNum<<2); /*coding destination register + A ending(00)*/
					(*memWords)++;
				}/*close else*/
				(*ins)[ic]+=(3<<2); /*coding the addressing methode of the destination operand -register*/
				(*i)+=2;
				return 1;
			}/*close if-register*/
		}/*close if-0-9*/
	}/*close if-r*/
	/*lable or struct oprand----------------*/
	for(; isalnum(sen[*i]);(*i)++)
		lable=1;
	if(lable && sen[*i]=='.'){
		if(sen[(*i)+1]=='1' || sen[(*i)+1] =='2'){ /*----struct---*/
			(*ins)[ic]+=(2<<4); /*coding the addressing methode of the destination operand -struct*/
			(*ins)[ic+ (*memWords) +1]= ((sen[(*i)+1]-'0')<<2); /*coding the number of field + A ending(00)*/
			(*memWords)+=2;
			(*i)+=2;
			return 1;
		}/*end if-struct*/	
	}/*end if -'.'*/
	if(lable){/*---label---*/
		(*ins)[ic]+=(1<<2); /*coding the addressing methode of the destination operand -label*/
		(*memWords)++;
		return 1;
	}/*close if- label*/
	return 0;

}/*end secondOper*/

/*The function checks for valid destination operand for 'cmp' or 'prn' order, and coding it into the instruction image.
Return -0 if there is an error. otherwise -1.
Variables: sen = the rest of the order sentence
			i = a pointer to the values of sen. (updating it to continue from the last char)
			ins= pointer on the array of the instruction image.
			memWords= counter of the memory words that coded (updating it to update ic if there is no error).
			regin= flag for source register.
			ic = the address of the first empty node in the ins array.*/
int secondOperTwo(char sen[], int *i, short (*ins)[], int *memWords, int regis, int ic){
	
	int regNum, j=0, pos=1;
	char word [MAX_SENT];
	int lable=0;

	memset(word,'\0',MAX_SENT-1);
	for(; sen[*i]==' ' || sen[*i]=='\t'; (*i)++){} /*Move over white characters*/
	/*integer operand------------------------*/
	if(sen[*i] =='#'){
		(*i)++;
		if(sen[*i] =='+')
			(*i)++;
		else if(sen[*i] =='-'){
			pos=0;
			(*i)++;
		}/*close if*/
		if(!isdigit(sen[*i]))/*there is no a number after #*/
			return 0;
		for(; isdigit(sen[*i]); (*i)++) {
			word[j] = sen[*i];
			j++;
		}/*close for loop*/
		if(pos)
			(*ins)[ic+(*memWords)] = (atoi(word)<<2);/*coding positive number + A ending(00)*/
		else
			(*ins)[ic+(*memWords)] = (((int)(pow(2,10) - atoi(word)))<<2);/*coding positive number + A ending(00)*/
		(*memWords)++; 
		return 1; /* the addressing methode of the destination operand is by difault 0*/
	}/*close if- integer */
	/*register operand-------------------*/
	if(sen[*i] =='r'){
		if(sen[(*i)+1]>='0' && sen[(*i)+1]<='9'){
			regNum = (sen[(*i)+1] -'0');
			if(sen[(*i)+2]==' ' || sen[(*i)+2]=='\t' || sen[(*i)+2]==','){/*is a register*/
				if(regis) /*there is a source operand of type register*/
					(*ins)[ic+1]= (regNum<<2); /*coding destination register + A ending(00)*/
				else{ /*no source operand of type register*/
					(*ins)[ic+(*memWords)]= (regNum<<2); /*coding destination register + A ending(00)*/
					(*memWords)++;
				}/*close else*/
				(*ins)[ic]+=(3<<2); /*coding the addressing methode of the destination operand -register*/
				(*i)+=2;
				return 1;
			}/*close if-register*/
		}/*close if-0-9*/
	}/*close if-r*/
	/*lable or struct oprand----------------*/
	for(; isalnum(sen[*i]);(*i)++)
		lable=1;
	if(lable && sen[*i]=='.'){
		if(sen[(*i)+1]=='1' || sen[(*i)+1] =='2'){ /*----struct---*/
			(*ins)[ic]+=(2<<4); /*coding the addressing methode of the destination operand -struct*/
			(*ins)[ic+ (*memWords) +1]= ((sen[(*i)+1]-'0')<<2); /*coding the number of field + A ending(00)*/
			(*memWords)+=2;
			(*i)+=2;
			return 1;
		}/*end if-struct*/	
	}/*end if -'.'*/
	if(lable){/*---label---*/
		(*ins)[ic]+=(1<<2); /*coding the addressing methode of the destination operand -label*/
		(*memWords)++;
		return 1;
	}/*close if- label*/
	return 0;

}/*end secondOperTwo*/
