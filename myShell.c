#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#define HISTORY_FILENAME ".myShell.hist"
#define HISTORY_COUNT 100

void getCurrentDirectory(char *);
void myShellInfiniteRun(void);
char * readCommandLine(void);
int clParserMain(char*, char**, char**, char *hist[], int, size_t);
void getCommandArgumentList(char* str, char** parsedLine);
int checkPipe(char* str, char** pipedParts);
int checkRedirect(char* str, char** pipedParts);
void startNormalProcess(char** parsedNormalLine);
void startPipedProcess(char** parsedNormalLine, char** parsedPipedLine);
int history(char *hist[], int current);
int clearHistory(char *hist[]);
int builtInFunctions(char** parsedNormalLine, char *hist[], int current, size_t histSize);
void usage();
char *strstrip(char *s);
void saveHistory(char *hist[], int current, size_t histSize);
void startRedirectProcess(char** parsedNormalLine, char** parsedPipedLine);

int main(int argc, char **argv) {

    myShellInfiniteRun();

    return 0;
}

char *strstrip(char *s) {
    size_t size;
    char *end;

    size = strlen(s);

    if (!size)
        return s;

    end = s + size - 1;
    while (end >= s && isspace(*end))
        end--;
    *(end + 1) = '\0';

    while (*s && isspace(*s))
        s++;

    return s;
}

void getCurrentDirectory(char * cdir) {
    char cwd[2048];
    getcwd(cwd, sizeof (cwd));
    cdir = cwd;

}

int builtInFunctions(char** parsedNormalLine, char *hist[], int current, size_t histSize) {

    if (parsedNormalLine[0] == NULL) {

        return 0;
    }

    int implementedFunctionsNum = 4, i, functionSelector = 0;
    char* functionList[implementedFunctionsNum];

    functionList[0] = "exit";
    functionList[1] = "cd";
    functionList[2] = "help";
    functionList[3] = "history";
    for (i = 0; i < implementedFunctionsNum; i++) {
        if (strcmp(parsedNormalLine[0], functionList[i]) == 0) {
            functionSelector = i + 1;
            break;
        }
    }

    switch (functionSelector) {
        case 1:
            //saveHistory(hist, current, histSize);
            exit(0);
        case 2:
            chdir(parsedNormalLine[1]);
            return 1;
        case 3:
            usage();
            return 1;
        case 4:
            history(hist, current);
            return 1;
        default:
            break;
    }

    return 0;
}

void usage() {
    printf("This is a simple shell. Just type commands and enjoy.\n");
}

void saveHistory(char *hist[], int current, size_t histSize) {
    FILE *fp;

    fp = fopen(HISTORY_FILENAME, "w+");

    fwrite(*hist, sizeof (char), histSize, fp);
    //fprintf(fp, "This is testing ...\n");
    fclose(fp);
}

// Function to take input

char *readCommandLine(void) {

    char *line = NULL;
    size_t size;
    if (getline(&line, &size, stdin) == -1) {
        exit(1);
    }
    line[strcspn(line, "\r\n")] = 0; // Removing \n
    return strstrip(line);
    //return line;

}

int clParserMain(char* str, char** parsedNormalLine, char** parsedPipedLine, char *hist[], int current, size_t histSize) {
    //printf("I am inside CL Tokenizer \n");
    char* pipedParts[2];
    char* redirectParts[2];
    int hasPipe = 0;
    int hasRedirect = 0;
    hasPipe = checkPipe(str, pipedParts);
    hasRedirect = checkRedirect(str, redirectParts);
    //printf("The value of hasPipe is %d\n", hasPipe);
    if (hasRedirect) {
        getCommandArgumentList(redirectParts[0], parsedNormalLine);
        getCommandArgumentList(redirectParts[1], parsedPipedLine);
        return 3;
    }

    if (hasPipe) {
        getCommandArgumentList(pipedParts[0], parsedNormalLine);
        getCommandArgumentList(pipedParts[1], parsedPipedLine);

    } else {

        getCommandArgumentList(str, parsedNormalLine);
    }

    if (builtInFunctions(parsedNormalLine, hist, current, histSize))
        return 0;
    else
        return 1 + hasPipe;
}



// check whether a pipe exists or not

int checkPipe(char* str, char** pipedParts) {
    int i;
    for (i = 0; i < 2; i++) {
        pipedParts[i] = strsep(&str, "|");
        if (pipedParts[i] == NULL)
            break;
    }

    if (pipedParts[1] == NULL)
        return 0;
    else {
        return 1;
    }
}

int checkRedirect(char* str, char** pipedParts) {
    int i;
    for (i = 0; i < 2; i++) {
        pipedParts[i] = strsep(&str, ">");
        if (pipedParts[i] == NULL)
            break;
    }

    if (pipedParts[1] == NULL)
        return 0;
    else {
        return 1;
    }
}

// function for parsing command words

void getCommandArgumentList(char* str, char** parsedLine) {

    int i;
    //printf("%s is the string to parse  \n", str);
    for (i = 0; i < 100; i++) {
        parsedLine[i] = strsep(&str, " ");

        if (parsedLine[i] == NULL)
            break;
        if (strlen(parsedLine[i]) == 0)
            i--;
    }
}

void startNormalProcess(char** parsedNormalLine) {
    pid_t pid = fork();

    if (pid == -1) {
        perror("myShell: "); // ERROR FORKING
        return;
    } else if (pid == 0) {
        if (execvp(parsedNormalLine[0], parsedNormalLine) == -1) {
            perror("myShell: ");
        }
        exit(0);
    } else {
        wait(NULL);
        return;
    }
}

void startRedirectProcess(char** parsedNormalLine, char** parsedPipedLine) {
    FILE * fd = fopen(parsedPipedLine[0], "w");
    int fd1 = fileno(fd);
    pid_t pid = fork();
    if (pid == -1) {
        perror("myShell: "); // ERROR FORKING
        return;
    } else if (pid == 0) {

        //dup2(fd1,1);
        //testfunc();
        //exit (0);
        //int fd1 = creat(output , 0644) ;
        dup2(fd1, STDOUT_FILENO);
        close(fd1);
        if (execvp(parsedNormalLine[0], parsedNormalLine) == -1) {
            perror("myShell: ");
        }
        exit(0);
    } else {
        wait(NULL);
        // fd.close();
        return;
    }
}

void startPipedProcess(char** parsedNormalLine, char** parsedPipedLine) {
    int pipefd[2];
    pid_t p1, p2;

    if (pipe(pipefd) == -1) {
        perror("myShell: ");
        return;
    }
    p1 = fork();
    if (p1 == -1) {
        perror("myShell: ");
        return;
    }

    if (p1 == 0) {
        // Child 1 executing..
        // It only needs to write at the write end
        dup2(pipefd[0], STDIN_FILENO);

        close(pipefd[0]);
        close(pipefd[1]);

        //printf("The command to execute at the child 1 is  %s\n",parsedNormalLine[0]);
        //
        if (execvp(parsedPipedLine[0], parsedPipedLine) == -1) {

            perror("myShell: ");
            exit(0);
        }
    } else {
        // Parent executing
        p2 = fork();

        if (p2 == -1) {
            perror("myShell: ");
            return;
        }

        if (p2 == 0) {
            dup2(pipefd[1], STDOUT_FILENO);
            close(pipefd[1]);
            close(pipefd[0]);
            //printf("The command to execute at the child 2 is  %s\n",parsedPipedLine[0]);
            if (execvp(parsedNormalLine[0], parsedNormalLine) == -1) {
                perror("myShell: ");
                exit(0);
            }
        } else {
            // parent executing, waiting for two children
            close(pipefd[1]);
            close(pipefd[0]);
            while (wait(NULL) > 0);
            //wait(NULL);
            // waitpid(0,NULL,0);
        }
    }
    //-------------------------------------------------------------------------------------------------------------------------------------------------- */


}

int history(char *hist[], int current) {
    int i = current;
    int hist_num = 1;

    do {
        if (hist[i]) {
            printf("%4d  %s\n", hist_num, hist[i]);
            hist_num++;
        }

        i = (i + 1) % HISTORY_COUNT;

    } while (i != current);

    return 0;
}

int clearHistory(char *hist[]) {
    int i;

    for (i = 0; i < HISTORY_COUNT; i++) {
        free(hist[i]);
        hist[i] = NULL;
    }

    return 0;
}

void myShellInfiniteRun(void) {

    //History
    char *hist[HISTORY_COUNT];
    size_t histSize = sizeof (hist);
    int i, current = 0;
    // Load From File
    for (i = 0; i < HISTORY_COUNT; i++)
        hist[i] = NULL;


    // Trim the ending spaces remaining  
    char *commandLine;
    //char **commands;
    int parseStatus;
    char *commands[100];
    char *pipeCommands[100];

    //char currdir[2048];

    while (1) {
        // "myShell>" looks better than the "directory name>" :)
        //getcwd(currdir, sizeof(currdir)); 
        //printf("%s> ",currdir);   
        printf("myShell>");

        free(hist[current]);

        commandLine = readCommandLine();
        hist[current] = strdup(commandLine);
        current = (current + 1) % HISTORY_COUNT;
        //printf("%s\n", commandLine);
        parseStatus = clParserMain(commandLine, commands, pipeCommands, hist, current, histSize);
        //printf("%s\n", commands[1]);
        //printf("The status is %d\n", status);

        if (parseStatus == 1)
            startNormalProcess(commands);

        if (parseStatus == 2) {
            startPipedProcess(commands, pipeCommands);
        }
        if (parseStatus == 3) {
            startRedirectProcess(commands, pipeCommands);
        }



        //free(commandLine);
    }

}
