Relevant files: p2.c, getword.c

This program is a command line interpreter for the UNIX system and acts like a shell. It handles built in
commands such as (cd, and environ) as well as executables such as (echo, ls, sleep), and filepaths to
executables. If the command is an executable then an execution of the command is made inside the child
of a fork. Additionally, this program also handles unix redirection with the ">", "<", and the "<<"
metacharacters as well as the ability for a process to run in the background with the "&" metacharacter. 
If the program is misused, appropriate error messages are displayed. This program also uses basic signal 
handling. An example of basic use of this program is: "echo hello > createFile.txt"

Unfortunately for the grading of this project, most of the code needed to be in a couple of files. Which I
realize is unpractical in most cases.

What I learned:
This project taught me a lot about how to structure a long project, and how to develop it incrementally.
I also learned how to use the GDB debugger efficiently, and lastly I learned a lot about how processes work.
