#include <stdio.h>
#include <stdlib.h>

void packFile(FILE *input, int *length, int *prevChar) {
    int nextChar = EOF;

    while ((nextChar = fgetc(input)) != EOF) {
        if (nextChar == *prevChar) {
            (*length)++;
        } else {
            if (*length > 0) {
                fwrite(length, sizeof(int), 1, stdout);
                fputc(*prevChar, stdout);
            }
            *prevChar = nextChar;
            *length = 1;
        }
    }
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("my-zip: file1 [file2 ...]\n");
        return 1;
    }

    int length = 0;
    int prevChar = EOF;
    int argIndex = 1;
    while (argIndex < argc) {
        FILE *input = fopen(argv[argIndex], "r");
        if (input == NULL) {
            printf("my-zip: cannot open file\n");
            return 1;
        }
        packFile(input, &length, &prevChar);
        fclose(input);
        argIndex++;
    }

    if (length > 0) {
        fwrite(&length, sizeof(int), 1, stdout);
        fputc(prevChar, stdout);
    }
    return 0;
}