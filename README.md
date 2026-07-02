# MiniShell - Linux Command Interpreter

A Unix-like MiniShell developed in C as part of the Emertxe Embedded Linux Professional Program.

The shell provides an interactive command-line interface that supports built-in commands, external command execution, process management, signal handling, command pipelines, and job control.

---

## Features

### Built-in Commands
- cd
- pwd
- echo
- jobs
- fg
- bg
- exit

### External Commands
- Executes Linux commands using fork() and execvp().
- Supports all standard Linux utilities available in the system.

### Pipe Support
- Supports single and multiple command pipelines.
- Example:

```bash
ls -l | grep ".c" | wc
```

### Job Control
- Background process management
- Foreground execution
- List running/stopped jobs
- Resume stopped jobs

Commands:
- jobs
- fg
- bg

### Signal Handling
- Ctrl + C (SIGINT)
- Ctrl + Z (SIGTSTP)
- SIGCHLD handling

### Shell Prompt
- Custom shell prompt using PS1

Example

```bash
PS1=RushiShell$
```

### Environment Variables

Supports

```bash
echo $HOME
echo $$
echo $?
```

---

## Technologies Used

- C Programming
- Embedded Linux
- POSIX System Calls
- GCC
- Linux Terminal

---

## System Calls Used

- fork()
- execvp()
- wait()
- waitpid()
- pipe()
- dup2()
- kill()
- signal()
- getcwd()
- chdir()

---

## Project Structure

```
main.c
minishell.c
command.c
shell.h
external_commands.txt
```

---

## Build

```bash
gcc *.c -o minishell
```

---

## Run

```bash
./minishell
```

---

## Sample Commands

```bash
pwd

cd Desktop

ls -l

echo $$

echo $HOME

jobs

fg

bg

ls -l | grep ".c"

exit
```

---

## Learning Outcomes

- Linux Process Management
- Shell Programming
- POSIX System Calls
- Signal Handling
- Job Control
- Inter Process Communication
- Pipe Programming
- Memory Management
