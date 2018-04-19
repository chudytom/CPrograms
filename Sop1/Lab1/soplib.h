#ifndef _SOPLIB_H
#define _SOPLIB_H

char* concat(const char* s1, const char* s2);
char* convertToCapital(const char* text);
void usage(char* pname);

#define NELEMS(x)  (sizeof(x) / sizeof((x)[0]));
#define ERR(source) (perror(source),\
					fprintf(stderr, "%s:%d\n", __FILE__, __LINE__),\
					exit(EXIT_FAILURE))

#endif