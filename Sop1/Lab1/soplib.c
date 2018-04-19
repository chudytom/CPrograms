#define _GNU_SOURCE
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>

#include "soplib.h"

char *concat(const char *s1, const char *s2)
{
    const size_t len1 = strlen(s1);
    const size_t len2 = strlen(s2);

    char *result = malloc(len1 + len2 + 1);

    memcpy(result, s1, len1);
    memcpy(result + len1, s2, len2 + 1);
    return result;
}

char *convertToCapital(const char *text)
{
    size_t len = NELEMS(text);
    char *result = malloc(len * sizeof(char));
    strcpy(result, text);
    for (int i = 0; i < len; i++)
    {
        if (result[i] >= 'a' && result[i] <= 'z')
            result[i] = result[i] + ('A' - 'a');
    }
    return result;
}

void usage(char *pname)
{
    fprintf(stderr, "USAGE:%s ([-t x] -n Name) ... \n", pname);
    exit(EXIT_FAILURE);
}

