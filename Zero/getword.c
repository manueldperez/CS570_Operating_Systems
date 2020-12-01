/*  
 *  getword.c
 *
 *  Student Name: Manuel Perez
 *
 *  Instructor Name: John Carroll
 *
 *  Class: CS570
 *
 *  Due Date: 9/1/20
 *
 *  Synopsis - This is a lexical analyzer, it takes a string of chars as input 
 *             from the p0.c and returns word lengths as well as word strings
 *             back to p0.c.
 */

/* Include Files */
#include <stdio.h>

int getword(char *w)
{
    int iochar = 0;	// Current char
    int prevChar;
    int lenWord = 0;
    int charCount = 0;
    int negNumber = 0;		// 1 if we encountered '$'
    int prevLen;
    *w = '\0';

    while ( ( iochar = getchar() ) != EOF ) {

	if ( prevChar == 10 ) {			// If the last char is a new line, return 0  
	    prevChar = iochar;
	    return 0;
	}

	else if ( iochar == 10 )		// checking for new line
	{
	    if ( lenWord  == 0 ) {		// There's no word, output 0
		prevChar = iochar;
		return 0;
	    }
	    else if ( prevChar == 92 ) {	// Is the prev char a '\'
		lenWord--;
		prevChar = 32;
		*w--;				
		*w-- = '\0';			// Removing the '\' from the string of chars
		if ( negNumber == 0 ) {		// Word didn't start with '$', output pos #
		    charCount = lenWord;
		    lenWord = 0;
		    return charCount;
		}
		else {				// Word started with '$', output neg #
		    charCount = lenWord * -1;
		    lenWord = 0;
		    negNumber = 0;
		    return charCount;
		}
	    }
	    else {		// Word didn't start with '$'
		if ( negNumber == 0 ) {		// Word didn't start with '$', output pos #
		    charCount = lenWord;
		    lenWord = 0;
		    prevChar = iochar;
		    prevLen = lenWord;
		    *w = '\0';			// inserting null at the end of the string of chars
		    return charCount;
		}
		else {				// Word started with '$', output neg #
		    charCount = lenWord * -1;
		    lenWord = 0;
		    negNumber = 0;
		    prevChar = iochar;
		    prevLen = charCount;
		    *w = '\0';			// inserting null at the end of the string of chars
		    return charCount;
		}	
	    }
	}

	else if ( iochar == 32 || iochar == 9 )	// checking for TAB or Space chars
	{
	    if ( lenWord == 0 ) {		// If no word, then ignore leading space
		if ( prevChar == 36 ) {		// if prevChar was a '$', then return 0
		    prevChar = iochar;
		    return 0;
		}
		continue;
	    }
	    else if ( negNumber == 0 ) {	 // Word didn't start with '$', output pos #
		charCount = lenWord;
		lenWord = 0;
		prevChar = iochar;
		*w = '\0';		// inserting null at the end of the string of chars
		return charCount;
	    }
	    else {				// Word started with '$', output negative #
		charCount = lenWord * -1;
		lenWord = 0;
		negNumber = 0;
		prevChar = iochar;
		*w = '\0';		// inserting null at the end of the string of chars
		return charCount;
	    }
	}

	else 
	{
	    if ( iochar == 36 && negNumber == 0 && lenWord == 0 ) { // checking for first '$'
		negNumber = 1;
		prevChar = iochar;
		continue;
	    }
	    else if ( prevChar == 36 && negNumber == 0 && lenWord == 0 ) { //checking if we missed the '$' by returning a 0 from a new line
		negNumber = 1;
		prevChar = iochar;
		lenWord++;
		*w = iochar;
		w++;
		continue;
	    }
	    else if ( prevChar == iochar && prevChar != 36 ) { // checking for a repeat char
		continue;
	    }
	    else {			// Handling char as a new valid char
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
            charCount = lenWord;
            lenWord = 0;
            prevChar = iochar;
	    *w = '\0';		// inserting null at the end of the string of chars
            return charCount;
        }
        else {                          // Word started with '$', output neg #
            charCount = lenWord * -1;
            lenWord = 0;
            negNumber = 0;
            prevChar = iochar;
	    *w = '\0';		// inserting null at the end of the string of chars
            return charCount;
        }
    }

    else if ( prevChar == 36 ) {	// last char was '$', return 0
	prevChar = iochar;
	return 0;
    }

    else if ( prevLen != 0 && prevChar == 10) {
	prevChar = 0;
	prevLen = 0;
	return 0;
    }

    else {
	return -255;
    }
}
