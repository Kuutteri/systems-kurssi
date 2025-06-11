#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h> 
#include <fcntl.h>

// Standard error message as required by assignment
void printError() {
    char error_message[30] = "An error has occurred\n";
    write(STDERR_FILENO, error_message, strlen(error_message));
}

// Execute a non built-in command
void execute_command(char *userInput, char **path, char *outputFile) {
    pid_t pid = fork();
    if (pid == -1) {
        printError();
        exit(EXIT_FAILURE);
    } else if (pid == 0) {
        // Child process
        if (outputFile != NULL) {
            // Open the file for writing, create it if it doesn't exist, truncate it if it does
            int fd = open(outputFile, O_WRONLY | O_CREAT | O_TRUNC, 0644);
            
            if (fd == -1) {
                printError();
                exit(EXIT_FAILURE);
            }
            // Redirect both stdout and stderr to the file
            dup2(fd, STDOUT_FILENO);
            dup2(fd, STDERR_FILENO);
            close(fd);
        }

        char *args[256];
        int argIndex = 0;

        // Tokenize userInput to get the command and its arguments
        char *token = strtok(userInput, " \t");
        while (token != NULL && argIndex < 255) {
            args[argIndex++] = token;
            token = strtok(NULL, " \t");
        }
        args[argIndex] = NULL;

        if (args[0] == NULL) {
            exit(EXIT_FAILURE);
        }

        // Try to execute the command from each path
        for (int i = 0; path[i] != NULL; i++) {
            char command[256];
            snprintf(command, sizeof(command), "%s/%s", path[i], args[0]);
            if (access(command, X_OK) == 0) {
                // Use execv as required by assignment
                execv(command, args);
                printError();
                exit(EXIT_FAILURE);
            }
        }
        // Command not found in any path
        printError();
        exit(EXIT_FAILURE);
    } else {
        // Parent process - don't wait here for parallel execution
        // Waiting will be handled by the caller
    }
}

int main(int argc, char *argv[]) {
    char *userInput = NULL;
    size_t len = 0;
    char *path[256] = {"/bin", NULL}; // Initial path should only contain /bin
    FILE *input = stdin;
    int batchMode = 0;

    // Check for batch file argument
    if (argc == 2) {
        input = fopen(argv[1], "r");
        if (input == NULL) {
            printError();
            exit(1);
        }
        batchMode = 1;
    } else if (argc > 2) {
        printError();
        exit(1);
    }

    // Main shell loop
    while (1) {
        // Print prompt only in interactive mode
        if (!batchMode) {
            printf("wish> ");
            fflush(stdout);
        }

        // Read input line
        ssize_t read = getline(&userInput, &len, input);
        if (read == -1) {
            // EOF reached
            break;
        }

        // Remove newline character
        userInput[strcspn(userInput, "\n")] = '\0';

        // Skip empty lines
        if (strlen(userInput) == 0) {
            continue;
        }

        // Check for redirection
        char *outputFile = NULL;
        char *redirection = strchr(userInput, '>');
        if (redirection != NULL) {
            *redirection = '\0';
            redirection++;
            
            // Skip whitespace after >
            while (*redirection == ' ' || *redirection == '\t') {
                redirection++;
            }
            
            // Check for multiple redirection operators or files
            if (strlen(redirection) == 0 || strchr(redirection, '>') != NULL) {
                printError();
                continue;
            }
            
            // Extract filename (should be single word)
            char *filename = strtok(redirection, " \t");
            if (filename == NULL || strtok(NULL, " \t") != NULL) {
                printError();
                continue;
            }
            outputFile = filename;
        }

        // Split input into parallel commands by '&'
        char *commands[256];
        int commandCount = 0;
        char *inputCopy = strdup(userInput);
        char *command = strtok(inputCopy, "&");
        
        while (command != NULL && commandCount < 255) {
            // Trim leading and trailing whitespace
            while (*command == ' ' || *command == '\t') command++;
            char *end = command + strlen(command) - 1;
            while (end > command && (*end == ' ' || *end == '\t')) end--;
            *(end + 1) = '\0';
            
            if (strlen(command) > 0) {
                commands[commandCount++] = strdup(command);
            }
            command = strtok(NULL, "&");
        }
        free(inputCopy);

        // Array to store child process IDs for parallel execution
        pid_t children[256];
        int childCount = 0;

        // Process each command
        for (int i = 0; i < commandCount; i++) {
            // Tokenize command to get first word
            char *commandCopy = strdup(commands[i]);
            char *token = strtok(commandCopy, " \t");
            
            if (token == NULL) {
                free(commandCopy);
                continue;
            }

            // Handle built-in commands
            if (strcmp(token, "exit") == 0) {
                // exit should have no arguments
                if (strtok(NULL, " \t") != NULL) {
                    printError();
                } else {
                    // Clean up and exit
                    for (int j = 0; j < commandCount; j++) {
                        free(commands[j]);
                    }
                    free(commandCopy);
                    free(userInput);
                    if (input != stdin) fclose(input);
                    exit(0);
                }
            } else if (strcmp(token, "cd") == 0) {
                char *dir = strtok(NULL, " \t");
                if (dir == NULL || strtok(NULL, " \t") != NULL) {
                    printError();
                } else if (chdir(dir) != 0) {
                    printError();
                }
            } else if (strcmp(token, "path") == 0) {
                // Clear existing path
                for (int j = 0; path[j] != NULL; j++) {
                    if (j > 0) free(path[j]); // Don't free initial /bin
                }
                
                // Set new path
                int pathIndex = 0;
                while ((token = strtok(NULL, " \t")) != NULL && pathIndex < 255) {
                    path[pathIndex++] = strdup(token);
                }
                path[pathIndex] = NULL;
            } else {
                // External command - execute it
                pid_t pid = fork();
                if (pid == -1) {
                    printError();
                } else if (pid == 0) {
                    // Child process
                    if (outputFile != NULL) {
                        int fd = open(outputFile, O_WRONLY | O_CREAT | O_TRUNC, 0644);
                        if (fd == -1) {
                            printError();
                            exit(EXIT_FAILURE);
                        }
                        dup2(fd, STDOUT_FILENO);
                        dup2(fd, STDERR_FILENO);
                        close(fd);
                    }

                    // Parse arguments
                    char *args[256];
                    int argIndex = 0;
                    char *argToken = strtok(commands[i], " \t");
                    while (argToken != NULL && argIndex < 255) {
                        args[argIndex++] = argToken;
                        argToken = strtok(NULL, " \t");
                    }
                    args[argIndex] = NULL;

                    // Search for executable in path
                    for (int j = 0; path[j] != NULL; j++) {
                        char fullPath[256];
                        snprintf(fullPath, sizeof(fullPath), "%s/%s", path[j], args[0]);
                        if (access(fullPath, X_OK) == 0) {
                            execv(fullPath, args);
                            printError();
                            exit(EXIT_FAILURE);
                        }
                    }
                    // Command not found
                    printError();
                    exit(EXIT_FAILURE);
                } else {
                    // Parent process - store child PID
                    children[childCount++] = pid;
                }
            }
            free(commandCopy);
        }

        // Wait for all child processes to complete
        for (int i = 0; i < childCount; i++) {
            waitpid(children[i], NULL, 0);
        }

        // Clean up command strings
        for (int i = 0; i < commandCount; i++) {
            free(commands[i]);
        }
    }

    // Clean up
    free(userInput);
    if (input != stdin) {
        fclose(input);
    }
    exit(0);
}
