TARGET = prog9
TARGET.O = prog9.o
TARGET.C = prog9.c

all: ${TARGET}
${TARGET}: ${TARGET.O} soplib.o
	gcc -Wall -o ${TARGET} ${TARGET.O} soplib.o -lm -lrt
${TARGET.O}: ${TARGET.C} soplib.h
	gcc -Wall -o ${TARGET.O} -c ${TARGET.C}
soplib.o: soplib.c soplib.h
	gcc -Wall -o soplib.o -c soplib.c