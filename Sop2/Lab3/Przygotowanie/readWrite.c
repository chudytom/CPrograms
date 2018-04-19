#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "soplib.h"

char* readFromConsole();
int writeToFile();
int readAndWrite();

int main(void)
{
    int result = readAndWrite("testfile.txt");
    return result;
}

char* readFromConsole()
{
    int n = 128;
    char* data = malloc(n * sizeof(char));

    if (read(0, data, n) < 0)
        write(2, "An error occurred in the read.\n", 31);
    return data;
}

int writeToFile(char* filename, char* data)
{
    int filedesc = 1; //write
    filedesc = open(filename, O_CREAT | O_WRONLY | O_APPEND);

    if (filedesc < 0)
    {
        return -1;
    }

    char* textToWrite = data;
    // char* textToWrite = concat(data, "\n");

    printf("Text to write: %s", textToWrite);

    if (write(filedesc, textToWrite, strlen(textToWrite)) != strlen(textToWrite))
    {
        write(2, "There was an error writing to testfile.txt\n", 43);
        return -1;
    }

    return 0;
}

int readAndWrite(char* filename)
{
    char* dataFromConsole = readFromConsole();
    int result = writeToFile(filename, convertToCapital(dataFromConsole));
    return result;
}


