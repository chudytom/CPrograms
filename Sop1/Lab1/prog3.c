#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ERR(source) (perror(source),\
					fprintf(stderr, "%s:%d\n", __FILE__, __LINE__),\
					exit(EXIT_FAILURE))
#define MAXL 20
#define STH 5

int main(int argc, char** argv) {
	char name[22];
	while(fgets(name, MAXL + 1, stdin)!=NULL)
		printf("The line: %s\n", name);

	// printf("Put your name\n");
	// scanf("%21s", name);
	// if(strlen(name) > 20 ) ERR("Name was to long");
	// printf("Your name is: %s\n", name);	

	return EXIT_SUCCESS;
}
