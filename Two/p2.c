/*  
 *  p2.c
 *
 *  Student Name: Manuel Perez
 *
 *  Instructor Name: John Carroll
 *
 *  Class: CS570
 *
 *  Due Date: 11/29/20
 *
 *  Synopsis - This program is a simple shell composed of a word analyzer. It takes input from the user and analyzes
 *             given words to perform special tasks.
 */

#include "p2.h"

/**************** Used in parse() and transferChars() *******************/
int max;        //maximum number of words
int pos = 0;    //also used to keep track of where we are in bigbuffer
int numWords;   //used to keep track of number of words in parse
int letterCount = 0;    //here is where we store what getword returns as the num of chars
int nulls;	//Keeps track of the number of nulls in newargv array
int i = 0;      //for keeping track on a loop in parse
int hereisTemp;	//temporary variable for a check in parse()
int readTemp;   //temporary variable for a check in transferChars()
int writeTemp;  //temporary variable for a check in transferChars()
int skip;	//If invalid username, we skip forking and reprompt
char s[STORAGE];	//final string that returns from getword
char *newargv[MAXITEM];	//array where we store all of our arguments
char bigbuffer[25500];	//big array where we store every string from getword
char *w;	//Used in getword to form strings

// For input and output redirection
char *inptr;	//we store final input string here
char inbuff[255];	//array to form input string
char *outptr;	//we store final output string here
char outbuff[255];	//array to form output string


/**************** Used for changeEnviron() *****************/
char pathBuff[255];	//array to form path string
char *path;	//we store path string here
int sizePath;	//length of path
char *home;	//used to store environment name
int sizeHome;	//length of environment name
char *env_name;	//used to store environment name
char *env_value;
char *bname;	//used to hold our basename


/**************** Used for hereis and username lookup *****************/
// Username
FILE *fp;
char *username;	//Used to store username
int field = 1;	//to keep track where we are in the username lookup string
int sizeName;	//length of name
char *name;	//stores name

// Hereis
FILE *hereis_fp;
char *delim;	//stores delimeter

// In Both
size_t linecapp = 0;
char *lineptr = NULL;
char *currentLine = NULL;
char tempBuff[255];


/**************** Universal variables and flags *****************/
int j;		// used to keep track on loop in transferChars and closePipes
int result = 0; //set to 1 if the command we tried to do failed
int METAFLAG;	//Set to 1 in getword if we have a metaflag character
int AMPERSANDFLAG;   //Sets the flag to 1 if we have encountered an ampersand in getword
int READFLAG;   //set to 1 if we have encountered '<'
int WRITEFLAG;  //set to 1 if we have encountered '>'
int DOLLAR_FLAG;   //set to 1 if we have encountered '$'
int TILDE_FLAG; //set to 1 if we have encountered '~'
int HEREIS_FLAG;  //set to 1 if we have encountered '<<'


/**************** Used in pipeline() *****************/
pid_t child;	//child pid
pid_t dcpid;	//dead child pid
pid_t gcpid;	//we use wait() and assign the return value to gcpid
pid_t grandchild;   //grandchild pid in the pipeline function
pid_t greatGrandchild; //greatGrandchild pid in the pipeline function
int numPipes;   //number of pipes ('|') we have encountered
int pipes[10];  //we can only have up to 10 pipes
int fildes[20]; //we need 20 filde descriptors for 10 pipes


/**************** Used in inputRedirection() and outputRedirection() *******************/
int hereis_dup;	//used to dup2 the temporary file created by hereis
int hereis_fd;	//used when opening our temporary file created by hereis
int in_fd;	//used when opening our input redirection file
int inDup;	//used to dup2 our input redirection file
int out_fd;	//used when opening our output redirection file
int outDup;	//used to dup2 our output redirection file

/* We reset all our variables and flags for the next line/reprompt */
void resetVars() {
    numWords = 0;
    i = 0;
    pos = 0;
    READFLAG = 0;
    WRITEFLAG = 0;
    field = 1;
    nulls = 0;
    numPipes = 0;
}

/* looks up username from /etc/passwd */ 
char *usernameLookup(char *user) {

    if ((fp = fopen("/etc/passwd", "r")) == NULL) { //try to open file
	perror("Couldn't open /etc/passwd");
	exit(6);
    }
    // go through every line in the file
    while (getline(&lineptr, &linecapp, fp) != -1) {
	currentLine = strtok(lineptr, ":");	//go to the next field
	if ((strcmp(currentLine, user)) == 0) {	//true if we reached the desired user
	    while (currentLine != NULL) {	//true if next field isn't null
		if (field == 6) return currentLine;	//if we're at the right field, return
		field++;
		currentLine = strtok(NULL, ":");	//go to next field in the string
	    }
	    break;	//if we didn't find the user, then break out of loop
	}
    }
    free(currentLine);
    fclose(fp);	//close file
    perror("username doesn't exist.");
    return NULL;

}

//utility method for parse()
void transferChars(char *bigbuff) {
      
    w = bigbuff + pos;	//increase w by offset of strings we built or processed
    bigbuff += pos;	//increase our position in bigbuff by the offset of the strings processed

    if (letterCount == -255) return;	//we reached EOF

    //if reached the end of the line, place null at the end and return
    else if (letterCount == 0 && numWords > 0) {
        *bigbuff = '\0';
        bigbuff++;
        pos++;
    }
  
    //if we encounter ampersand place null in our big buffer and return
    else if (AMPERSANDFLAG == 1 && s[0] == '&') {
        *bigbuff = '\0';
        return;
    }

    //if we didn't receive a word and don't have any words in our newargv,
    //then assign null to our bigbuffer and return
    else if (letterCount == 0 && numWords == 0) {
        *bigbuff = '\0';	//assigning null
        return;
    }

    else if (TILDE_FLAG == 1) {
	int k = 0;

	//if only '~' was encountered then return HOME environment
	if (letterCount == 0) {
	    home = getenv("HOME");  // Storing home directory in home variable
	    if (home == NULL) {
                return;
            }
            sizeHome = strlen(home);  //total number of chars in home directory
            strncpy(w, home, strlen(home));	//place directory string in w
            pos += sizeHome;	//increase our position by the directory string length
	}
	//if a word was after '~' then look for the path from that username
	else if (letterCount >= 1) {
	    for (j = 0; j < letterCount; j++) {
		pathBuff[j] = s[j];
		k++;
	    }
	    pathBuff[k+1] = '\0';

	    username = pathBuff;
	    path = usernameLookup(username);
	    
            if (path == NULL) {	//if no path was found then return
	        return;
	    }

	    //if path was found, place it in our w variable
	    sizePath = strlen(path);
	    strncpy(w, path, sizePath);
	    w += sizePath;
	    *w = '\0';
	    w -= sizePath;
	    pos += sizePath + 1;

	}
	TILDE_FLAG = 0;
	return;
    }

    //hereis detected  
    else if (s[0] == '<' && s[1] == '<') {
	HEREIS_FLAG++;
	hereisTemp++;
	return;
    }

    //here is where we process our hereis work
    else if (hereisTemp == 1) {
	for (j = 0; j < letterCount; j++) {
	    tempBuff[j] = s[j];
	}
 	tempBuff[letterCount] = '\0';
	delim = tempBuff;	//place our delimeter in delim
	
	//open temporary file for writing
	if ((hereis_fp = fopen("/tmp/hereis.txt", "w+")) == NULL) {
	    perror("Couldn't open /tmp/hereis.txt");
	    exit(6);
	}

	//get lines from standard input
	while (getline(&lineptr, &linecapp, stdin) >= 0) {	    
	    currentLine = strtok(lineptr, "\n");  //storing current line and removing new line char
	    
            while (currentLine != NULL) {

		//if we find our delimeter then close file and return
		if (strcmp(currentLine, delim) == 0) {
		    fclose(hereis_fp);
		    free(currentLine);
		    return;
                }
		fprintf(hereis_fp, "%s\n", currentLine);   //store currentline in our tmp file
		currentLine = strtok(NULL, " ");  //get the next word in the current string
	    }
	}

	//if we didn't find our delimeter, close file and return
	fclose(hereis_fp);
	free(currentLine);
	return;
    }

    //we have encountered input redirection char
    else if (s[0] == '<' && METAFLAG == 1) {
        *bigbuff = '\0';
	READFLAG++;	//setting flag
	readTemp = 1;
        return;
    }

    //we store input redirection string into our input buffer
    else if (readTemp == 1) {
        for (j = 0; j < letterCount; j++) {
            inbuff[j] = s[j];
        }
        inbuff[letterCount] = '\0';
        return;
    }
  
    //we have encountered output redirection char
    else if (s[0] == '>' && METAFLAG == 1) {
	*bigbuff = '\0';
        WRITEFLAG++;	//setting flag
	writeTemp = 1;
        return;
    }

    //We redirect output to a variable after a '$'
    else if (writeTemp == 1 && letterCount < 0) {
	letterCount *= -1;
	for (j = 0; j < letterCount; j++) {
	    tempBuff[j] = s[j];
	}
	tempBuff[letterCount] = '\0';

	name = getenv(tempBuff);	//store name of environment

        if (name == NULL) { //true if env variable returns as null
            fprintf(stderr, "%s: Undefined variable.\n", w);
            skip = 1;
	    return;
        }

	sizeName = strlen(name);

	//assign environment name in our output buffer
	for (j = 0; j < sizeName; j++) {
	    outbuff[j] = *name;
	    name++;
	}	
	outbuff[sizeName] = '\0';  //assigning null at the end of our buffer

	return;
    }

    //we store our output redirection string into our output buffer  
    else if (writeTemp == 1) {
        for (j = 0; j < letterCount; j++) {
            outbuff[j] = s[j];
        }
        outbuff[letterCount + 1] = '\0';
	return;
    }
  
    //true if we have encountered a pipe
    else if (s[0] == '|') {
	*bigbuff = '\0';
	bigbuff++;
        numPipes++;
        return;
    }

/* Encountered a valid '$' sign */
    else if (letterCount < 0) {
	DOLLAR_FLAG = 1;
	letterCount *= -1;
	
	for (j = 0; j < letterCount; j++) {
	    *bigbuff = s[j];
	    bigbuff++;
	    pos++;
	}
	*bigbuff = '\0';
	bigbuff++;
	pos++;
	return;
    }

    //normal word, store in our big buffer
    for (j = 0; j < letterCount; j++) {
        *bigbuff = s[j];
        bigbuff++;
        pos++;  
    }

    //Only add null at the end here if we added a word to bigbuff
    if (letterCount > 0) {
        *bigbuff = '\0';
        bigbuff++;
        pos++;
    }
}
  
void parse() {
  
    for (;;) {

        letterCount = getword(s);	//get word and return the length of it

	transferChars(bigbuffer);	//process the word we just received

	//if we reached EOF and have no words, return to main
        if ( letterCount == -255 && numWords <= 0 ) {
            newargv[0] = w;     //assigning it EOF
            break;
        }
  
	//if the first character of our word doesn't begin with a metacharacter then place it in newargv
        else if ( ((*w != '\0') && (s[0] != '&')) && ((*w != '\0') && (s[0] != '>' || METAFLAG == 0))
                  && ((*w != '\0') && (s[0] != '<' || METAFLAG == 0)) && ((*w != '\0') && (s[0] != '|')) 
		  && ((*w != '\0') && (DOLLAR_FLAG != 1)) ) {

            newargv[i] = w;
            i++;
	    numWords++;
        }
   
	//if our first char in our word is a metacharacter proceed to the next word
        else if ( ((s[0] == '&') && (AMPERSANDFLAG == 1)) || 
		  ((s[0] == '>') && (WRITEFLAG == 1)) ||
		  ((s[0] == '<') && (READFLAG == 1)) ||
		  ((s[0] == '<') && (s[1] == '<') && (HEREIS_FLAG == 1)) ) {
            continue;
        }
  
	//get value of word after a '$'
	else if ((DOLLAR_FLAG == 1) && (s[0] != '\0')) {
	    name = getenv(w);

	    if (name == NULL) {	//true if env variable returns as null
		fprintf(stderr, "%s: Undefined variable.\n", w);
		DOLLAR_FLAG = 0;	//set our '$' flag to 0
		skip = 1;
		continue;
	    }

	    sizeName = strlen(name); //length of new env variable
	    pos += sizeName;	//advance our pos in bigbuffer by length of env var
	    newargv[i] = name;	//place env variable in newargv
	    i++;	//increase our position in newargv by 1
	    numWords++;	//increase our number of words by 1
	    DOLLAR_FLAG = 0;	//set our '$' flag to 0
	    continue;
	}
 
	//if we encountered a pipe set a null in newargv
        else if ((s[0] == '|') && (numPipes >= 1)) {
	    max = (numWords > MAXITEM) ? MAXITEM : i;
            newargv[max] = NULL;
	    i++;
	    nulls++;
	    pipes[numPipes] = max + 1;
	    continue;
        }
  
	//place input redirection string into inptr
        else if (readTemp == 1) {
            inptr = inbuff;
	    readTemp = 0;
            continue;
        }
  
	//place output redirection string into outptr
        else if (writeTemp == 1) {
	    writeTemp = 0;
            outptr = outbuff;
            continue;
        }

	//we already did the work for hereis, make temp flag 0 again
	else if (hereisTemp == 1) {
	    hereisTemp = 0;
	    continue;
	}
  
	//if no more words, place null at the end of newargv
        else if (numWords > 0) {
            max = (numWords > MAXITEM) ? MAXITEM : numWords;
            newargv[max + nulls] = NULL;
            break;
        }

	//if there was no words, place null in first place of newargv
        else {
            newargv[0] = NULL;
            break;
        }
    }
}

/* Checks for multiple redirections */
int checkForMultipleRedirections() {

    if ((READFLAG > 1) || (WRITEFLAG > 1) || (READFLAG >= 1 && HEREIS_FLAG >= 1)) {

	if (READFLAG >= 1 && HEREIS_FLAG >= 1) {
	    perror("Cannot have both \'<\' and \'<<\'.");
	    return 1;
	}
	if (READFLAG > 1) {
	    perror("Cannot have multiple \'<\'.");
	    return 1;
	}
	if (WRITEFLAG > 1) {
	    perror("Cannot have multiple \'>\'.");
	    return 1;
	}
    }
    else { return 0; } //if there's no mult redirection, return 0
}

/* handles input redirection */
int inputRedirection() {

    //if true, handle '<<'
    if (HEREIS_FLAG == 1) {
	hereis_fd = open("/tmp/hereis.txt", O_RDONLY);
	if (hereis_fd < 0) {
	    perror("Couldn't open temporary file.");
	    exit(6);
        }

	hereis_dup = dup2(hereis_fd, STDIN_FILENO);
	if (hereis_dup < 0) {
	    perror("Could not dup2 temporary file.");
	    exit(6);
	}

	HEREIS_FLAG = 0;
	remove("/tmp/hereis.txt");	//delete temporary file
	close(hereis_fd);
	return 0;
    }

    //this handles normal input redirection: '<'
    in_fd = open(inptr, O_RDONLY);
    if (in_fd < 0) {
        perror("Cannot open file or file doesn't exist.");
        exit(6);
    }
    inDup = dup2(in_fd, STDIN_FILENO);
    if (inDup < 0) {
        perror("Could not dup2 input file.");
        exit(6);
    }

    READFLAG = 0;
    close(in_fd);
    return 0;
}  

/* handles output redirection */
int outputRedirection() {

    out_fd = open(outptr, O_CREAT | O_EXCL | O_RDWR, S_IRUSR | S_IWUSR);
    if (out_fd < 0) {
        perror("File already exists and cannot overwrite.");
        exit(6);
    }
    outDup = dup2(out_fd, STDOUT_FILENO);
    if (outDup < 0) {
        perror("Could not dup2 output file.");
        exit(6);
    }

    WRITEFLAG = 0;
    close(out_fd);
    return 0;
}

/* Deals with cd command */
int changeDirectory() {
    /* Deals with lone cd */
    if (numWords == 1) { //this means the command typed is only cd
        home = getenv("HOME");  // Storing home directory in home variable
        sizeHome = strlen(home);  //total number of chars in home directory
        if (chdir(home) == 0) {
	    bname = basename(home);
            resetVars();
            return 1;
        }
        else {
	    perror("Unable to change to home directory.");
            return 0;
        }
    }
    //Deals with cd and a variable
    else if (numWords == 2) {
        home = newargv[1];  // Storing target directory in home variable
        sizeHome = strlen(home);  //total number of chars in home directory
	if (chdir(home) == 0) {
	    if (username != NULL) {
		bname = basename(username);
		resetVars();
		return 1;
	    }
	    else {
	        bname = basename(home);
                resetVars();
		return 1;
	    }
        }
        else {
            (void) fprintf(stderr, "Couldn't change to %s.\n", newargv[1]);
            return 0;
        }
    }
    //Runs if there's 3 arguments or more
    else {
	perror("More than 2 arguments.");
        resetVars();
        return 1;
    }
}

/* Deals with environ command */
int changeEnviron() {

/* deals with 0 args passed to the command */
    if (numWords == 1) { //this means the command typed is only environ
	perror("Zero args provided to the environ command.");
	resetVars();
	return 1;
    }

/* This handles 1 argument */
    else if (numWords == 2) {
	env_name = getenv(newargv[1]);
        if (env_name != 0) {
	    printf("%s\n", env_name);
	    resetVars();
	    return 1;
	}
	else {
            (void) fprintf(stderr, "%s\n", env_name);
	    resetVars();
            return 1;
        }
    }

/* This handles 2 arguments */
    else if (numWords == 3) {
	env_name = newargv[1];
	env_value = newargv[2];
	if (setenv(env_name, env_value, 1) < 0) {
	    (void) fprintf(stderr, "Unable to set environ to: %s.\n", env_value);
	    resetVars();
	    return 1;
	}
	resetVars();
	return 1;
    }

/* This is if more than 2 args are passed to the command */
    else {
	perror("More than 2 arguments.");
        resetVars();
        return 1;
    }
}

/* close all file descriptors */
void closeAllPipes() {

    for (j = 0; j < numPipes * 2; j++) {
	close(fildes[j]);
    }

}

//Handles one or multiple pipe commands
void pipeline() {
    for (j = 0; j < numPipes; j++) {

        if (pipe(fildes + (j * 2)) < 0) {
	    perror("Failed to pipe() file descriptors");
	    exit(2);
	}
    }

    if (-1 == (child = fork())) {
        perror("Couldn't fork: child");
        exit(1);
    }
    else if (child == 0) {

        if (-1 == (greatGrandchild = fork())) {
            perror("Couldn't fork: great grandchild");
            exit(1);
        }
        else if (greatGrandchild == 0) {

            for (j = 0; j < numPipes - 1; j++) {
                grandchild = fork();
                if (grandchild > 0) {
                    if (dup2(fildes[j * 2], STDIN_FILENO) < 0) {
			perror("Failed to dup2 grandchild");
			exit(2);
		    }
                    if (dup2(fildes[(j * 2) + 3], STDOUT_FILENO) < 0) {
			perror("Failed to dup2 grandchild");
			exit(2);
		    }
                    closeAllPipes();

                    fflush(NULL);
                    if (execvp(newargv[pipes[j + 1]], newargv+pipes[j + 1]) < 0) {
			perror("Failed to execvp grandchild command");
			exit(5);
		    }
                }
            }

            if (dup2(fildes[1], STDOUT_FILENO) < 0) {
		perror("Failed to dup2 in great grandchild");
		exit(2);
	    }
            closeAllPipes();

            result = checkForMultipleRedirections();
            if (result == 1) exit(2);

            if (READFLAG == 1 || HEREIS_FLAG == 1) {
                result = inputRedirection();
                if (result == 1) exit(4);
            }

            fflush(NULL);
            if (execvp(newargv[0], newargv) < 0) {
		perror("Failed to exevp great grandchild command");
		exit(5);
	    }
        }
        
        if (dup2(fildes[(numPipes - 1) * 2], STDIN_FILENO) < 0) {
	    perror("Failed to dup2 in child");
	    exit(2);
	}
        closeAllPipes();

        result = checkForMultipleRedirections();
        if (result == 1) exit(2);
        if (WRITEFLAG == 1) {
            result = outputRedirection();
            if (result == 1) exit(3);
        }

        fflush(NULL);
        if (execvp(newargv[pipes[numPipes]], newargv+pipes[numPipes]) < 0) {
	    perror("Failed to execvp child command");
            exit(5);
	}
    }
}

void nonPipeline() {
    if (-1 == (child = fork())) {
	perror("Could not fork child.");
        exit(1);
    }
    else if (child == 0) {

        result = checkForMultipleRedirections();
        if (result == 1) exit(2);

        if (READFLAG == 1 || HEREIS_FLAG == 1) {
	    result = inputRedirection();
            if (result == 1) exit(3);
        }
        if (WRITEFLAG == 1) {
	    result = outputRedirection();
            if (result == 1) exit(4);
        }

	fflush(NULL);
        execvp(newargv[0], newargv);

        /* Only reached if execvp failed */
        perror("Execvp failed.");
        exit(5);

    }

}

void myHandler() {}
  
int main(int argc, char *argv[]) {
  
    setpgid(0, 0);
    (void) signal(SIGTERM, myHandler);
  
    for(;;) {

	//Handles the prompt
	if (bname == NULL) {
	    printf(":570: ");
	}
	else {
	    printf("%s:570: ", bname);
	}

        parse();
  
        if (newargv[0] == w) break;
        if (newargv[0] == NULL) continue;

	//If username look up failed, skip forking and reprompt
	if (skip == 1) {
	    skip = 0;
	    resetVars();
	    continue;
	}
 
	//handles cd command   
        if (strcmp(newargv[0], "cd") == 0) {
            result = changeDirectory();
            if (result == 1) {
                continue;   
            }   
        }

	//Handles environ commmand
	else if (strcmp(newargv[0], "environ") == 0) {
	    result = changeEnviron();
	    if (result == 1) {
		continue;
	    }
	}

        fflush(NULL);

	if (numPipes >= 1) {
	    pipeline();
	    closeAllPipes();
	}
	else {
	    nonPipeline(); //Do a normal fork()
	}

	/* Only the parent reaches this point */

	/* background job does not wait for child */
	if (AMPERSANDFLAG == 1) {
            printf("%s [%d]\n", newargv[0], child);

            /* child cannot read from standard input */
            if (READFLAG == 0) {
                int devnull = open("/dev/null", O_RDONLY);
                if (devnull < 0) { 
		    perror("Couldn't open /dev/null");
		    exit(6);
		}
                close(devnull);
            }
            resetVars();
            AMPERSANDFLAG = 0;
	}

	else {
	    //wait for child to finish process
            for(;;) {
                dcpid = wait(NULL);
                if (dcpid == child) break; //break when child process is done
            }
            resetVars();
	}    
  
    }
  
    killpg(getpgrp(), SIGTERM);// Terminate any children that are
                               // still running. WARNING: giving bad args
                               // to killpg() can kill the autograder!
    printf("p2 terminated.\n");// MAKE SURE this printf comes AFTER killpg
    _exit(0);
}
