#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ERR(source) (perror(source),\
					fprintf(stderr, "%s:%d\n", __FILE__, __LINE__),\
					exit(EXIT_FAILURE))
#define MAXL 20
#define STH 5

int main(int argc, char** argv) {
	for(int i=0; i< argc; i++)
	{
		printf("Argument: %s\n", argv[i]);
	}

	return EXIT_SUCCESS;
}
