#include <stdio.h>
#include <stdlib.h>
#include "myShell.h"
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <stdbool.h>
#include <limits.h>
#include <signal.h>
#include <ctype.h>
#include <errno.h>

int main(int argc, char **argv)
{

    char *commandToRead;
    int state = 1;
    char fullPathName[FILENAME_MAX];

    // set the defaut enviroment variables paths
  
    getcwd(fullPathName, sizeof(fullPathName));
    //read from profile file and excute commands + open CIS3110_history to write to
    joinedCommand command;

    char *historyName = malloc(sizeof(char)*100);
    strcpy(historyName, fullPathName);
    strcat(historyName,"/.CIS3110_history");
    FILE *filePointer = fopen(historyName, "a+");

    while (state)
    {
        // get current working directory
        getcwd(fullPathName, sizeof(fullPathName));
        printf("%s> ", fullPathName);
        // read commands written from stdin
        commandToRead = readCommand(STDIN_MODE, NULL);
        // store history commands in .CIS3110_HISTORY
         writeHistory(commandToRead, filePointer);
        // use strtok to parse file into list of commands
        command = parser(commandToRead);
        // excecute the commands
        state = executor(command);
        free(commandToRead);
    }
     fclose(filePointer);
     free(historyName);

    return 0;
}
void readProfileFile(char *filename, joinedCommand command)
{
    FILE *fptr;
    char *contentOfProfile;
    fptr = fopen(filename, "r");

    // if file does not exit then create it
    if (fptr == NULL)
        fptr = fopen(filename, "w");

    // read and excute commands while its not end of file
    while (!feof(fptr))
    {
        contentOfProfile = readCommand(FILE_MODE, fptr);
        // store history commands in .CIS3110_HISTORY
        writeHistory(contentOfProfile, fptr);
        // use strtok to parse file into list of commands
        command = parser(contentOfProfile);
        // excecute the commands
        executor(command);
        free(contentOfProfile);
    }
    fclose(fptr);
}
/**
 * store history commands and write to .CIS3110_HISTORY
**/
void writeHistory(char *commandToWrite, FILE *file)
{
    // duplicate of the commandToWrite and malloc memory for the new string histArg
    char *histArg;
    if ((histArg = strdup(commandToWrite)) != NULL)
    {
        // write to file and update stream
        fprintf(file, "%s\n", histArg);
        fflush(file);
    }
    free(histArg);
}
/**
 * retrieve history commands and read it to .CIS3110_HISTORY
**/
void readHistory()
{
    char *fileName = malloc(sizeof(char) * SIZE);
    char history[SIZE];
    FILE *fp;
    int NumOfHistCmds = 0;

    // open file .CIS3110_history for reading
    strcpy(fileName, ".CIS3110_history");
    fp = fopen(fileName, "r+");

    while (fgets(history, SIZE, fp) != NULL)
    {
        printf(" %d  %s", NumOfHistCmds, history);
        NumOfHistCmds++;
    }

    // close file .CIS3110_history
    fclose(fp);
    free(fileName);
}

/**
 * retrieve history N commands and read it ffrom .CIS3110_HISTORY
**/
int HistoryEntryN(char *arg)
{
    // error checking
    char *lastString;
    if (strtol(arg, &lastString, 10) == 0)
        return 0;

    else
    {
        // convert string (lastString) to long integer (TotalCommands)
        int TotalCommands = strtol(arg, &lastString, 10);
        char *fileName = malloc(sizeof(char) * SIZE);
        char history[SIZE];
        FILE *fp;
        int NumOfHistCmds = 0;

        // open file .CIS3110_history for reading
        strcpy(fileName, ".CIS3110_history");
        fp = fopen(fileName, "r+");

        while ((fgets(history, SIZE, fp) != NULL) && (NumOfHistCmds <= TotalCommands))
        {
            printf(" %d  %s\n", NumOfHistCmds, history);
            NumOfHistCmds++;
        }

        // close file .CIS3110_history
        fclose(fp);
        free(fileName);
    }
    return 0;
}
// return command with spaces with multiple words
char *readCommand(int inputMode, FILE *file)
{
    char ch;
    int length = SIZE;
    int lastCharacter = 0;
    char *string = calloc(length, sizeof(char));

    // error checking for calloc
    if (string == NULL)
    {
        printf("memory allocation error\n");
        exit(1);
    }
    while (true)
    {

        // get line
        if (inputMode == STDIN_MODE)
        {
            scanf("%c", &ch);
        }
        else
        {
            ch = getc(file);
        }

        // read command inputed character by character
        switch (ch)
        {
        case EOF:
            string[lastCharacter] = '\0';
            return string;
        case '\n':
            string[lastCharacter] = '\0';
            return string;

        default:
            string[lastCharacter] = ch;
            break;
        }
        lastCharacter = lastCharacter + 1;

        //  add more space if user exceed length
        if (lastCharacter >= length)
        {
            length = length + SIZE;
            string = realloc(string, length);

            //error check
            if (string == NULL)
            {
                printf("memory allocation error\n");
                exit(1);
            }
        }
    }
}

// split line with spaces to commands
joinedCommand parser(char *line)
{
    char copy[500]; 
    strcpy(copy,line);
    char **processes;
    char *processA = NULL;
    int pipelocation = -1; // |
    joinedCommand command;
    partialCommand firstPartialCommand;
    partialCommand secondPartialCommand;

    //ensures that processes always have at least one char prevents crashing in case of NULL
    // processes will get tokenized later on if pipe is found
    processes = malloc(sizeof(char *));
    processes[0] = line;

    if (processes != NULL)
    {

        processA = processes[0];

        for (int i = 0; i < strlen(processA); i++)
        {
            //get location of | , > , <
            if ('|' == processA[i])
            {
                pipelocation = i;
            }
        }
        // if there is a pipe tokenize prossesses
        if (pipelocation != -1)
        {
            processes = tokenizeArray(line, "|");
        }

        // check last character for & percent and remove the & after indicating it has been found
        if ('&' == line[strlen(line) - 1])
        {
            command.backgroundExec = 1;
            line[strlen(line) - 1] = '\0';
        }
        else
        {
            command.backgroundExec = 0;
        }

        // Case Process A exists : This is always True
        // get rid of spaces in front and after input and output file + tokenize process A commands
        firstPartialCommand = ParseSingleCommand(processes[0]);
        if (firstPartialCommand.inputfile != NULL)
        {
            trim(firstPartialCommand.inputfile);
        }
        if (firstPartialCommand.outputfile != NULL)
        {
            trim(firstPartialCommand.outputfile);
        }    
        // copy the first PartialCommand structure to JoinedCommand structure process A related variables
        command.processATokens = firstPartialCommand.process;
        command.aOutputFile = firstPartialCommand.outputfile;
        command.aInputFile = firstPartialCommand.inputfile;
       // command.processBTokens = NULL;

        // Case Process B exists : Only when there is a Pipe
        // get rid of spaces in front and after input and output file + tokenize process B commands
        if (pipelocation != -1)
        {   
            //printf("SUP\n");
            secondPartialCommand = ParseSingleCommand(processes[1]);
            if (secondPartialCommand.inputfile != NULL)
            {
                trim(secondPartialCommand.inputfile);
            }
            if (secondPartialCommand.outputfile != NULL)
            {
                trim(secondPartialCommand.outputfile);
            }
        }
        // copy the first PartialCommand structure to JoinedCommand structure process B related variables
        command.bInputFile = secondPartialCommand.inputfile;
        command.bOutputFile = secondPartialCommand.outputfile;
        command.processBTokens = secondPartialCommand.process;
          
        char *ret1 = strstr(copy,"|");  // check if |  is there
        char *ret2 = strstr(copy,"<");  // check if <  is there
        char *ret3 = strstr(copy,">");  // check if >  is there

        if (ret1 == NULL && ret2 == NULL && ret3 == NULL ) {
            command.processBTokens = NULL;
        }

    }

    free(processes);
    return command;
}
partialCommand ParseSingleCommand(char *line)
{
    int inputlocation = -1;  // <
    int outputlocation = -1; // >
    char *string;
    partialCommand myPartialCommand;

    // initialize struct variables to NULL
    myPartialCommand.inputfile = NULL;
    myPartialCommand.outputfile = NULL;
    myPartialCommand.process = NULL;

    for (int i = 0; i < strlen(line); i++)
    {
        //get location of pipe (|) , output redirect(>) , input redirect(<)
        if ('>' == line[i])
        {
            outputlocation = i;
        }
        if ('<' == line[i])
        {
            inputlocation = i;
        }
    }
    // Case Both Input Redirection (<) and Output Redirection (>)
    if (inputlocation > -1 && outputlocation > -1)
    {
        int smallest = lowestNumber(inputlocation, outputlocation);
        // input file happens first
        if (smallest == 0)
        {
            string = strtok(line, "><");
            myPartialCommand.inputfile = strtok(NULL, "<>");
            myPartialCommand.outputfile = strtok(NULL, "<>");
            myPartialCommand.process = tokenizeArray(string, DELIM);
        }
        // output file comes first
        if (smallest == 1)
        {
            string = strtok(line, "><");
            myPartialCommand.outputfile = strtok(NULL, "<>");
            myPartialCommand.inputfile = strtok(NULL, "<>");
            myPartialCommand.process = tokenizeArray(string, DELIM);
        }
    }
    // Case Output Redirection Alone (>)
    if (inputlocation == -1 && outputlocation > -1)
    {
        string = strtok(line, "><");
        myPartialCommand.outputfile = strtok(NULL, "<>");
        myPartialCommand.process = tokenizeArray(string, DELIM);
    }
    // Case Input Redirection Alone (<)
    if (inputlocation > -1 && outputlocation == -1)
    {
        string = strtok(line, "><");
        myPartialCommand.inputfile = strtok(NULL, "<>");
        myPartialCommand.process = tokenizeArray(string, DELIM);
    }
    // Case Neither
    if (inputlocation == -1 && outputlocation == -1)
    {
        myPartialCommand.process = tokenizeArray(line, DELIM);
    }

    return myPartialCommand;
}
char **tokenizeArray(char *line, char *delim)
{
    char *string;
    int length = SIZE;
    int lastCharacter = 0;
    char **tokens = malloc(sizeof(char *) * length);

    if (tokens == NULL)
    {
        printf("memory allocation error\n");
        exit(1);
    }
    //split command line into list of arguments
    string = strtok(line, delim);

    // add extra space if needed in case user ran out of it,
    // since we dont know how much will be entered to command line
    while (string)
    {
        tokens[lastCharacter] = string;
        lastCharacter = lastCharacter + 1;

        if (lastCharacter >= length)
        {
            length = length + SIZE;
            line = realloc(line, length * sizeof(char *));

            if (line == NULL)
            {
                printf("memory allocation error\n");
                exit(1);
            }
        }
        string = strtok('\0', delim);
    }
    tokens[lastCharacter] = '\0';
    return tokens;
}
// get the minimum between two numbers
int lowestNumber(int a, int b)
{
    // if A is less than B then Input file happens first so set value to 0
    int smallest;
    if (a < b)
    {
        smallest = 0;
    }
    // if B is less than A then output file happens first so set value to 1
    else
    {
        smallest = 1;
    }
    return smallest;
}

// trim the beginning and the end of the line
int trim(char *line)
{
    int last_character = strlen(line) - 1;
    int count = 0;
    int countBack = 0;
    int i = 0;
    int j = 0;
    int x = 0;
    int lim = 0;
    char newLine[SIZE];

    // finds and removes leading space
    while (line[x] == ' ')
    {
        count++;
        x++;
    }
    // finds and remove trailing space
    lim = last_character;
    while (line[lim] == ' ')
    {
        countBack++;
        lim--;
    }
    // copy the string into a new one and return it
    for (i = x; i <= lim; i++)
    {
        newLine[j] = line[i];
        j++;
    }
    newLine[j] = '\0';

    strncpy(line, newLine, strlen(newLine));
    line[strlen(newLine)] = '\0';

    return 0;
}
// terminates the program
int myShellExit(joinedCommand command)
{
    printf("myShell terminating...\n");
    exit(0);
}
// builtin command cd
int myShellCd(joinedCommand command)
{
    chdir(command.processATokens[1]);
    return 1;
}
// Builtin command Echo of enviromental variables PATH, HISFILE,HOME
int myShellEcho(joinedCommand command)
{
    if (strcmp(command.processATokens[1], "$PATH") == 0)
    {
        printf("%s\n", getenv("PATH"));
    }
    if (strcmp(command.processATokens[1], "$HISTFILE") == 0)
    {
        printf("%s\n", getenv("HISTFILE"));
    }
    if (strcmp(command.processATokens[1], "$HOME") == 0)
    {
        printf("%s\n", getenv("HOME"));
    }
    return 1;
}
// helper function to get the size of a given array
int getArraySize(char **arr)
{
    int len = 0;
    while (arr[++len] != NULL)
    {
    }
    return len;
}
// Builtin command Export of enviromental variables PATH, HISFILE,HOME
int myShellExport(joinedCommand command)
{
    char *token = strsep(&command.processATokens[1], "=");
    char **locationArray = NULL;
    int locationLength = 0;
    char *FinalLocation = malloc(sizeof(char) * 1000);

    strcpy(FinalLocation, "\0");

    locationArray = tokenizeArray(command.processATokens[1], "$=");
    locationLength = getArraySize(locationArray);

    // loop through the length of the path
    for (int i = 0, j = 0; i < locationLength; i++, j++)
    {

        // if its written path (e.g /bin)
        if (locationArray[i][0] == '/')
        {
            char *typedLocation = locationArray[i];
            strcat(FinalLocation, typedLocation);
        }
        else
        {
            // else get the path of the enviroment variable
            char *envLocation = getenv(locationArray[i]);
            strcat(FinalLocation, envLocation);
        }
    }
    if (strcmp(token, "PATH") == 0)
    {
        setenv("PATH", FinalLocation, 1);
    }
    if (strcmp(token, "HISTFILE") == 0)
    {
        setenv("HISTFILE", FinalLocation, 1);
    }
    if (strcmp(token, "HOME") == 0)
    {
        setenv("HOME", FinalLocation, 1);
    }
    free(FinalLocation);
    free(locationArray);

    return 1;
}

// Function responsible for the built in command history
int myShellHistory(joinedCommand command)
{
    if (command.processATokens[1] != 0)
    {
        // clear the history if given flag -c
        if (strstr(command.processATokens[1], "-c") != NULL)
        {
            fclose(fopen(".CIS3110_history", "w"));
        }
        else
        {
            // if given a number after history, then print the last N commands
            HistoryEntryN(command.processATokens[1]);
        }
    }
    else
    {
        // retrieve history using readHistory function
        readHistory();
    }
    return 1;
}

int launcher(joinedCommand command)
{
    forkExec(command);
    return 1;
}
// handle signals
void handler(int sig, siginfo_t *signalInfo, void *hold)
{
    int state = 0;
    waitpid(-1,&state,WNOHANG);
    if (sig == SIGINT)
    {
        exit(0);
    }
}

int forkExec(joinedCommand command)
{
    

    // process identification to dublicate calling process to create child process
    pid_t childProcess = fork();
    int state = 0;
    struct sigaction sigact;
    sigact.sa_sigaction = handler;
    int LengthOfProcessId = 0;

    if (childProcess == 0)
    {
         
        // if there s a Process B then Pipe exists so Execute piper function to handle it
        if (command.processBTokens != NULL)
        {
            piper(command);
        }
        else
        {
            // otherwise we only have process A
            if (command.aInputFile != NULL)
            {
                freopen(command.aInputFile, "r", stdin);
            }
            if (command.aOutputFile != NULL)
            {
                freopen(command.aOutputFile, "w+", stdout);
            }
        }

        //execute the command
       
        state = execvp(command.processATokens[0], command.processATokens);
        if (state < 0)
        {
            printf("Command '%s' not found\n", command.processATokens[0]);
            exit(1);
        }

        // checks if fork had an error
    }
    else if (childProcess < 0)
    {
        perror("dublicating calling process");
        exit(1);
        // fork has been executed, parent process wait for command that the child is executing to finish running
    }
    else
    {
        // if there is an & then execute the command and return immediately, not
        // blocking until the command finishes using sigaction
        if (command.backgroundExec == 1)
        {
            sigaction(SIGCHLD, &sigact, NULL);
            int i = LengthOfProcessId;
            if (WIFEXITED(state) != 0)
            {
                printf("[%d]    %d\n", ++i, childProcess);
            }
        }
        else
        {
            waitpid(childProcess, &state, 0);
        }

    }

    return 0;
}

int piper(joinedCommand command)
{
    int fd[2];
    if (command.processBTokens != NULL)
        pipe(fd);
    int processAStatus;
    int processBStatus;
    pid_t childProcess = fork();

    // child process
    if (childProcess == 0)
    {
        processAStatus = redirectPipeIO(command.aInputFile, command.aOutputFile, command.processATokens, fd, 0, 1);
    }
    // parent process
    else if (childProcess > 0)
    {
        processBStatus = redirectPipeIO(command.bInputFile, command.bOutputFile, command.processBTokens, fd, 1, 0);

            close(fd[0]);
            close(fd[1]);
            waitpid(childProcess, &processAStatus, 0);
            waitpid(childProcess, &processBStatus, 0);
    }

    return 0;
}
int redirectPipeIO(char *inputFile, char *outputFile, char **processTokens, int fd[2], int num1, int num2)
{
    int state;

    // if num1 = 0 && num2 = 1: close stdin and it duplicates the output side of pipe to stdout then closes stdout
    // if num1 = 1 && num2 = 0: close stdout and it duplicates the input side of pipe to stdin then closes stdin
    close(fd[num1]);
    dup2(fd[num2], num2);
    close(fd[num2]);

    if (inputFile != NULL)
    {
        freopen(inputFile, "r", stdin);
    }
    if (outputFile != NULL)
    {
        freopen(outputFile, "w+", stdout);
    }

    state = execvp(processTokens[0], processTokens);
    return state;
}
// Function to execute builtin commands
int executor(joinedCommand command)
{
    // list of system calls
    char *execCalls[] = {"exit", "cd", "history"};
    int numOfCommands = sizeof(execCalls) / sizeof(char *);
    int (*callingFuncs[])(joinedCommand command) = {&myShellExit, &myShellCd, &myShellHistory};
    if (command.processATokens[0] == 0)
        return 1;

    int i = 0;
    while (i < numOfCommands)
    {
        if (strcmp(command.processATokens[0], execCalls[i]) == 0)
        {
            return (*callingFuncs[i])(command);
        }
        i++;
    }

    return launcher(command);
}
