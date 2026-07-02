#include "shell.h"

pid_t pid = 0;                  /* Current child process id */
char prompt[50] = "rushi@minishell:"; /* Default shell prompt */

int main(void)
{
    char input_str[100];

    /* Clear terminal screen */
    system("clear");

    /* Start shell */
    scan_input(prompt, input_str);

    return 0;
}
