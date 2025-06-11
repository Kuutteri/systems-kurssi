#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[]) {
    if (argc < 2) {
        //search not specified, return 1
        printf("my-grep: searchterm [file ...]\n");
        exit(1);
    }

    char *pattern = argv[1];
    char *buffer = NULL;
    size_t buf_size = 0;

    if (argc != 2) {
        int arg_index = 2;
        while (arg_index < argc) {
            FILE *file = fopen(argv[arg_index], "r");
            if (file == NULL) {
                printf("my-grep: cannot open file\n");
                exit(1);
            }
            while (getline(&buffer, &buf_size, file) != -1) {
                if (strstr(buffer, pattern) != NULL) {
                    printf("%s", buffer);
                }
            }
            fclose(file);
            arg_index++;
        }
    } else {
        // no files, read from stdin
        while (getline(&buffer, &buf_size, stdin) != -1) {
            if (strstr(buffer, pattern) != NULL) {
                printf("%s", buffer);
            }
        }
    }
    free(buffer);
    //Return 0 for successful
    return 0;
}
