#include "shell.h"
/*builtin commands*/
	char *builtins[] = {"echo", "printf", "read", "cd", "pwd", "pushd", "popd", "dirs", "let", "eval",
						"set", "unset", "export", "declare", "typeset", "readonly", "getopts", "source",
						"fg","bg","jobs","exit", "exec", "shopt", "caller", "true", "type", "hash", "bind", "help", NULL};


  //fetch the first word -->ls -l so, fetch first word ls;
  //return ls

int status = 0;
int last_status = 0;
volatile sig_atomic_t got_sigchld = 0;

Job *head = NULL;
int job_count = 0;

void add_job(pid_t pid, char *cmd, int stopped)
{
    Job *new = malloc(sizeof(Job));
    new->pid    = pid;
    new->status = stopped;
    new->next   = NULL;
    strncpy(new->cmd, cmd, 99);

    job_count++;
    new->job_id = job_count;

    /* append to end of list */
    if (head == NULL) { head = new; return; }
    Job *tmp = head;
    while (tmp->next) tmp = tmp->next;
    tmp->next = new;
}

void print_list()
{
    Job *tmp = head;
    if (tmp == NULL) { printf("No jobs\n"); return; }
    while (tmp)
    {
        printf("[%d]+  %s\t\t%s\n",
               tmp->job_id,
               tmp->status ? "Stopped" : "Running",
               tmp->cmd);
        tmp = tmp->next;
    }
}


void delete_pid(pid_t pid)
{
    Job *tmp = head, *prev = NULL;
    while (tmp)
    {
        if (tmp->pid == pid)
        {
            if (prev) prev->next = tmp->next;
            else      head       = tmp->next;
            free(tmp);
            job_count--;
            return;
        }
        prev = tmp;
        tmp  = tmp->next;
    }
}

Job *get_job_by_pid(pid_t pid)
{
    Job *tmp = head;
    while (tmp)
    {
        if (tmp->pid == pid) return tmp;
        tmp = tmp->next;
    }
    return NULL;
}
char *get_command(char *input_string)
{
    static char command[100];
    int i = 0;

    if (input_string == NULL || *input_string == '\0')
    {
        command[0] = '\0';
        return command;
    }

    /* skip spaces */
    while (*input_string == ' ')
        input_string++;

    /* copy first word */
    while (*input_string != ' ' &&
           *input_string != '\0' &&
           *input_string != '\n' &&
           i < 99)              /* ADD boundary check */
    {
        command[i++] = *input_string++;
    }

    command[i] = '\0';
    return command;
}

void extract_external_commands(char **external_commands)
{
    FILE *fp;
    char buffer[100];
    int i = 0;

    /* open file */
    fp = fopen("external_commands.txt", "r");

    if(fp == NULL)
    {
        printf("File not found\n");
        return;
    }

    /* read line by line */
    while(fgets(buffer, sizeof(buffer), fp) != NULL)
    {
        /* remove newline character */
        buffer[strcspn(buffer, "\n")] = '\0';

        /* allocate memory */
        external_commands[i] = malloc(strlen(buffer) + 1);

        if(external_commands[i] == NULL)
        {
            printf("Memory allocation failed\n");
            return;
        }

        /* copy command */
        strcpy(external_commands[i], buffer);

        i++;
    }

    /* last element NULL */
    external_commands[i] = NULL;

    fclose(fp);
}


int check_command_type(char *command, char **external_commands)
{
    int i;

    /* check builtin commands */
    for(i = 0; builtins[i] != NULL; i++)
    {
        if(strcmp(command, builtins[i]) == 0)
        {
            return BUILTIN;
        }
    }

    /* check external commands */
    for(i = 0; external_commands[i] != NULL; i++)
    {
        if(strcmp(command, external_commands[i]) == 0)
        {
            return EXTERNAL;
        }
    }
    return NO_COMMAND;
}

void execute_internal_commands(char *input_string)
{
    char buff[1024];

    /* exit */
    if (strcmp(input_string, "exit") == 0)
        exit(0);

    /* pwd */
    else if (strcmp(input_string, "pwd") == 0)
    {
        getcwd(buff, sizeof(buff));
        printf("%s\n", buff);
    }

    /* cd */
    else if (strcmp(input_string, "cd") == 0)
        chdir(getenv("HOME"));

    else if (strncmp(input_string, "cd ", 3) == 0)
    {
        if (chdir(input_string + 3) == -1)
            perror("cd");
    }

    /* echo */
    else if (strncmp(input_string, "echo ", 5) == 0)
    {
        char *arg = input_string + 5;

        if (strcmp(arg, "$$") == 0)
            printf("%d\n", getpid());
        else if (strcmp(arg, "$?") == 0)
            printf("%d\n", last_status);
        else if (arg[0] == '$')
        {
            char *val = getenv(arg + 1);
            printf("%s\n", val ? val : "");
        }
        else
            printf("%s\n", arg);
    }

    /* jobs */
    
    else if (strcmp(input_string, "jobs") == 0)
    {
	    print_list();
    }
    
    
    /* fg */
    
    else if (strcmp(input_string, "fg") == 0 || strncmp(input_string, "fg %", 4) == 0)
    {
	    Job *j = NULL;
	    if (head == NULL) 
	    {
		    printf("fg: no jobs\n");
	    }
	    else
	    {
		    if (strcmp(input_string, "fg") == 0)
		    {
			    /* get last job */
			    j = head;
			    while (j->next) j = j->next;
		    }
		    
		    else
		    {
			    j = get_job_by_pid(atoi(input_string + 4));
		    }
		    
		    if (j == NULL)
		    {
			    printf("bg: job not found\n");
		    }

		    
		    else
		    {
			    printf("%s\n", j->cmd);
			    pid = j->pid;
			    char saved_cmd[100];
			    strncpy(saved_cmd, j->cmd, 99);
			    delete_pid(pid);kill(pid, SIGCONT);
			    waitpid(pid, &status, WUNTRACED);
			   
			    if (WIFEXITED(status))
				    last_status = WEXITSTATUS(status);
			   
			    else if (WIFSIGNALED(status))
				    last_status = 128 + WTERMSIG(status);
           
			    else if (WIFSTOPPED(status))
			    {
                
				    last_status = 128 + WSTOPSIG(status);
				    add_job(pid, saved_cmd, 1);
				    printf("\n[%d] Stopped\t%s\n", job_count, saved_cmd);
			    }
			    pid = 0;
		    }
	    }
    }
    
    /* bg */

    else if (strcmp(input_string, "bg") == 0 ||strncmp(input_string, "bg %", 4) == 0)
    {
	    Job *j = NULL;
	    if (head == NULL) { printf("bg: no jobs\n"); }
	    else
	    {
		    if (strcmp(input_string, "bg") == 0)
		    {
			    j = head;
			    while (j->next) 
				    j = j->next;
		    }
		    else
			    j = get_job_by_pid(atoi(input_string + 4));

		    if (j == NULL) 
		    {
			    printf("bg: job not found\n");
		    }
		    else
		    {
			    j->status = 0;
			    kill(j->pid, SIGCONT);
			    printf("[%d] %s &\n", j->job_id, j->cmd);
		    }
	    }
    }
}

void execute_external_commands(char *input_string)
{
    /* Check if pipe is present */
    if (strchr(input_string, '|') == NULL)
    {
        char *argv[20];
        int i = 0;

        /* Convert command string to argv[] */
        argv[i] = strtok(input_string, " ");
        while (argv[i] != NULL)
        {
            i++;
            argv[i] = strtok(NULL, " ");
        }

        /* NULL check before execvp */
        if (argv[0] == NULL)
        {
            fprintf(stderr, "Error: empty command\n");
            exit(1);
        }

        /* Execute external command */
        execvp(argv[0], argv);

        /* execvp only reaches here on failure */
        fprintf(stderr, "%s: command not found\n", argv[0]);
        exit(127);
    }
    else
    {
        char *cmds[20];
        int cmd_count = 0;

        /* Split input string using pipe delimiter */
        cmds[cmd_count] = strtok(input_string, "|");
        while (cmds[cmd_count] != NULL)
        {
            cmd_count++;
            cmds[cmd_count] = strtok(NULL, "|");
        }

        /* Calculate number of pipes */
        int pipe_count = cmd_count - 1;

        char *commands[20][20];

        /* Convert each command into argv format */
        for (int i = 0; i < cmd_count; i++)
        {
            int j = 0;

            /* Trim leading spaces from each command */
            while (cmds[i] && *cmds[i] == ' ')
                cmds[i]++;

            commands[i][j] = strtok(cmds[i], " ");
            while (commands[i][j] != NULL)
            {
                j++;
                commands[i][j] = strtok(NULL, " ");
            }
        }

        int fd[20][2];

        /* Create ALL pipes BEFORE forking */
        for (int i = 0; i < pipe_count; i++)
        {
            if (pipe(fd[i]) == -1)
            {
                perror("pipe");
                exit(1);
            }
        }

        /* Create one process per command */
        for (int i = 0; i < cmd_count; i++)
        {
            pid_t pid = fork();

            if (pid == -1)
            {
                perror("fork");
                exit(1);
            }

            if (pid == 0)
            {
                /* Connect previous pipe to stdin (not for first command) */
                if (i > 0)
                {
                    if (dup2(fd[i - 1][0], STDIN_FILENO) == -1)
                    {
                        perror("dup2 stdin");
                        exit(1);
                    }
                }

                /* Connect stdout to next pipe (not for last command) */
                if (i < pipe_count)
                {
                    if (dup2(fd[i][1], STDOUT_FILENO) == -1)
                    {
                        perror("dup2 stdout");
                        exit(1);
                    }
                }

                /* Close ALL pipe descriptors in child */
                for (int j = 0; j < pipe_count; j++)
                {
                    close(fd[j][0]);
                    close(fd[j][1]);
                }

                /* NULL check */
                if (commands[i][0] == NULL)
                {
                    fprintf(stderr, "Error: empty command in pipe\n");
                    exit(1);
                }

                /* Execute command */
                execvp(commands[i][0], commands[i]);

                /* Only reached on failure */
                fprintf(stderr, "%s: command not found\n", commands[i][0]);
                exit(127);
            }
        }

        /* Parent closes ALL pipe descriptors */
        for (int i = 0; i < pipe_count; i++)
        {
            close(fd[i][0]);
            close(fd[i][1]);
        }

        /* Wait for ALL child processes */
        for (int i = 0; i < cmd_count; i++)
        {
            int status;
            wait(&status);
        }
    }
}
