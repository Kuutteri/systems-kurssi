#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#define INITIAL_CAPACITY 100

// Function to free the allocated memory for lines
void free_lines(char **lines, int count) {
    for (int i = 0; i < count; i++) {
        free(lines[i]);
    }
    free(lines);
}

// Function to check if two files are the same based on their inode numbers
int same_file(const char *path1, const char *path2) {
    struct stat stat1, stat2;
    return (stat(path1, &stat1) == 0 && 
            stat(path2, &stat2) == 0 &&
            stat1.st_ino == stat2.st_ino);
}

// Main function to read lines from input and write reversed version to output
int main(int argc, char *argv[]) {
    FILE *input = stdin;
    FILE *output = stdout;
    char *line = NULL;
    size_t len = 0;
    ssize_t read;
    int line_count = 0, capacity = INITIAL_CAPACITY;

    // Check for correct number of arguments
    if (argc > 3) {
        fprintf(stderr, "usage: reverse <input> <output>\n");
        exit(1);
    }

    // If an input file is specified, open it. Otherwise, use stdin
    if (argc >= 2) {
        input = fopen(argv[1], "r");
        if (!input) {
            fprintf(stderr, "reverse: cannot open file '%s'\n", argv[1]);
            exit(1);
        }
    }

    // If an output file is specified, open it. Otherwise, use stdout
    if (argc == 3) {
        if (same_file(argv[1], argv[2])) {
            fprintf(stderr, "reverse: input and output file must differ\n");
            exit(1);
        }
        output = fopen(argv[2], "w");
        if (!output) {
            fprintf(stderr, "error: cannot open file '%s'\n", argv[2]);
            exit(1);
        }
    }

    // Allocate initial memory for lines
    char **lines = malloc(capacity * sizeof(char *));
    if (!lines) {
        fprintf(stderr, "malloc failed\n");
        exit(1);
    }

    // Read lines from input until EOF
    while ((read = getline(&line, &len, input)) != -1) {
        if (line_count >= capacity) {
            capacity *= 2;
            char **tmp = realloc(lines, capacity * sizeof(char *));
            if (!tmp) {
                fprintf(stderr, "malloc failed\n");
                free_lines(lines, line_count);
                free(line);
                exit(1);
            }
            lines = tmp;
        }
        lines[line_count++] = strdup(line);
        if (!lines[line_count - 1]) {
            fprintf(stderr, "malloc failed\n");
            free_lines(lines, line_count - 1);
            free(line);
            exit(1);
        }
    }

    // reversed order to output
    for (int i = line_count - 1; i >= 0; i--) {
        fprintf(output, "%s", lines[i]);
        free(lines[i]);
    }

    // Free allocated memory
    free(lines);
    free(line);
    if (input != stdin) fclose(input);
    if (output != stdout) fclose(output);

    return 0;
}
