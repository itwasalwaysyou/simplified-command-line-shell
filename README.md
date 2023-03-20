# simplified-command-line-shell
The goal of this project is to write a simplified command-line shell called swish. This shell will be less functional in many ways from standard shells like bash (the default on most Linux machines), but will still have some useful features.

This project will cover a number of important systems programming topics:

    String tokenization using strtok()
    Getting and setting the current working directory with getcwd() and chdir()
    Program execution using fork() and wait()
    Child process management using wait() and waitpid()
    Input and output redirection with open() and dup2()
    Signal handling using setpgrp(), tcsetpgrp(), and sigaction()
    Managing foreground and background job execution using signals and kill()




## Job Lifecycle

You might find this diagram that (partially) details a job’s lifecycle helpful in trying to understand how to manage the shell’s job list. Note that, technically, a background job could also be suspended by receipt of SIGSTP. 

![image](https://user-images.githubusercontent.com/112202949/226257228-51b3ff48-7a75-498c-bbb2-ba1aeb6d5db5.png)



## Task 0: String Tokenization

One of the most important tasks of any shell is to break user input into tokens – essentially substrings separated by whitespace. For example, the string `ls -l -a` consists of three tokens: `ls`, `-l` and `-a`.


## Task 1: Working Directory Management

The `main()` function in `swish.c` is built around a simple loop: read a line from standard input, tokenize it, and then perform some operation based on the comments of this line. If certain tokens are present at the beginning of this line, your shell should perform built-in operations. Otherwise, it will default to running a new program whose name is specified by the first token.

for example: 
If the user types in `cd`, the `shell` should change the current working directory.

## Task 2: Running Commands

When the first token from the user’s input doesn’t match any built-in operation (e.g., `cd`, `pwd`), shell should take the following steps (which you should implement in the main() function’s while loop):

    fork() a child process, which will then call run_command().
    Within run_command(), build up the necessary arguments for the new program and use an exec() system call to start running that program.
    In the parent process, wait for the child process to terminate.
    
    

## Task 3: Redirecting Input and Output

this shell give you the ability to redirect standard input and standard output when running a program. 

Here are a few examples:

`ls -l > out.txt` redirects the output of `ls -l` from standard output to the `file out.txt`. That is, rather than printing to the screen, ls writes to the designated file. If the file already exists, it is truncated and overwritten.
`cat < gatsby.txt` redirects input to the `cat` program from standard input to the file `gatsby.txt`. That is, rather than consuming keyboard input from the user, the program reads from the specified file.
`ls -l >> out.txt` redirects the output of `ls -l` from standard output to the `file out.txt`. However, out.txt is not overwritten if it already exists. Rather, the output is appended to the end of the existing file. The file is created if it does not already exist.


## Task 4: Basic Signal Management

Terminals usually give the user the ability to send signals to the currently running program. It is also possible to “pause” programs to be resumed later. The keyboard shortcut `Ctrl-Z` traditionally sends the SIGSTP signal to the currently running program. 

We can resolve this through the concept of foreground and background process groups. Each process in a Unix-based system is given a unique process ID. Every process also possesses a (potentially non-unique) `process group ID` (pgid), which we can get or set using the `getpgid()` and `setpgid() `system calls. By default, a process inherits the pgid of its parent.


## Task 5: Dealing with Stopped Processes

Typing `Ctrl-Z` into your shell will send the SIGSTP signal to the foreground process group. This will pause, but not terminate, the currently executing program. However, this will not cause a simple call to `wait()` or `waitpid()` to return as it would when the program exits.

```
@> jobs
0: wc (stopped)
1: list_main (stopped)
```

## Task 6: Background Job Execution

Real shells allow you to run programs in the background. This is accomplished by creating and starting a process for the user command and then immediately re-prompting the user for the next command while the previous command runs in parallel. Here, we add support for creating background processes and cleaning up terminated background processes.
