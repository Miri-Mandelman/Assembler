/*This file is the function 'getWord'.*/
#include <stdio.h>
#include <string.h>
#include "data.h"

/*The function reads a full word from a given file.
 Returns: c- the last readen char.
 Variables: fd -pointer to the file to read from.
	    word -string type,the word to put the readen word to.  */
char getWord(FILE *fd,char word[MAX_SENT]){
	char c;
	int i=0;
	
	c = fgetc(fd);
	memset(word,'\0',MAX_SENT-1);/*reset word*/
	while(c==' ' ||c=='\t' ){ /*reading character till mitting the start of the word (!(space/tab/enter)).*/
		
		if(!(feof(fd)))
			c=fgetc(fd);
	}/*close while loop*/

	while(c!=' ' && c!='\t' && c!='\n'){ /*reading character and adding them to 'word', 
						till mitting the end of the word (space/tab/enter).*/
		if(!(feof(fd))){

			word[i] = c;
			c=fgetc(fd);
			i++;
		}
		else
			return '\n';
	}/*close while loop.*/
	return c;

}/*end getWord.*/
