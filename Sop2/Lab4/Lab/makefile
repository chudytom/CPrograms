CLIENT = udpClient
SERVER = udpServer

CLIENT.O = udpClient.o
SERVER.O = udpServer.o

CLIENT.C = udpClient.c
SERVER.C = udpServer.c

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
	gcc -Wall -o ${SERVER.O} -c ${SERVER.C} -lm -lrt
${CLIENT.O}: ${CLIENT.C} soplib.h
	gcc -Wall -o ${CLIENT.O} -c ${CLIENT.C} -lm -lrt
soplib.o: soplib.c soplib.h
	gcc -Wall -o soplib.o -c soplib.c

.PHONY: clean

clean: 
		-rm -f ${FILES}