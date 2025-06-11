/*

    Test file for wish shell for testing situations where
    run time takes longer than few milliseconds.

    

 */


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>


char filename[] = "/tmp/tempfileXXXXXX";
FILE *file;

void cleanup() {
    if (file != NULL) {
        fclose(file);
    }
    unlink(filename);
}

int main() {
 
    int i;
    char buffer[255];
    int fd;



    fd = mkstemp(filename);

    if (fd == -1) {
        printf("Error creating temporary file\n");
        return 1;
    }


    file = fopen(filename, "w+");

    if (file == NULL) {
        printf("Error opening file\n");
        close(fd);
        return 1;
    }

    atexit(cleanup);


    if (file == NULL) {
        printf("Error opening file\n");
        return 1;
    }

    for(i = 10; i > 0; i--) {
        fprintf(file, "This is a test%d\n",i);
        sleep(1);
    }

    rewind(file);
    
    while (fgets(buffer, sizeof(buffer), file) != NULL) {
        printf("%s", buffer);
    }

    return 0;
}
