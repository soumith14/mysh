# UNIX Shell Implementation (C, POSIX APIs)

A minimal custom shell in C featuring:
- **Command execution** via `fork()`, `execvp()`, and `waitpid()`
- **I/O redirection** (`<` and `>`) using `open()` and `dup2()`
- **Background jobs** (`&`) with a simple job scheduler and `jobs` built-in
- **Signal handling** (`SIGINT`, `SIGCHLD`) to ignore Ctrl-C in the shell and reap zombies
- **Piping** (`|`) to chain two commands together

## Building & running

```bash
gcc -Wall -Wextra main.c -o mysh
./mysh

## Usage

After compiling, launch with:

```bash
./mysh

# 1. Piping
mysh> ls -l | grep ".c"

# 2. Background jobs
mysh> sleep 10 &
[bg 0] 12345
mysh> jobs

# 3. I/O redirection
mysh> echo "Hello, World" > out.txt
mysh> cat out.txt

# 4. Combined
mysh> grep foo < out.txt | sort > sorted.txt
