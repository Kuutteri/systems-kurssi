/*
    AI USE STATEMENT
    Oskar Sankila
    000471121

    Github Copilot was used for fixing and verifying 
    the syntax in the code.

    Other AI tools were not used.

*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h> 
#include <fcntl.h>


void printError(const char*message) {
    write(STDERR_FILENO, message, strlen(message));
}

// Execute a non built-in command
void execute_command(char *userInput, char **path, char *outputFile) {
    pid_t pid = fork();
    if (pid == -1) {
        printError("Error: Fork failed\n");
        exit(EXIT_FAILURE);
    } else if (pid == 0) {
        if (outputFile != NULL) {
            //open the file for writing, create it if it doesnt exist, truncate it if it does
            //give permissions
            int fd = open(outputFile, O_WRONLY | O_CREAT | O_TRUNC, 0644);
            
            if (fd == -1) {
                printError("Error: Cannot open file\n");
                exit(EXIT_FAILURE);
            }
            dup2(fd, STDOUT_FILENO);
            dup2(fd, STDERR_FILENO);
            close(fd);
        }


        char *args[256];
        int argIndex = 0;

        //Tokenize userInput to get the command and its arguments
        //argIndex is used to keep track of the index of the args array
        char *token = strtok(userInput, " ");
        while (token != NULL && argIndex < 255) {
            args[argIndex++] = token;
            token = strtok(NULL, " ");
        }
        args[argIndex] = NULL;


        //Try to execute the command from each path
        for (int i = 0; path[i] != NULL; i++) {
            char command[256];
            snprintf(command, sizeof(command), "%s/%s", path[i], args[0]);
            if (access(command, X_OK) == 0) {
                execvp(command, args);
                printError("Error: Execution failed\n");
                exit(EXIT_FAILURE);
            }
        }
        fprintf(stderr, "Error: Command not found: %s\n", args[0]);

        exit(EXIT_FAILURE);
    } else {
        wait(NULL);
    }
}


int main(int argc, char *argv[]) {
    int loop = 1;
    char *userInput = NULL;
    size_t len = 0;
    char *path[256] = {"/bin","/usr/bin", NULL};
    FILE *input = stdin;
    int batchExists = 0;

    // Check for a batch file
    if (argc == 2) {
        input = fopen(argv[1], "r");

        if (input == NULL) {
            printError("Error: Cannot open file\n");
            exit(EXIT_FAILURE);
        }
        batchExists = 1;
    } else if (argc > 2) {
        fprintf(stderr, "Usage: %s [batch_file]\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    //start the shell
    while (loop == 1) {
        if (input == stdin) {
            printf("wish$ ");
        }

        if (getline(&userInput, &len, input) != -1) {
            
            userInput[strcspn(userInput, "\n")] = '\0';

            // Check for redirection
            char *outputFile = NULL;
            char *redirection = strchr(userInput, '>');
            if (redirection != NULL) {
                *redirection = '\0';
                redirection++;
                while (*redirection == ' ') redirection++;
                outputFile = redirection;
                if (strchr(outputFile, ' ') != NULL) {
                    fprintf(stderr, "Error: Multiple redirection operators or files\n");
                    continue;
                }
            }

            // Split userInput into separate commands by '&'
            char *commands[256];
            int commandCount = 0;
            char *command = strtok(userInput, "&");
            while (command != NULL && commandCount < 255) {
                commands[commandCount++] = command;
                command = strtok(NULL, "&");
            }
            commands[commandCount] = NULL;

            for (int i = 0; i < commandCount; i++) {
                // Trim leading and trailing spaces
                while (*commands[i] == ' ') commands[i]++;
                char *end = commands[i] + strlen(commands[i]) - 1;
                while (end > commands[i] && *end == ' ') end--;
                *(end + 1) = '\0';


                //Create a copy of userInput for tokenization
                //required because the original is sent to the execute_command function later on
                char *commandCopy = strdup(commands[i]);
                if (commandCopy == NULL) {
                    printError("Error: Memory allocation failed\n");
                    continue;
                }
                // Tokenize user input
                char *token = strtok(commandCopy, " ");
                if (token == NULL) {
                    continue;
                }


                // Handle built-in commands
                if (strcmp(token, "exit") == 0) {
                    if (strtok(NULL, " ") != NULL) {
                        fprintf(stderr, "exit: too many arguments\n");
                    } else {
                        free(commandCopy);
                        exit(0);
                    }
                } else if (strcmp(token, "cd") == 0) {
                    char *dir = strtok(NULL, " ");
                    if (dir == NULL || strtok(NULL, " ") != NULL) {
                        fprintf(stderr, "cd: wrong number of arguments\n");
                    } else if (chdir(dir) != 0) {
                        printError("Error: Cannot change directory\n");
                    }
                } else if (strcmp(token, "path") == 0) {
                    int i = 0;
                    while ((token = strtok(NULL, " ")) != NULL) {
                        path[i++] = token;
                    }
                    path[i] = NULL;
                } else if (strcmp(token, "curpath") == 0) {
                    char cwd[1024];
                    if (getcwd(cwd, sizeof(cwd)) != NULL) {
                        printf("%s$ \n", cwd);
                    } else {
                        printError("Error: Cannot get current directory\n");
                        exit(EXIT_FAILURE);
                    }
                //if command is not built-in, execute it
                } else {
                    execute_command(commands[i], path, outputFile);
                }
                free(commandCopy);
            }
        } else if (batchExists == 1) {
            loop = 0;
        } 
    }

    free(userInput);
    if (input != stdin) {
        fclose(input);
    }
    return 0;
}