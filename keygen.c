#include <stdio.h>
#include <stdlib.h>

int main (int argc, char* argv[])
{
    if (argc < 2) { 
        fprintf(stderr, "Program requires 1 argument (key length)\n"); exit(1); 
    } // Check usage & args

    int keyLength = atoi(argv[1]);
    char* key = malloc(sizeof(char) * (keyLength + 1));
    int asciiVal;
    time_t t;

    srand((unsigned) time(&t));

    int i;
    for (i = 0; i < keyLength; ++i) {

        asciiVal = rand() % 27;

        if (asciiVal == 26)
			asciiVal = 32;
		else
			asciiVal += 65;

        key[i] = asciiVal;    
    }

    key[i] = '\n';

    printf("%s", key); fflush(stdout);

    return 0;
}