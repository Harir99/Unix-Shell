#ifndef MY_1SHELL_H
#define MY_SHELL_H
#define SIZE 1000
#define DELIM " \t\r\n\a"
#define HIST_MAX 10
#define FILE_MODE 1
#define STDIN_MODE 0
#include <signal.h>


typedef struct joinedCommand {
    char** processATokens;
    char** processBTokens;
    char* aOutputFile;
    char* bInputFile;
    char* aInputFile;
    char* bOutputFile;
    int backgroundExec;
} joinedCommand;

typedef struct partialCommand {
    char** process;
    char* outputfile;
    char* inputfile;
} partialCommand;

int getArraySize(char** arr);
void readProfileFile(char* filename,joinedCommand command);

char * readCommand(int inputMode,FILE* file);
joinedCommand parser(char *line);
int executor(joinedCommand command);
int launcher(joinedCommand command);

int lowestNumber(int a, int b);
int trim ( char *line );
char** tokenizeArray(char* line,char* delim);

void handler(int signal, siginfo_t* signalInfo, void *hold);
int forkExec(joinedCommand command);
void sigquit(int signo);
void exit();

int myShellCd(joinedCommand command);
int myShellEcho(joinedCommand command);
int myShellExport(joinedCommand command);

int myShellHistory(joinedCommand command);
void writeHistory(char* commandToWrite,FILE*file);
void historyPrint(int NumOfPrints,joinedCommand command);
partialCommand ParseSingleCommand(char* line);
void readHistory();
int HistoryEntryN(char * arg);
int piper(joinedCommand command);
int redirectPipeIO(char* inputFile, char* outputFile, char**processTokens,int fd[2], int num1, int num2);
#endif 


