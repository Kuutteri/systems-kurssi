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
            printf("wcat: cannot open file\n");
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
