#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "soplib.h"

#define MAXL 20
#define STH 5

int main(int argc, char **argv)
{
	if(argc!=3) usage(argv[0]);
	int i,j=atoi(argv[2]);
	if(0==j) usage(argv[0]);
	for(i=0;i<j;i++)
		printf("Hello %s\n",argv[1]);
}
