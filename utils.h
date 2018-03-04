/**
* Made by © 2018 Almog Hamdani
* utils.h
*
* This is the header file of the utils.c
**/

/**
* This function converts a value to base 32 that is composed of 2 characters in the 'weird' 32 base.
* Parameters: The string to store it in and the value to be converted.
* Return value: The string.
*/
char *convertToBase32(char *, unsigned int);

/**
* This function updates all the data symbols in the symbol list with a specific offset
* Parameters: The offset.
* Return value: None.
*/
void updateDataSymbolsWithOffset(int);

/**
* This is checking if a certain symbol exists.
* Parameters: The symbol.
* Return value: The symbol from the symbol list if found and false otherwise.
*/
symbol_t *checkIfSymbolExists(symbol_t);

/**
* This is printing an error in a specific line to stderr.
* Parameters: The line and the format of the message with optional parameters after it that will be used in the format.
* Return value: None.
*/
void printError(line_t *, char *, ...);

/**
* This is checking if a character is a blank.
* Parameters: The character to be checked.
* Return value: True if it blank and false otherwise.
*/
bool isblank(char);

/**
* This function checks if an externals file is needed.
* Parameters: None.
* Return value: True if an externals file is needed or false otherwise.
*/
bool checkIfExternalsFileIsNeeded();

/**
* This function checks if an entries file is needed.
* Parameters: None.
* Return value: True if an entries file is needed or false otherwise.
*/
bool checkIfEntriesFileIsNeeded();
