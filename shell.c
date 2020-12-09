#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>

int isspace(int argument);

void printPrompt()
{
    char *user;
    char buf[100];
    getcwd(buf, 100);
    user = getlogin();
    printf("%s@cs345sh:%s$ ", user, buf);
}

//Code from: https://www.geeksforgeeks.org/
char *trimSpaces(char *str)
{
    char *end;
    while (isspace((unsigned char)*str))
        str++;
    if (*str == 0)
        return str;
    end = str + strlen(str) - 1;
    while (end > str && isspace((unsigned char)*end))
        end--;
    end[1] = '\0';
    return str;
}

void handlePause(int sig)
{
    printf("Proccess paused! Press ctrl + Q to continue the excecution..\n");
}

void handleBegin(int sig)
{
    printf("Excecution continues..\n");
}

char *getEnteredInstruction(void)
{
    int ctr;
    int ch;
    char *ptr = (char *)malloc(250 * sizeof(char));
    if (ptr == 0)
        fprintf(stderr, "Memory allocation failed");
    char *instruction = ptr;
    while (((ch = getchar()) != '\n') && (ch != EOF))
    {
        /*if (ch == 19) // ctrl + s
            signal(SIGTSTP, handlePause);
        if (ch == 17) // ctrl + q
            signal(SIGCONT, handleBegin);*/
        *ptr = ch;
        ptr++;
    }
    *ptr = '\0';
    instruction = trimSpaces(instruction);
    return instruction;
}

char **parseInstruction(char *line)
{
    int index = 0;
    char *token;
    char **tokenArray = (char **)malloc(100 * sizeof(char *));
    if (tokenArray == 0)
        fprintf(stderr, "Memory allocation failed");
    token = strtok(line, " \t\r\n\a|||");
    while (token != NULL)
    {
        tokenArray[index] = token;
        index++;
        token = strtok(NULL, " \t\r\n\a|||");
    }
    tokenArray[index] = NULL;
    return tokenArray;
}

int forkAndExecute(char **arguments)
{
    pid_t pid, wpid;
    int childStatus;
    pid = fork();
    if (pid == 0)
    { //new child code
        execvp(arguments[0], arguments);
    }
    else if (pid < 0)
    {
        fprintf(stderr, "the creation of a child process was unsuccessful!");
    }
    else
    { //parent code (wait for the child to finish)
        waitpid(pid, &childStatus, 0);
    }
    return 1;
}

void handleInterrupt(int sig)
{
    printf("\tSignal caught! Instruction/Proccess killed..\n");
}

char **seperateInput(char *input, int *totalCommandsNum)
{
    int index = 0;
    char *tmp;
    char **commands = malloc(sizeof(char *) * 10);
    if (commands == NULL)
        printf("Memory allocation failed!");
    commands[index] = strtok(input, ">");
    tmp = strtok(NULL, ">");
    while (tmp)
    {
        index++;
        commands[index] = tmp;
        tmp = strtok(NULL, ">");
    }
    *totalCommandsNum = index;
    return commands;
}

void forkWithPipes(char **instructions, int totalPipes)
{
    int fd[2];
    int fd2 = 0;
    int status;
    char **arguments;
    int i;
    pid_t pid;

    for (i = 0; i <= totalPipes; i++)
    {
        pipe(fd);
        arguments = parseInstruction(instructions[i]);

        if ((pid = fork()) != 0)
        {
            waitpid(pid, &status, 0);
            close(fd[1]);
            fd2 = fd[0];
        }
        else
        {
            dup2(fd2, 0);
            if (i < totalPipes)
                dup2(fd[1], 1);
            close(fd[0]);
            if (execvp(arguments[0], arguments) == -1)
                exit(1);
        }
    }
}

int checkForRedirections(char *s)
{
    if (strstr(s, "|||"))
        return 3;
    if (strstr(s, "||"))
        return 2;
    if (strstr(s, "|"))
        return 1;
    return 0;
}

void redirectInput(char *s)
{
    int fileDescriptor, childStatus;
    char **arguments;
    pid_t pid;
    char *p1, *p2, *fileName;
    p1 = strstr(s, "|");
    fileName = trimSpaces(p1 + 1);
    *p1 = '\0';
    arguments = parseInstruction(s);

    fileDescriptor = open(fileName, 00);

    pid = fork();
    if (pid == 0)
    { //new child code
        dup2(fileDescriptor, 0);
        execvp(arguments[0], arguments);
    }
    else if (pid < 0)
    {
        fprintf(stderr, "the creation of a child process was unsuccessful!");
    }
    else
    { //parent code (wait for the child to finish)
        waitpid(pid, &childStatus, 0);
    }
    close(fileDescriptor);
}

void singleRedirectOut(char *s)
{
    int fileDescriptor, childStatus;
    char **arguments;
    pid_t pid;
    char *p1, *p2, *fileName;
    p1 = strstr(s, "||");
    fileName = trimSpaces(p1 + 2);
    *p1 = '\0';
    arguments = parseInstruction(s);

    fileDescriptor = open(fileName, O_RDWR); // (O_CREAT | O_RDWR) for file creation

    pid = fork();
    if (pid == 0)
    { //new child code
        dup2(fileDescriptor, 1);
        execvp(arguments[0], arguments);
    }
    else if (pid < 0)
    {
        fprintf(stderr, "the creation of a child process was unsuccessful!");
    }
    else
    { //parent code (wait for the child to finish)
        waitpid(pid, &childStatus, 0);
    }
    close(fileDescriptor);
}

void doubleRedirectOut(char *s)
{
    int fileDescriptor, childStatus;
    char **arguments;
    pid_t pid;
    char *p1, *p2, *fileName;
    p1 = strstr(s, "|||");
    fileName = trimSpaces(p1 + 3);
    *p1 = '\0';
    arguments = parseInstruction(s);

    fileDescriptor = open(fileName, O_APPEND | O_RDWR); //add O_CREAT for file creation

    pid = fork();
    if (pid == 0)
    { //new child code
        dup2(fileDescriptor, 1);
        execvp(arguments[0], arguments);
    }
    else if (pid < 0)
    {
        fprintf(stderr, "the creation of a child process was unsuccessful!");
    }
    else
    { //parent code (wait for the child to finish)
        waitpid(pid, &childStatus, 0);
    }
    close(fileDescriptor);
}

int main(void)
{
    int childStatus, i, pipes, pipefd[2], redFlag;
    char *input;
    char **arguments;
    char **instructions;
    pid_t pid;

    while (1)
    {
        arguments = NULL;
        pipes = 0;
        printPrompt();
        signal(2, handleInterrupt);
        input = getEnteredInstruction();
        if (!strcmp(input, "")) //only whitespace was entered
            continue;

        instructions = seperateInput(input, &pipes);
        if (pipes == 0)
        {
            redFlag = checkForRedirections(instructions[0]);
            if (redFlag == 3) //dipli anakateuthinsi eksodou
            {
                doubleRedirectOut(instructions[0]);
                continue;
            }
            else if (redFlag == 2) //moni anakateuthinsi eksodou
            {
                singleRedirectOut(instructions[0]);
                continue;
            }
            else if (redFlag == 1) //anakateuthunsi eisodou
            {
                redirectInput(instructions[0]);
                continue;
            }

            arguments = parseInstruction(instructions[0]);

            if (strcmp(arguments[0], "exit") == 0)
                exit(0);
            if (strcmp(arguments[0], "cd") == 0)
            {
                if (arguments[1] == NULL)
                {
                    chdir(getenv("HOME"));
                    continue;
                }
                else
                {
                    int tmp = chdir(arguments[1]);
                    if (tmp < 0)
                        printf("Directory not found!");
                    continue;
                }
            }

            forkAndExecute(arguments);
        }
        else
        {
            forkWithPipes(instructions, pipes);
        }
    }
    return 0;

    //Debugging
    //for (i = 0; arguments[i] != NULL; i++)
    //        printf("argv[%d]=%s\n", i, arguments[i]);
    //    printf("  %d  ", pipes); */
}