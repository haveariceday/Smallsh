# Smallsh
## OSU Winter2023 | CS344- Operating Systems✨

#### Smallsh Goals and Requirements
In this assignment you will write smallsh your own shell in C. smallsh will implement a command line interface similar to well-known shells, such as bash. Your program will

- Print an interactive input prompt
- Parse command line input into semantic tokens
- Implement parameter expansion
- Shell special parameters $$, $?, and $!
- Tilde (~) expansion
- Implement two shell built-in commands: exit and cd
- Execute non-built-in commands using the the appropriate EXEC(3) function.
- Implement redirection operators ‘<’ and ‘>’
- Implement the ‘&’ operator to run commands in the background
- Implement custom behavior for SIGINT and SIGTSTP signals


#### How to Run and Exit
Make sure that you have an environment where you can run C program. There are one of two ways to run the project.
1. With the attached the makefile.
```sh
make
./smallsh
ctrl + c to exit
```

2. Compile files with gcc 99

```sh
gcc -std=c99 -o smallsh smallsh.c
./smallsh
ctrl + c to exit
```

#### Duration of the project
It took me about 10 days to build this project. Prior to this project, I already finished "base64 encode project" and "tree project" to warm up.


## License
Created by Hiromi Watanabe. Please do not use this to cheat on your project.

