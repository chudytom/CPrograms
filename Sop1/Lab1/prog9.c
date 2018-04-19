#include <stdio.h>
#include <stdlib.h>
#include <string.h>
// #include <unistd.h>
#include <getopt.h>

#include "soplib.h"

#define MAXL 20
#define STH 5

int main(int argc, char **argv)
{
	int x = 1;
	char c = 'm';
	while ((c = getopt(argc, argv, "t:n:")) != -1)
	{
		switch (c)
		{
		case 't':
			x = atoi(optarg);
			break;
		case 'n':
			for (int i = 0; i < x; i++)
				printf("Hello %s\n", optarg);
			break;
		default:
		printf("Default\n");
			usage(argv[0]);
		}
	}
	if(argc > optind) usage(argv[0]);
}
