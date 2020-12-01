#include <stdio.h>	//fflush(), perror(), getline()
#include <stdlib.h>	//exit(), getenv(), setenv()
#include <unistd.h>	//dup2(), chdir(), fork(), execvp(), setpgid(), getpgrp(), access(), pipe(), getcwd(), stat(), close()
#include <fcntl.h> 	//open()
#include <signal.h>	//killpg(), sigaction(), signal()
#include <sys/types.h>	//open(), wait(), stat()
#include <sys/wait.h>	//wait()
#include <sys/stat.h>	//stat(), open()
#include <string.h>	//strsep(), strtok()
#include <libgen.h>	//basename()

#include "getword.h"

#define MAXITEM 100 /* max number of words per line */

#define MAXARGS 20

