#ifndef MAIN_H
#define MAIN_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdio_ext.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
 

#define BUILTIN		1
#define EXTERNAL	2
#define NO_COMMAND  3

#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_RESET   "\x1b[0m"

typedef struct Job {
    int job_id;
    pid_t pid;
    char cmd[100];
    int status;          /* 0=running, 1=stopped */
    struct Job *next;
} Job;

extern pid_t pid;
extern Job *head;
extern int job_count;      
extern char prompt[50];     
extern int status;          
extern int last_status;


void add_job(pid_t pid, char *cmd, int stopped);
void delete_pid(pid_t pid);
void print_list(void);
extern volatile sig_atomic_t got_sigchld;

Job *get_job_by_pid(pid_t pid);      



void scan_input(char *prompt, char *input_string);
char *get_command(char *input_string);
void copy_change(char *prompt, char *input_string);
int check_command_type(char *command, char **external_commands);
void execute_internal_commands(char *input_string);
void signal_handler(int sig_num);
void extract_external_commands(char **external_commands);
void execute_external_commands(char *input_string);
#endif
