/*  
 *  getword.c
 *
 *  Student Name: Manuel Perez
 *
 *  Instructor Name: John Carroll
 *
 *  Class: CS570
 *
 *  Due Date: 9/15/20
 *
 *  Synopsis - This is a lexical analyzer, it takes a string of chars as input 
 *             from p1.c and returns word lengths as well as word strings
 *             back to p1.c.
 */

/* Include Files */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int getword(char *w)
{
    int iochar = 0;		// Current char
    int lenWord = 0;		// number of chars
    int negNumber = 0;		// 1 if we encountered '$'
    static int prevChar = 0;
    static int prevLen = 0;	// stores the last word length
    static int returnZero = 0;	// 1 when we need to return an extra 0
    static int meta;		// 1 if we encountered metacharacter
    static int forgotLetter;	// 1 if we returned a word and missed the first letter of a new word

    *w = '\0';		// Inserting null at the beginning of every new word
    
    while ( ( iochar = getchar() ) != EOF ) {


	// Checking if the buffer is full of chars, if so add null to the end of the string of chars, and return
	if ( lenWord == 253 ) {
	    prevChar = iochar;
	    lenWord += 1;
	    prevLen = lenWord + 1;
	    *w = iochar;
	    w++;
	    *w = '\0';		// inserting null at the end of the char string
	    return lenWord;
	}
	
	// If the last char is a new line, and we didn't return 0 for it, return 0 now
	else if ( prevChar == 10 && returnZero == 1 ) {
	    forgotLetter = 1;
	    prevChar = iochar;
	    returnZero = 0;
	    return 0;
	}

	// encountered 2 consecutive '<', return length of 2 and string of '<<'
	else if ( prevChar == 60 && iochar == 60 && meta == 1) {
	    lenWord = 2;
	    prevLen = 2;
	    meta = 0;
	    *w = iochar;
	    w++;
	    *w = iochar;
	    w++;
	    *w = '\0';
	    w++;
	    return lenWord;
	}

	// encountered metacharacter, return word in progress
	else if ( prevChar != 92 && (iochar == 124 || iochar == 60 || iochar == 62) && lenWord > 0 ) {
	    prevChar = iochar;
	    prevLen = lenWord;
	    meta = 1;
	    *w = '\0';              // inserting null at the end of the char string
	    w++;
	    return lenWord;
        }

	// returning metacharacter '|' or '<' or '>' with word length of 1
	else if ( meta == 1 && (prevChar == 60 || prevChar == 62 || prevChar == 124) ) {
	    *w = prevChar;
	    w++;
	    prevChar = iochar;
	    lenWord = 1;
	    meta = 0;
	    forgotLetter = 1;
	    *w = '\0';              // inserting null at the end of the char string
	    return lenWord;
	}

	// checking for the metacharacter '|' without a backlash before it
	else if ( prevChar != 92 && iochar == 124 ) {
            prevChar = iochar;
            lenWord += 1;
            prevLen = lenWord + 1;
            *w = iochar;
            w++;
            *w = '\0';
            return lenWord;
        }

	else if ( iochar == 10 ) {	// checking for new line

	    // If iochar is new line and prevChar is '$' then return 0
	    if ( lenWord == 0 && prevChar == 36 ) {
		returnZero = 1;
		prevChar = iochar;
		return 0;
	    }

	    else if ( lenWord  == 0 ) {		// There's no word, output 0
		prevChar = iochar;
		return 0;
	    }

	    else if ( prevChar == 92 ) {	// Is the prev char a '\'
		lenWord--;
		prevChar = 32;
		*w--;				
		*w-- = '\0';			// Removing the '\' from the string of chars
		
		if ( negNumber == 0 ) {		// Word didn't start with '$', output pos # 
		    prevLen = lenWord;
		    return lenWord;
		}

		else {				// Word started with '$', output neg #  
		    lenWord *= -1;
		    prevLen = lenWord;
		    negNumber = 0;
		    return lenWord;
		}
	    }

	    else {
		if ( negNumber == 0 ) {		// Word didn't start with '$', output pos # 
		    prevLen = lenWord;
		    prevChar = iochar;
		    returnZero = 1;
		    *w = '\0';			// inserting null at the end of the char string
		    return lenWord;
		}

		else {				// Word started with '$', output neg # 
		    prevLen = lenWord;
		    lenWord *= -1;
		    negNumber = 0;
		    prevChar = iochar;
		    returnZero = 1;
		    *w = '\0';			// inserting null at the end of the char string
		    return lenWord;
		}	
	    }
	}

	else if ( (iochar == 32 || iochar == 9) && prevChar != 92 ) {  // checking for TAB or Space chars
	
	    forgotLetter = 0;  // There can't be an overlooked letter when prevChar is a Space or TAB

	    if ( lenWord == 0 ) {		// If no word, then ignore leading space

		if ( prevChar == 36 ) {		// if prevChar was a '$', then return 0
		    prevChar = iochar;
		    return 0;
		}
		continue;
	    }

	    else if ( negNumber == 0 ) {	 // Word didn't start with '$', output pos #
		prevLen = lenWord;
		prevChar = iochar;
		*w = '\0';		// inserting null at the end of the char string
		return lenWord;
	    }

	    else {				// Word started with '$', output negative #
		prevLen = lenWord;
		lenWord *= -1;
		negNumber = 0;
		prevChar = iochar;
		*w = '\0';		// inserting null at the end of the char string
		return lenWord;
	    }
	}

	else {

	    if ( iochar == 36 && negNumber == 0 && lenWord == 0 ) { // checking for first '$'
		negNumber = 1;
		prevChar = iochar;
		continue;
	    }

	    // encountered '~' without a backlash or '$' before it, we retrieve home directory
	    else if ( iochar == 126 && lenWord == 0 && (prevChar != 36 && prevChar != 92) ) {
		
		char *home = getenv("HOME");  // Storing home directory in home variable
		int sizeHome = strlen(home);  //total number of chars in home directory
		prevChar = iochar;
		strncpy(w, home, strlen(home));
		w += sizeHome;
		lenWord = sizeHome;
		continue;
	    }

	    // checking if we missed the '$' by returning a 0 from a new line
	    else if ( prevChar == 36 && negNumber == 0 && lenWord == 0 ) {
		negNumber = 1;
		prevChar = iochar;
		lenWord++;
		*w = iochar;
		w++;
		continue;
	    }

	    // If the previous char is a backslash and the current char is a metacharacter, then ignore backslash
	    else if ( prevChar == 92 && ( iochar == 62 || iochar == 60 || iochar == 36 || iochar == 38 || iochar == 126 || iochar == 32 || iochar == 124) ) {
		*w-- = '\0';
		prevChar = iochar;
		*w = iochar;
		w++;
		meta = 0;
		continue;
	    }

	    // checking for first '<' for the sequence of '<<'
	    else if ( iochar == 60 && prevChar != 92 && lenWord == 0 && meta == 0) {
		lenWord++;
		prevChar = iochar;
		meta = 1;
		continue;
	    }

	    /* If forgotLetter is set to 1 and previous char is a Space or TAB, 
	       then we didn't miss the first letter of new word */
	    else if ( forgotLetter == 1 && (prevChar == 32 || prevChar == 9) ) {
		forgotLetter = 0;
		lenWord++;
		prevChar = iochar;
		*w = iochar;
		w++;
		continue;
	    }

	    // checking if we missed the first letter of the new word
	    else if ( forgotLetter == 1 ) {
		forgotLetter = 0;
		lenWord += 2;
		*w = prevChar;
		w++;
		prevChar = iochar;
		*w = iochar;
		w++;
		continue;
	    }

	    else {		  // Handling char as a new valid char
		forgotLetter = 0;
		prevChar = iochar;
		lenWord++;
		*w = iochar;
                w++;
	    	continue;
	    }
	}

    }

/* Reached the EOF char */

    if ( lenWord > 0 ) {	// We still have to return a word
	
	if ( negNumber == 0 ) {         // Word didn't start with '$', output pos #
            prevChar = iochar;
	    *w = '\0';		// inserting null at the end of the char string
            return lenWord;
        }

        else {                          // Word started with '$', output neg #
            lenWord *= -1;
            negNumber = 0;
            prevChar = iochar;
	    *w = '\0';		// inserting null at the end of the char string
            return lenWord;
        }
    }

    else if ( prevChar == 36 ) {	// last char was '$', return 0
	prevChar = iochar;
	return 0;
    }

    else if ( prevLen != 0 && (prevChar == 10 || prevChar == 32) ) {
	prevChar = 0;
	prevLen = 0;
	return 0;
    }

    else {
	return -255;
    }
}
