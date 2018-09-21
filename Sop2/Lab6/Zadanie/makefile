CLIENT = client
SERVER = server

CLIENT.O = client.o
SERVER.O = server.o

CLIENT.C = client.c
SERVER.C = server.c

TARGET = ${SERVER} ${CLIENT}
FILES = ${TARGET} ${SERVER.O} ${CLIENT.O} soplib.o
FILES_SERVER = ${SERVER.O} soplib.o
FILES_CLIENT = ${CLIENT.O} soplib.o

all: ${TARGET}
${SERVER}: ${FILES_SERVER}
	gcc -Wall -o ${SERVER} ${FILES_SERVER} -lm -lrt -pthread
${CLIENT}: ${FILES_CLIENT}
	gcc -Wall -o ${CLIENT} ${FILES_CLIENT} -lm -lrt -pthread
${SERVER.O}: ${SERVER.C} soplib.h
	gcc -Wall -o ${SERVER.O} -c ${SERVER.C} -lm -lrt -pthread
${CLIENT.O}: ${CLIENT.C} soplib.h
	gcc -Wall -o ${CLIENT.O} -c ${CLIENT.C} -lm -lrt -pthread
soplib.o: soplib.c soplib.h
	gcc -Wall -o soplib.o -c soplib.c

.PHONY: clean

clean: 
		-rm -f ${FILES}