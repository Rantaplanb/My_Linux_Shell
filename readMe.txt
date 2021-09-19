I implemented a very basic linux command shell.
Compile with: ```gcc -o shell shell.c```
Run with: ```./shell```

Features: 
	-Excecution of binary commands
	-Excecution of system commands (cd, exit)
	-Pipelining between instructions
	-Redirection of I/O
	-Shortcuts (ctrl + C)

Let me give you a simplified explanation of how the program works.
When the program is executed, a command prompt appears and the
life cycle of the program beggins. The user may type line of command(s).
Then, the following steps take place:
	1. Input line is stored.
	2. It is decided if there are pipes or redirections
	3. The input gets parsed and splitted into instructions->tokens
	4. The token/argument buffer gets passed to execvp and the
entered instructions are executed.
	5. Repeat the sequence of steps
	

I hope that you like it, thanks for checking it out :)
