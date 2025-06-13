#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>

int main(int argc, char *argv[]) {
    char *userInput = NULL;
    FILE *input = stdin;
    size_t len = 0;
    ssize_t read;
    int batchMode = 0;
    char *path[256] = {"/bin", NULL};

    // Check arguments
    if (argc > 2) {
        write(STDERR_FILENO, "An error has occurred\n", 22); // Standard error message
        exit(1);
    }
    
    // Check for batch file
    if (argc == 2) {
        input = fopen(argv[1], "r");
        if (input == NULL) {
            write(STDERR_FILENO, "An error has occurred\n", 22); // Standard error message
            exit(1);
        }
        batchMode = 1;
    }

    // Main shell loop
    while (1) {
        if (!batchMode) {
            printf("wish> ");
            fflush(stdout);
        }

        // Read input line
        read = getline(&userInput, &len, input);
        if (read == -1) {
            break;
        }

        // Remove newline and trim whitespace
        userInput[strcspn(userInput, "\n")] = '\0';
        char *trimmed = userInput;
        while (*trimmed == ' ' || *trimmed == '\t') trimmed++;
        if (strlen(trimmed) == 0) {
            continue;
        }

        // Split input into parallel commands
        char *commands[256];
        int commandCount = 0;
        char *inputCopy = strdup(userInput);
        if (!inputCopy) {
            write(STDERR_FILENO, "An error has occurred\n", 22); // Standard error message
            continue;
        }
        char *command = strtok(inputCopy, "&");
        
        while (command != NULL && commandCount < 255) {
            // Trim whitespace
            while (*command == ' ' || *command == '\t') command++;
            char *end = command + strlen(command) - 1;
            while (end > command && (*end == ' ' || *end == '\t')) end--;
            *(end + 1) = '\0';
            
            if (strlen(command) > 0) {
                commands[commandCount++] = strdup(command);
                if (!commands[commandCount-1]) {
                    write(STDERR_FILENO, "An error has occurred\n", 22); // Standard error message
                    break;
                }
            }
            command = strtok(NULL, "&");
        }
        free(inputCopy);

        // Array to store child PIDs
        pid_t children[256];
        int childCount = 0;

        for (int i = 0; i < commandCount; i++) {
            // Check for redirection in this command
            char *outputFile = NULL;
            char *cmdCopy = strdup(commands[i]);
            if (!cmdCopy) {
                write(STDERR_FILENO, "An error has occurred\n", 22); // Standard error message
                continue;
            }
            char *redirection = strchr(cmdCopy, '>');
            if (redirection != NULL) {
                *redirection = '\0';
                redirection++;
                
                // Skip whitespace after >
                while (*redirection == ' ' || *redirection == '\t') {
                    redirection++;
                }
                
                // Check for invalid redirection
                if (strlen(redirection) == 0 || strchr(redirection, '>') != NULL) {
                    write(STDERR_FILENO, "An error has occurred\n", 22); // Standard error message
                    free(cmdCopy);
                    continue;
                }
                
                // Extract filename
                char *filename = strtok(redirection, " \t");
                if (filename == NULL || strtok(NULL, " \t") != NULL) {
                    write(STDERR_FILENO, "An error has occurred\n", 22); // Standard error message
                    free(cmdCopy);
                    continue;
                }
                outputFile = filename;
            }

            // Tokenize command
            char *commandCopy = strdup(cmdCopy);
            if (!commandCopy) {
                write(STDERR_FILENO, "An error has occurred\n", 22); // Standard error message
                free(cmdCopy);
                continue;
            }
            char *token = strtok(commandCopy, " \t");
            
            if (token == NULL) {
                if (outputFile != NULL) {
                    write(STDERR_FILENO, "An error has occurred\n", 22); // Standard error message
                }
                free(commandCopy);
                free(cmdCopy);
                continue;
            }

            // Handle built-in commands
            if (strcmp(token, "exit") == 0) {
                if (strtok(NULL, " \t") != NULL) {
                    write(STDERR_FILENO, "An error has occurred\n", 22); // Standard error message
                } else {
                    for (int j = 0; j < commandCount; j++) {
                        free(commands[j]);
                    }
                    free(commandCopy);
                    free(cmdCopy);
                    free(userInput);
                    if (input != stdin) fclose(input);
                    for (int j = 1; path[j]; j++) {
                        free(path[j]);
                    }
                    exit(0);
                }
            } else if (strcmp(token, "cd") == 0) {
                char *dir = strtok(NULL, " \t");
                if (dir == NULL || strtok(NULL, " \t") != NULL) {
                    write(STDERR_FILENO, "An error has occurred\n", 22); // Standard error message
                } else if (chdir(dir) != 0) {
                    write(STDERR_FILENO, "An error has occurred\n", 22); // Standard error message
                }
            } else if (strcmp(token, "path") == 0) {
                // Clear existing path (skip "/bin")
                for (int j = 1; path[j]; j++) {
                    free(path[j]);
                    path[j] = NULL;
                }
                
                // Set new path
                int pathIndex = 0;
                while ((token = strtok(NULL, " \t")) != NULL && pathIndex < 255) {
                    path[pathIndex++] = strdup(token);
                    if (!path[pathIndex-1]) {
                        write(STDERR_FILENO, "An error has occurred\n", 22); // Standard error message
                        break;
                    }
                }
                path[pathIndex] = NULL;
            } else {
                // External command
                pid_t pid = fork();
                if (pid == -1) {
                    write(STDERR_FILENO, "An error has occurred\n", 22); // Standard error message
                } else if (pid == 0) {
                    if (outputFile) {
                        int fd = open(outputFile, O_WRONLY | O_CREAT | O_TRUNC, 0644);
                        if (fd == -1) {
                            write(STDERR_FILENO, "An error has occurred\n", 22); // Standard error message
                            exit(1);
                        }
                        dup2(fd, STDOUT_FILENO);
                        dup2(fd, STDERR_FILENO);
                        close(fd);
                    }

                    // Parse arguments
                    char *args[256];
                    int argIndex = 0;
                    char *argToken = strtok(cmdCopy, " \t");
                    while (argToken != NULL && argIndex < 255) {
                        args[argIndex++] = argToken;
                        argToken = strtok(NULL, " \t");
                    }
                    args[argIndex] = NULL;

                    // Search for executable
                    for (int j = 0; path[j]; j++) {
                        char fullPath[256];
                        snprintf(fullPath, sizeof(fullPath), "%s/%s", path[j], args[0]);
                        if (access(fullPath, X_OK) == 0) {
                            execv(fullPath, args);
                            write(STDERR_FILENO, "An error has occurred\n", 22); // Standard error message
                            exit(1);
                        }
                    }
                    write(STDERR_FILENO, "An error has occurred\n", 22); // Standard error message
                    exit(1);
                } else {
                    children[childCount++] = pid;
                }
            }
            free(commandCopy);
            free(cmdCopy);
        }

        // Wait for children
        for (int i = 0; i < childCount; i++) {
            waitpid(children[i], NULL, 0);
        }

        // Clean up commands
        for (int i = 0; i < commandCount; i++) {
            free(commands[i]);
        }
    }

    // Clean up
    free(userInput);
    if (input != stdin) {
        fclose(input);
    }
    // Free path entries (skip "/bin")
    for (int j = 1; path[j]; j++) {
        free(path[j]);
    }
    exit(0);
}
