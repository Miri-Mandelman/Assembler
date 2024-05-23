asembler: macroCare.o getWord.o prog.o functions.o firstScan.o secondScan.o outFiles.o
	gcc -ansi -Wall -g macroCare.o getWord.o prog.o functions.o firstScan.o secondScan.o outFiles.o -o asembler -lm
macroCare.o: macroCare.c data.h
	gcc -c -ansi -Wall macroCare.c -o macroCare.o
prog.o: prog.c data.h
	gcc -c -ansi -Wall prog.c -o prog.o
getWord.o: getWord.c data.h
	gcc -c -ansi -Wall getWord.c -o getWord.o
functions.o: functions.c data.h
	gcc -c -ansi -Wall functions.c -o functions.o
firstScan.o: firstScan.c data.h
	gcc -c -ansi -Wall firstScan.c -o firstScan.o -lm
secondScan.o: secondScan.c data.h
	gcc -c -ansi -Wall secondScan.c -o secondScan.o
outFiles.o: outFiles.c data.h
	gcc -c -ansi -Wall outFiles.c -o outFiles.o

