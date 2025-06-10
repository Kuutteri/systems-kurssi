#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("wgrep: searchterm [file ...]\n");
        exit(1);
    }

    char *search_term = argv[1];
    char *line = NULL;
    size_t len = 0;

    if (argc == 2) { // Read from stdin
        while (getline(&line, &len, stdin) != -1) {
            if (strstr(line, search_term) != NULL) {
                printf("%s", line);
            }
        }
    } else { // Read from files
        for (int i = 2; i < argc; i++) {
            FILE *fp = fopen(argv[i], "r");
            if (fp == NULL) {
                printf("wgrep: cannot open file\n");
                exit(1);
            }
            while (getline(&line, &len, fp) != -1) {
                if (strstr(line, search_term) != NULL) {
                    printf("%s", line);
                }
            }
            fclose(fp);
        }
    }
    free(line);
    return 0;
}