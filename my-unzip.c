#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("my-unzip: file1 [file2 ...]\n");
        exit(1);
    }

    int arg_index = 1;
    while (arg_index < argc) {
        FILE *file = fopen(argv[arg_index], "r");
        if (file == NULL) {
            printf("my-unzip: cannot open file\n");
            //unable to open the file, return 1
            exit(1);
        }

        int index;
        char character;
        //Reads all of the characters from the file
        while (fread(&index, sizeof(int), 1, file) == 1 && fread(&character, sizeof(char), 1, file) == 1) {
            int repeat = 0;
            while (repeat < index) {
                printf("%c", character);
                repeat++;
            }
        }
        fclose(file);
        arg_index++;
    }
    //Return 0 for successful
    return 0;
}
