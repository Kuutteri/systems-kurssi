/*
    AI USE STATEMENT
    Joona Kuutniemi
    001119893

    I used Geok3 was used for fixing issues I was having.

    Other AI tools were not used.

*/
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
    if (argc < 2) {
        return 0;
    }

    int index = 1;
    while (index < argc) {
        FILE *fp = fopen(argv[index], "r");
        if (fp == NULL) {
            fprintf(stderr, "wcat: unable to access file\n");
            return 1;
        }

        char line[1024];
        while (fgets(line, sizeof(line), fp) != NULL) {
            fputs(line, stdout);
        }
        fclose(fp);
        index++;
    }
    
    return 0;
}
