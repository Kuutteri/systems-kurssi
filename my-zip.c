
#include <stdio.h>
#include <stdlib.h>

void compressFile(FILE *file, int *count, int *previousChar) {
    int currentChar = EOF; 

    //Read characters from the file until end of file and icrease count if char is the same as previous char
    while ((currentChar = fgetc(file)) != EOF) {
        if (currentChar == *previousChar) { 
            (*count)++;
        } else {
            //Write count and char to stdout
            if (*count > 0) {
                fwrite(count, sizeof(int), 1, stdout);
                fputc(*previousChar, stdout);
            }
            *previousChar = currentChar;
            *count = 1;
        }
    }
}

int main(int argc, char *argv[]) {
    //Check if at least one file is provided
    if (argc < 2) {
        printf("my-zip: file1 [file2 ...]\n");
        return 1;
    }

    int count = 0;
    int previousChar = EOF;

    for (int i = 1; i < argc; i++) {
        FILE *file = fopen(argv[i], "r");
        if (file == NULL) {
            //If the file cannot be opened print an error message and return 1
            printf("my-zip: cannot open file\n");
            return 1;
        }

        compressFile(file, &count, &previousChar);
        fclose(file);
    }

    //Write the remaining count and character to stdout
    if (count > 0) {
        fwrite(&count, sizeof(int), 1, stdout);
        fputc(previousChar, stdout);
    }

    return 0; //Return 0 -> ran successfully
}