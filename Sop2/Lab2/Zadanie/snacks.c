#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/wait.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <time.h>
#include <mqueue.h>

#define MENU_ITEMS "10 frytki-12 hamburger-5 cola-8 zapiekanka"
                

#define ERR(source) (fprintf(stderr,"%s:%d\n",__FILE__,__LINE__),\
                     perror(source),kill(0,SIGKILL),\
                                     exit(EXIT_FAILURE))


int main(int argc, char** argv) {

        // char longString[] = "This is a long string.\nIt has multiple lines of text in it.\nWe want to examine each of these lines separately.\nSo we will do that.";
        // char str[80] = "This is-www.tutorialspoint.com-website";
        char items[80] = MENU_ITEMS;
        const char s[2] = "-";
        char myStrings[4][80];
        int index = 0;

        char *line;
        line =  strtok(items, s);

        while(line != NULL)
        {
                printf("%s\n", line);
                strcpy(myStrings[index], line);
                line = strtok(NULL, s);
                index++;
        }
        printf("Other thing\n");
        for(int i = 0; i < index; i++)
        {
                printf("Next line");
                printf("%s\n", myStrings[i]);
        }

        char *price;
        char *snack;
        const char s2[2] = " ";
        
        for(int i = 0; i < index; i++)
        {
                price = strtok(myStrings[i], s2);
                printf("Cena: %s\n", price);
                snack = strtok(NULL, s2);
                printf("Snack: %s\n", snack);                
        }
        return EXIT_SUCCESS;
}