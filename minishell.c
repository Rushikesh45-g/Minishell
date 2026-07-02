#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include "shell.h"


/* Handle Ctrl+C and Ctrl+Z */
void signal_handler(int signum)
{
    char cwd[1024];

    if (signum == SIGINT)
    {
        if (pid == 0)
	{
		char cwd[1024];
		getcwd(cwd, sizeof(cwd));
		char *home = getenv("HOME");
		
		printf(ANSI_COLOR_CYAN "%s" ANSI_COLOR_RESET, prompt);
		
		if (strncmp(cwd, home, strlen(home)) == 0)
		{
			printf(ANSI_COLOR_MAGENTA "~%s$ " ANSI_COLOR_RESET,cwd + strlen(home));
		}
		else
		{
			printf(ANSI_COLOR_MAGENTA "%s$ " ANSI_COLOR_RESET, cwd);
		}
		 write(STDOUT_FILENO, "\n", 1);
		fflush(stdout);
        }
    }
    else if (signum == SIGTSTP)
    {
        if (pid == 0)
        {
            char cwd[1024];
            getcwd(cwd, sizeof(cwd));
            char *home = getenv("HOME");

            printf(ANSI_COLOR_CYAN "%s" ANSI_COLOR_RESET, prompt);
            if (strncmp(cwd, home, strlen(home)) == 0)
            {
                printf(ANSI_COLOR_MAGENTA "~%s$ " ANSI_COLOR_RESET, cwd + strlen(home));
            }
            else
            {
                printf(ANSI_COLOR_MAGENTA "%s$ " ANSI_COLOR_RESET, cwd);
            }
            write(STDOUT_FILENO, "\n", 1);
            fflush(stdout);
        }
    }
    else if (signum == SIGCHLD)
    {
        int s;
        pid_t p;
        while ((p = waitpid(-1, &s, WNOHANG)) > 0)
        {
            Job *j = get_job_by_pid(p);      /* ← by pid, not job_id */
            if (j)
                printf("\n[%d]+  Done\t\t%s\n", j->job_id, j->cmd);
            delete_pid(p);
        }
    }
}

void scan_input(char *prompt, char *input_str)
{
    char *external_commands[152];
    /* Load external commands */
    extract_external_commands(external_commands);
    /* Register signal handlers */
    signal(SIGINT, signal_handler);
    signal(SIGTSTP, signal_handler);
    signal(SIGCHLD, signal_handler);
    while (1)
    {
            char cwd[1024];
            getcwd(cwd, sizeof(cwd));
            char *home = getenv("HOME");
            printf(ANSI_COLOR_CYAN "%s" ANSI_COLOR_RESET, prompt);
            if (strncmp(cwd, home, strlen(home)) == 0)
            {
                    printf(ANSI_COLOR_MAGENTA "~%s$ " ANSI_COLOR_RESET,cwd + strlen(home));
            }
            else
            {
                    printf(ANSI_COLOR_MAGENTA "%s$ " ANSI_COLOR_RESET, cwd);
            }
            fflush(stdout);
        /* Read user input */
        scanf(" %[^\n]", input_str);
        /* Change prompt */
        if (strncmp(input_str, "PS1=", 4) == 0)
        {
            if (input_str[4] != ' ')
            {
                strcpy(prompt, input_str + 4);
            }
            else
            {
                printf("ERROR : Invalid PS1 command\n");
            }
        }
        else
        {
            /* Extract command */
            char *cmd = get_command(input_str);
            if (cmd == NULL || cmd[0] == '\0')
                    continue;
            /* Check command type */
            int ret = check_command_type(cmd, external_commands);
            /* Execute built-in command */
            if (ret == BUILTIN)
            {
                execute_internal_commands(input_str);
            }
            /* Execute external command */
            else if (ret == EXTERNAL)
{
    sigset_t mask, old;
    sigemptyset(&mask);
    sigaddset(&mask, SIGCHLD);
    sigprocmask(SIG_BLOCK, &mask, &old);   /* block SIGCHLD */
    pid = fork();
    if (pid > 0)
    {
        sigprocmask(SIG_UNBLOCK, &mask, NULL); /* unblock after fork */
        waitpid(pid, &status, WUNTRACED);
        if (WIFEXITED(status))
            last_status = WEXITSTATUS(status);
        else if (WIFSIGNALED(status))
            last_status = 128 + WTERMSIG(status);
        else if (WIFSTOPPED(status))
        {
            last_status = 128 + WSTOPSIG(status);
            add_job(pid, input_str, 1);
            printf("\n[%d] Stopped\t%s\n", job_count, input_str);
        }
        pid = 0;
    }
    else if (pid == 0)
    {
        sigprocmask(SIG_SETMASK, &old, NULL);  /* restore in child */
        signal(SIGINT, SIG_DFL);
        signal(SIGTSTP, SIG_DFL);
        execute_external_commands(input_str);
        exit(0);
    }
    else
        perror("fork");
}
            /* Invalid command */
            else  /* NO_COMMAND */
            {
                    fprintf(stderr, ANSI_COLOR_RED "Command '%s' not found\n" ANSI_COLOR_RESET, cmd);
                    last_status = 127;
            }
        }
    }
}