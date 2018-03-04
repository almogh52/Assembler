/**
* Made by © 2018 Almog Hamdani
* utils.c
*
* This file is incharge of all the utils functions.
* It does the conversion to base 32.
* It prints the errors in a specific format.
**/

#include "types.h"
#include "linked_list.h"
#include "utils.h"
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>

#define BASE_32 32

const char base32[32] = {'!', '@', '#', '$', '%', '^', '&', '*', '<', '>', 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v'};

/**
* This is printing an error in a specific line to stderr.
* Parameters: The line and the format of the message with optional parameters after it that will be used in the format.
* Return value: None.
*/
void printError(line_t *line, char *f, ...)
{
  va_list arg; /* This var will store the extra arguments */
  char format[MAX_ERROR_LENGTH];

  sprintf(format, "ERROR: %s:%d: %s\n", line->fileName, line->lineNumber, f);
  line->error = true; /* Set line error to true */

  va_start(arg, f); /* Init the args' pointer */
  vfprintf(stderr, format, arg); /* Format error and print to stderr using vfprintf */
  va_end(arg); /* Release the pointer */
}

/**
* This function updates all the data symbols in the symbol list with a specific offset
* Parameters: The offset.
* Return value: None.
*/
void updateDataSymbolsWithOffset(int offset)
{
  symbol_node_t *ptr = symbolHead;

  /* While the item exists, the list didn't end */
  while (ptr)
  {
    if (ptr->data.type == data || ptr->data.type == strct || ptr->data.type == string) /* If the item is data(numbers, struct or string) */
      ptr->data.pointer += offset; /* Add the offset to the item' pointer */

    ptr = ptr->next; /* Point ptr to the next item in the list */
  }
}

/**
* This is checking if a certain symbol exists.
* Parameters: The symbol.
* Return value: The symbol from the symbol list if found and false otherwise.
*/
symbol_t *checkIfSymbolExists(symbol_t symbol)
{
  symbol_node_t *ptr = symbolHead;

  /* While the item exists, the list didn't end */
  while (ptr)
  {
    if (strcmp(ptr->data.name, symbol.name) == 0) /* If the item's symbol is equal to the new one return true */
      return &ptr->data;

    ptr = ptr->next; /* Point ptr to the next item in the list */
  }

  return NULL;
}

/**
* This is checking if a character is a blank.
* Parameters: The character to be checked.
* Return value: True if it blank and false otherwise.
*/
bool isblank(char ch)
{
  return ch == ' ' || ch == '\t' ? true : false;
}

/**
* This function converts a value to base 32 that is composed of 2 characters in the 'weird' 32 base.
* Parameters: The string to store it in and the value to be converted.
* Return value: The string.
*/
char *convertToBase32(char *result, unsigned int value)
{
  int i;

  /* At start set !! as the result */
  result[0] = '!';
  result[1] = '!';
  result[2] = 0;

  for (i = 1; i >= 0 && value; i--) /* Going through the result str */
  {
    result[i] = base32[value % BASE_32]; /* Setting first char */
    value /= BASE_32; /* Dividing in 32 to get the second char */
  }

  return result;
}

/**
* This function checks if an externals file is needed.
* Parameters: None.
* Return value: True if an externals file is needed or false otherwise.
*/
bool checkIfExternalsFileIsNeeded()
{
  bool found = false;
  line_node_t *linePtr;

  linePtr = lineHead; /* Set to point to list' head */

  /* Go through the list and check if external is found, if found break and return */
  while (linePtr && !found)
  {
    if (linePtr->data.type == instruction)
      if (linePtr->data.lineData.instruction.srcOp.data.symbol.type == ext || linePtr->data.lineData.instruction.dstOp.data.symbol.type == ext) /* If source or destination operand is external */
        found = true; /* Set found to true */

    linePtr = linePtr->next; /* Move line pointer to the next item */
  }

  return found;
}

/**
* This function checks if an entries file is needed.
* Parameters: None.
* Return value: True if an entries file is needed or false otherwise.
*/
bool checkIfEntriesFileIsNeeded()
{
  bool found = false;
  symbol_node_t *symbolPtr;

  symbolPtr = symbolHead; /* Set to point to list' head */

  /* Go through the list and check if entry is found, if found break and return */
  while (symbolPtr && !found)
  {
    if (symbolPtr->data.type == ent) /* If an entry was found */
      found = true; /* Set found to true */

    symbolPtr = symbolPtr->next; /* Move symbol pointer to the next item */
  }

  return found;
}
