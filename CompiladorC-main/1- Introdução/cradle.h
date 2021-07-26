#ifndef _CRADLE_H
#define _CRADLE_H


#define UPCASE(C) (~(1<<5) & (C))

// Constant Declarations
#define MAX_BUF 100

// Variable Declarations
static char tmp[MAX_BUF];
char Look;

// Read New Character From Input Stream
void GetChar();

// Report an Error
void Error(char *s);

// Report Error and Halt
void Abort(char *s);

// Report What Was Expected
void Expected(char *s);

// Match a Specific Input Character
void Match(char x);


// Recognize an Alpha Character
int IsAlpha(char c);

// Recognize a Digit
int IsDigit(char c);


// Get an Identifier
char GetName();

// Get a Number
char GetNum();


// Output a String with Tab
void Emit(char *s);

// Output a String with Tab and CRLF
void EmitLn(char *s);


// Initialize
void Init();

#endif
