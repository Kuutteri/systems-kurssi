#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("my-zip: file1 [file2 ...]\n");
        exit(1);
    }

    for (int i = 1; i < argc; i++) {
        FILE *fp = fopen(argv[i], "r");
        if (fp == NULL) {
            printf("my-zip: cannot open file\n");
            exit(1);
        }

        int count = 0;
        char current = 0, prev = 0;
        while (fread(&current, sizeof(char), 1, fp) == 1) {
            if (count == 0 || current == prev) {
                count++;
            } else {
                fwrite(&count, sizeof(int), 1, stdout);
                fwrite(&prev, sizeof(char), 1, stdout);
                count = 1;
            }
            prev = current;
        }
        if (count > 0) { // Write final run
            fwrite(&count, sizeof(int), 1, stdout);
            fwrite(&prev, sizeof(char), 1, stdout);
        }
        fclose(fp);
    }
    return 0;
}