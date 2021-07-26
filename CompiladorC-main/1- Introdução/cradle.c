#include "cradle.h"
#include <stdio.h>
#include <stdlib.h>


// Read New Character From Input Stream
void GetChar() {
    Look = getchar();
}

// Report an Error
void Error(char *s) {
    printf("\nError: %s.", s);
}

// Report Error and Halt
void Abort(char *s) {
    Error(s);
    exit(1);
}

// Report What Was Expected
void Expected(char *s) {
    sprintf(tmp, "%s Expected", s);
    Abort(tmp);
}

// Match a Specific Input Character
void Match(char x) {
    if(Look == x) {
        GetChar();
    } else {
        sprintf(tmp, "' %c ' ",  x);
        Expected(tmp);
    }
}

// Recognize an Alpha Character
int IsAlpha(char c) {
    return (UPCASE(c) >= 'A') && (UPCASE(c) <= 'Z');
} 

// Recognize a Digit
int IsDigit(char c) {
    return (c >= '0') && (c <= '9');
}

// Get an Identifier
char GetName() {
    char c = Look;

    if( !IsAlpha(Look)) {
        sprintf(tmp, "Name");
        Expected(tmp);
    }

    GetChar();

    return UPCASE(c);
}

// Get a Number
char GetNum() {
    char c = Look;

    if( !IsDigit(Look)) {
        sprintf(tmp, "Integer");
        Expected(tmp);
    }

    GetChar();

    return c;
}

// Output a String with Tab
void Emit(char *s) {
    printf("\t%s", s);
}

// Output a String with Tab and CRLF
void EmitLn(char *s) {
    Emit(s);
    printf("\n");
}

// Initialize
void Init() {
    GetChar();
}
