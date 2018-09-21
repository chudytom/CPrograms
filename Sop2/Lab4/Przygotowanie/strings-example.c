#include "string.h"
#include "stdio.h"

void stringsAndArraysExample();
void printWords();
void displayAllArray();
void displayString();
void evenOrOddNumberExample();
void alphabeticalSortExample();
void sortWordsByAlphabet();
void printAllWords();

int main(int argc, char **argv)
{
    alphabeticalSortExample();
}

void stringsAndArraysExample()
{
    printf("Hello beautiful world\n");
    printWords("Apple", "Orange");
    int numbers[6] = {1, 2, 3};
    int numbersCount = sizeof(numbers) / sizeof(int);
    displayAllArray(numbersCount, numbers);
    displayString("Kaczka");
}

void printWords(char *word1, char *word2)
{
    printf("This is word1: %s and this is word2 %s\n", word1, word2);
}

void displayAllArray(int elementsCount, int *array)
{
    for (int i = 0; i < elementsCount; i++)
    {
        printf("%d\n", array[i]);
    }
}

void displayString(char *subtitle)
{
    printf("Here is the string %s\n", subtitle);
}

void evenOrOddNumberExample()
{
    int number = 1000;
    while (number >= 0)
    {
        printf("Enter a positive number\n");
        scanf("%d", &number);
        if (number % 2 == 0)
            printf("%d is an even number\n", number);
        else
            printf("%d is an odd number\n", number);
    }
    printf("%d is a negative number. Program finished\n", number);
}

void alphabeticalSortExample()
{
    int wordsCount = 5;
    char *words[5] = {"Orange", "Apple", "Grape", "Peach", "Banana"};
    // char words[5][80];
    // char *word = "Banana";
    printf("Before sort:\n");
    printAllWords(wordsCount, words);

    printf("\n");
    sortWordsByAlphabet(wordsCount, words);
    printf("After sort:\n");
    printAllWords(wordsCount, words);
}

void sortWordsByAlphabet(int wordsCount, char **words)
{
    for(int currentIndex = 0; currentIndex < wordsCount - 1; currentIndex++ )
    {
        char *minValue = "zzzzzz";
        int minIndex = -1;
        for(int i=currentIndex; i< wordsCount; i++)
        {
            if(strcmp(words[i], minValue) <=0 )
            {
                minIndex = i;
                minValue = words[i];
            }
        }
        if(minIndex >= 0)
        {
            char *temp = words[currentIndex];
            words[currentIndex] = words[minIndex];
            words[minIndex] = temp;
        }
    }
}


void printAllWords(int wordsCount, char **words)
{
    for(int i=0; i< wordsCount; i++)
    {
        printf("%s\n", words[i]);
    }
}
