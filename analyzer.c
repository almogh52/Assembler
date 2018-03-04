/**
* Made by © 2018 Almog Hamdani
* analyzer.c
*
* This file is incharge of all the analyze functions, and the first pass.
* It takes each line in the input file and format it into the line type which will be later used to encode the line.
**/

#include "types.h"
#include "analyzer.h"
#include "utils.h"
#include "encoder.h"
#include "linked_list.h"
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <math.h>

void checkForLabel(line_t *);
bool checkIfEmpty(char[]);
void checkIsLabelValid(line_t *);
bool stringIsValidInt(line_t *, char [], int);
void analyzeString(line_t *, char []);
void analyzeInstruction(line_t *, int, char [], char []);
operand_t analyzeOperand(line_t *, char []);
register_t isRegister(char *);
bool checkIsSymbolValid(line_t *, char *);
void printBits(size_t const size, void const * const ptr);

/**
* This function analyzes a line in the file and formats it into the line type.
* Parameters: The pointer to the line type.
* Return value: None.
*/
void analyzeLine(line_t *line)
{
  char *tok; /* This var will hold the current token(part of a string) that we are checking */
  int i, j, temp;
  char values[MAX_VALUES][MAX_VALUE_LENGTH] = {{0}};

  if (line->input[0] == ';') /* If the first char in line after blanks is ; it means we have a comment and we need to skip this line */
  {
    line->type = nothing; /* Set type to empty */
    return;
  }

  checkForLabel(line); /* This will search for a label and set it */

  if (checkIfEmpty(line->input)) /* Check if line is empty */
  {
    if (line->symbol.name[0] == 0) /* If label is empty, it means we have an empty line */
    {
      line->type = nothing; /* Set type to empty */
      return;
    } else
      printError(line, "Line have only a label without data!");
      return;
  }

  tok = strtok(line->input, " \t"); /* Get token(token is a string seperated by space or tab) */

  if (tok[0] == '.') /* It means we have a directive sentence */
  {
    line->symbol.pointer = dataCnt; /* Set the label's value to DC, current data line */

    if (strcmp(tok, ".data") == 0) /* If it's a data */
    {
      line->type = data; /* Set type */
    } else if (strcmp(tok, ".string") == 0) /* If it's a string */
    {
      line->type = string; /* Set type */
    } else if (strcmp(tok, ".struct") == 0) /* If it's a struct */
    {
      line->type = strct; /* Set type */
    } else if (strcmp(tok, ".entry") == 0) /* If it's an entry */
    {
      line->type = ent; /* Set type */
    } else if (strcmp(tok, ".extern") == 0) /* If it's an external */
    {
      line->type = ext; /* Set type */
    } else { /* Unknown directive */
      printError(line, "Unknown directive \"%s\"", tok); /* Print error */
      line->type = unknown; /* Set type */
    }
  } else { /* It means we have an instruction sentence */
    line->symbol.pointer = instructionCnt; /* Set the label's value to IC, current instruction line */

    line->instruction = NULL; /* Set instruction to be nothing */

    /* Check instruction name */
    for (i = 0; i < AMOUNT_OF_INSTRUCTIONS; i++)
      if (strcmp(tok, instructions[i].name) == 0) /* If instruction found */
        line->instruction = &(instructions[i]);

    if (line->instruction == NULL) /* If instruction not found */
    {
      printError(line, "Unknown instruction \"%s\"", tok); /* Print error */
      line->type = unknown; /* Set type */
    } else {
      line->type = instruction; /* Set type */
    }
  }

  line->symbol.type = line->type; /* Set symbol' type as the line' type */

  if (!checkIfEmpty(line->symbol.name) && line->type != ext && line->type != ent && line->type != unknown) /* If a label was found (not empty) and the line isn't an extern, entry or unknown, add it to the symbol list */
  {
    if (checkIfSymbolExists(line->symbol)) /* If symbol already exists print error */
      printError(line, "Duplicate symbol \"%s\"! Symbol must be uniqe", line->symbol.name);
    else
      addSymbolToList(line->symbol);
  }

  if (line->type == string) /* If the line type is string */
  {
    tok = strtok(NULL, ""); /* Get the rest of the line */

    if (tok == NULL || checkIfEmpty(tok)) /* No string */
    {
      printError(line, "Missing string after \".string\" directive!");
      return;
    }
    analyzeString(line, tok);
  } else if (line->type == strct) /* If the line type is struct */
  {
    tok = strtok(NULL, ","); /* Get the next token, should be integer */

    if (tok == NULL) /* No values for the struct */
    {
      printError(line, "Missing struct values after \".struct\" directive!");
      return;
    }

    temp = -1; /* Set for missing comma check */
    sscanf(tok, "%*s %*s%n", &temp); /* Check for a pattern, if there is a string, a blank and then another string it means we have a possible missing comma */
    if(temp != -1) /* If temp isn't -1(Start value), it means the pattern is found */
    {
      printError(line, "Missing comma between struct's values \"%s\"", tok); /* Print error */
      return;
    }

    if (stringIsValidInt(line, tok, DATA_INTEGER_BITS)) /* Token is a valid int */
      line->lineData.strct.num = atoi(tok);

    tok = strtok(NULL, ","); /* Get the rest of the line or if another param was found, should be string */

    if (tok == NULL)
    {
      printError(line, "Missing string value for struct!");
      return;
    } else if (checkIfEmpty(tok))
    {
      printError(line, "String value cannot be empty! Struct values must be [integer, string]");
      return;
    }
    analyzeString(line, tok);

    tok = strtok(NULL, ""); /* Get the rest of the line, if there is more params */
    if (tok != NULL) /* If another parameter was found, send an error */
    {
      printError(line, "Too much values for sturct! Struct values must be [integer, string]");
      return;
    }

  } else { /* The rest of the types */

    if (!checkIfEmpty(tok + strlen(tok) + 1)) /* Check if the rest of the line isn't empty */
    {
      /* Get all operands into the operands array */
      for (i = 0; i < MAX_VALUES && (tok = strtok(NULL, ",")) != NULL; i++) /* Get a new token */
        strcpy(values[i], tok); /* Copy the operand */
    } else {
      i = 0; /* Set values to 0 if the rest is empty */
    }

    switch(line->type)
    {
      case instruction:
        if (values[0] == 0 && values[1] == 0)
          analyzeInstruction(line, i, "", "");
        else if (values[1][0] == 0) /* If second opernd is null, send first operand as destination operand */
          analyzeInstruction(line, i, "", values[0]);
        else
          analyzeInstruction(line, i, values[0], values[1]);

        if (!line->error) /* If the line doesn't have error, calc it's size and add it to IC */
          addInstructionSize(line);
        break;

      case data:
        if (i >= MAX_VALUES) /* Too many data values */
          printError(line, "Too many data values in line"); /* Print error */
        else if (i == 0) /* No values are entered */
          printError(line, "No values entered for data directive"); /* Print error */

        line->lineData.data.count = i; /* Set the amount of numbers */
        for (j = 0; j < i; j++) /* Going through all the the operands found */
        {
          temp = -1; /* Set for missing comma check */
          sscanf(values[j], "%*s %*s%n", &temp); /* Check for a pattern, if there is a string, a blank and then another string it means we have a possible missing comma */
          if(temp != -1) /* If temp isn't -1(Start value), it means the pattern is found */
            printError(line, "Missing comma between values \"%s\"", values[j]); /* Print error */
          else if (stringIsValidInt(line, values[j], DATA_INTEGER_BITS)) /* If value is a valid integer, set it in the data values */
            line->lineData.data.numbers[j] = atoi(values[j]);
        }
        break;

      case ent:
      case ext:
        strcpy(values[0], strtok(values[0], " \t")); /* Remove spaces */

        if (checkIfEmpty(values[0])) /* No values entered */
        {
          if (line->type == ext) /* Extern */
            printError(line, "Missing label name for extern!");
          else /* Entry */
            printError(line, "Missing label name for entry!");
          return;
        } else if (i > 1) /* Extern\Entry can only have 1 value, Check if there are any other operands in the line except label name */
        {
          if (line->type == ext) /* Extern */
            printError(line, "Extraneous text after label name! Extern must have only 1 value");
          else /* Entry */
            printError(line, "Extraneous text after label name! Entry must have only 1 value");
          return;
        } else if (strtok(NULL, " \t") != NULL) /* Check if there is more text after label name */
        {
          if (line->type == ext) /* Extern */
            printError(line, "Extraneous text after label name!");
          else /* Entry */
            printError(line, "Extraneous text after label name!");
          return;
        }

        line->lineData.extent.symbol.pointer = 0;
        strcpy(line->lineData.extent.symbol.name, values[0]); /* Set symbol name */
        line->lineData.extent.symbol.type = line->type; /* Set symbol' type as the line' type, extern or entry */

        if (line->type == ext) /* If the line is extern add it to the symbol list */
          addSymbolToList(line->lineData.extent.symbol);

      default:
        break;
    }
  }

  if (line->type == data || line->type == string || line->type == strct) /* If the line is a data line, encode it */
    encodeData(line);
}

/**
* This function analyzes a string that should be a parameter, it checks its valid and inform of errors to the user.
* Parameters: The pointer to the line type and the string.
* Return value: None.
*/
void analyzeString(line_t *line, char str[])
{
  int i, j = 0;
  bool stringStarted = false, stringEnded = false;

  /* Going through the entire string */
  for (i = 0; str[i]; i++)
  {
    if (!stringStarted && str[i] == '\"') /* If the start of the string is found */
    {
      stringStarted = true;
    } else if (stringStarted && str[i] == '\"' && !stringEnded) /* If the end of the string is found and the string isn't finished yet */
    {
      stringEnded = true;
    } else if (!isblank(str[i]) && stringEnded == true) /* If the string is done and we found a char that is not blank it means we have an error */
    {
      if (line->type == string) /* If line type is string */
        printError(line, "Invalid characters after string \"%s\"", line->lineData.string.str);
      else /* If line type is struct */
        printError(line, "Invalid characters after string \"%s\"", line->lineData.strct.str);
      return;
    } else if (!isblank(str[i]) && !stringStarted) /* If string hasn't found yet and characters are found */
    {
      printError(line, "Invalid characters before string. The string must start with a \" and end with \"!");
      return;
    } else if (stringStarted && !stringEnded) { /* If we in the string, copy the char to it's place */
      if (line->type == string) /* If line type is string */
        line->lineData.string.str[j++] = str[i];
      else /* If line type is struct */
        line->lineData.strct.str[j++] = str[i];
    }
  }

  /* End string */
  if (line->type == string) /* If line type is string */
    line->lineData.string.str[j] = 0;
  else /* If line type is struct */
    line->lineData.strct.str[j] = 0;
}

/**
* This function analyzes an instruction, it checks if the operand are valid and fit to the operation and inform of errors.
* Parameters: The pointer to the line type, the amount of ops received, first operand and second operand as strings.
* Return value: None.
*/
void analyzeInstruction(line_t *line, int ops, char op1[], char op2[])
{
  char temp[MAX_VALUE_LENGTH];
  char *tok;

  /* First set both operands to empty before the check */
  line->lineData.instruction.srcOp.type = empty;
  line->lineData.instruction.dstOp.type = empty;

  /* -------------- */
  /* Soruce Operand */
  /* -------------- */

  if (op1[0] != 0 && checkIfEmpty(op1)) /* If operand isn't null but empty */
    printError(line, "Source operand cannot be empty!");
  else if (op1[0] != 0) /* If operand isn't null */
  {

    strcpy(temp, op1); /* Copy operand to temp string to use it instead of the original value */

    tok = strtok(temp, " \t"); /* Get a token seprated by blanks, this is to check if there is only 1 word in the operand, should be only 1 */

    if (strtok(NULL, " \t") != NULL) /* Trying to get another token, if found the function shouldn't return null, if found it means we have 2 words, error */
      printError(line, "Invalid operand \"%s\"! Might be a missing comma", op1);
    else {
      line->lineData.instruction.srcOp = analyzeOperand(line, tok); /* Set the operand using the analyze operand method */

      /* Check if addressing mode fits the instruction' properties */
      switch(line->lineData.instruction.srcOp.type)
      {
        case reg: /* The operand is a register */
          if (line->instruction->srcAddressingMode.registerAddress != true) /* Check if the address mode is false in the instruction */
            printError(line, "The instruction \"%s\" doesn't accept source operand, \"%s\", to be a register!", line->instruction->name, tok);

          break;

        case symbol: /* The operand is a symbol */
          if (line->instruction->srcAddressingMode.directAddress != true) /* Check if the address mode is false in the instruction */
            printError(line, "The instruction \"%s\" doesn't accept source operand, \"%s\", to be a symbol!", line->instruction->name, tok);

          break;

        case structField: /* The operand is a struct */
          if (line->instruction->srcAddressingMode.structAddress != true) /* Check if the address mode is false in the instruction */
            printError(line, "The instruction \"%s\" doesn't accept source operand, \"%s\", to be a struct!", line->instruction->name, tok);

          break;

        case number: /* The operand is a number */
          if (line->instruction->srcAddressingMode.immediateAddress != true) /* Check if the address mode is false in the instruction */
            printError(line, "The instruction \"%s\" doesn't accept source operand, \"%s\", to be an integer!", line->instruction->name, tok);

          break;

        default:
          break;
      }

    }
  }

  /* ------------------- */
  /* Destination Operand */
  /* ------------------- */

  if (op2[0] != 0 && checkIfEmpty(op2)) /* If operand isn't null but empty */
    printError(line, "Source operand cannot be empty!");
  else if (op2[0] != 0) /* If operand isn't null */
  {
    strcpy(temp, op2); /* Copy operand to temp string to use it instead of the original value */

    tok = strtok(temp, " \t"); /* Get a token seprated by blanks, this is to check if there is only 1 word in the operand, should be only 1 */

    if (strtok(NULL, " \t") != NULL) /* Trying to get another token, if found the function shouldn't return null, if found it means we have 2 words, error */
      printError(line, "Invalid operand \"%s\"! Might be a missing comma", op2);
    else {
      line->lineData.instruction.dstOp = analyzeOperand(line, tok); /* Set the operand using the analyze operand method */

      /* Check if addressing mode fits the instruction' properties */
      switch(line->lineData.instruction.dstOp.type)
      {
        case reg: /* The operand is a register */
          if (line->instruction->dstAddressingMode.registerAddress != true) /* Check if the address mode is false in the instruction */
            printError(line, "The instruction \"%s\" doesn't accept destination operand, \"%s\", to be a register!", line->instruction->name, tok);

          break;

        case symbol: /* The operand is a symbol */
          if (line->instruction->dstAddressingMode.directAddress != true) /* Check if the address mode is false in the instruction */
            printError(line, "The instruction \"%s\" doesn't accept destination operand, \"%s\", to be a symbol!", line->instruction->name, tok);

          break;

        case structField: /* The operand is a struct */
          if (line->instruction->dstAddressingMode.structAddress != true) /* Check if the address mode is false in the instruction */
            printError(line, "The instruction \"%s\" doesn't accept destination operand, \"%s\", to be a struct!", line->instruction->name, tok);

          break;

        case number: /* The operand is a number */
          if (line->instruction->dstAddressingMode.immediateAddress != true) /* Check if the address mode is false in the instruction */
            printError(line, "The instruction \"%s\" doesn't accept destination operand, \"%s\", to be an integer!", line->instruction->name, tok);

          break;

        default:
          break;
      }
    }
  }

  /* Amount of operands check */
  if (line->instruction->ops > ops) /* If we got a fewer operands than needed */
      printError(line, "Missing operands! The instruction \"%s\" must have %d operands", line->instruction->name, line->instruction->ops);
  else if (line->instruction->ops < ops) /* If we got too many operands than needed */
  {
    if (line->instruction->ops > 0)
      printError(line, "Too many operands! The instruction \"%s\" must have %d operands", line->instruction->name, line->instruction->ops);
    else
      printError(line, "Too many operands! The instruction \"%s\" must have no operands", line->instruction->name);
  }
}

/**
* This function analyzes an operand and finds it's type.
* Parameters: The pointer to the line type and the operand' string.
* Return value: The operand formatted (operand type).
*/
operand_t analyzeOperand(line_t *line, char *operand)
{
  operand_t op;
  char temp[MAX_VALUE_LENGTH];
  char *tok;

  strcpy(temp, operand); /* Copy operand to temp string to use it instead of the original value */

  if (operand[0] == '#') /* It means we have a possible number */
  {
    if (stringIsValidInt(line, operand + 1, INSTRUCTION_INTEGER_BITS)) /* Check if the number that is after the # is a valid int */
      op.data.number = atoi(operand + 1); /* Set the operand in the line data using atoi(string to int) */

    op.type = number; /* Set type */
  } else if((op.data.reg = isRegister(operand)) != -1) { /* If the function returns -1 it means no register found else it returns the register */
    op.type = reg; /* Set type */
  } else if (strcmp(tok = strtok(temp, "."), operand)) /* Try to split string using . delimeter, if successful the given token should be different of the original operand(temp), if all true it means we have a struct */
  {
    if (checkIsSymbolValid(line, tok)) /* Check if the symbol is valid */
      strcpy(op.data.symbol.name, tok); /* Copy the symbol */
    else /* Print error */
      printError(line, "Invalid symbol \"%s\"!", temp);

    tok = strtok(NULL, "."); /* Get a new token, should be struct field */
    if (strcmp(tok, "1") == 0 || strcmp(tok, "2") == 0) /* Check if the field is 1 or 2 or either */
      op.data.field = atoi(tok); /* Set the token as the struct field */
    else /* Invalid field */
      printError(line, "Invalid struct field \"%s\"! Should be 1 or 2", tok); /* Print error */

    tok = strtok(NULL, "."); /* Try get a new token */
    if (tok != NULL) /* If another field was found, illegel, print error */
      printError(line, "Struct should have only 1 field! Usage: [Struct label].[Struct field]");

    op.type = structField; /* Set type */
  } else { /* If nothing was found, it means we probably have a symbol */
    if (checkIsSymbolValid(line, temp)) /* Check if the symbol is valid */
      strcpy(op.data.symbol.name, temp); /* Copy the symbol */
    else /* Print error */
      printError(line, "Invalid symbol \"%s\"!", temp);

    op.type = symbol; /* Set type */
  }
  return op;
}

/**
* This function checks if the string is a valid register.
* Parameters: The string to be checked.
* Return value: The register type that specifices which register is it or -1 if not found.
*/
register_t isRegister(char *operand)
{
  /* Check if is a valid register */
  if (strcmp(operand, "r0") == 0)
    return r0;
  else if (strcmp(operand, "r1") == 0)
    return r1;
  else if (strcmp(operand, "r2") == 0)
    return r2;
  else if (strcmp(operand, "r3") == 0)
    return r3;
  else if (strcmp(operand, "r4") == 0)
    return r4;
  else if (strcmp(operand, "r5") == 0)
    return r5;
  else if (strcmp(operand, "r6") == 0)
    return r6;
  else if (strcmp(operand, "r7") == 0)
    return r7;
  else /* Register not found */
    return -1;
}

/**
* This function checks if a given string is a valid integer and in correct range.
* Parameters: The pointer to the line type, the string and the range in bits.
* Return value: True if it is valid integer and in range or false otherwise.
*/
bool stringIsValidInt(line_t *line, char str[], int rangeBits)
{
  int i;
  bool numFound = false, numFinished = false;

  if (checkIfEmpty(str)) /* If string is empty */
  {
    printError(line, "Data value cannot be empty!");
    return false;
  }

  /* Going through the entire string */
  for (i = 0; str[i]; i++)
  {
    if (!isblank(str[i])) /* If current char isn't blank */
    {
      if ((!isdigit(str[i]) && str[i] != '+' && str[i] != '-') || numFinished == true) /* If the char isn't a digit, plus and minus or the num is already finished */
      {
        printError(line, "Invalid number \"%s\". The number must be a valid integer", str);
        return false;
      } else if (numFound == false) /* If number hasn't started yet and now we found the first digit */
        numFound = true;
    } else if (numFound == true && numFinished != true) /* If number is found but not finished yet and we found a blank so we need to set it to finished */
      numFinished = true;
  }

  if (atoi(str) < -pow(2, rangeBits - 1) || atoi(str) > pow(2, rangeBits - 1) - 1) /* Out of range integer */
  {
    printError(line, "Invalid number \"%s\". The number must in range of %.2f to %.2f", str, -pow(2, rangeBits - 1), pow(2, rangeBits - 1) - 1);
    return false;
  }

  return true;
}

/**
* This function checks for a label in the line. It would set it in the line pointer and remove it from the input.
* Parameters: The pointer to the line type.
* Return value: None.
*/
void checkForLabel(line_t *line)
{
  char label[MAX_LINE_LENGTH]; /* Max label legnth even if it is over the max of the valid label length */
  int n = 0; /* This will be used to determine if a : was found between the "label" and the rest of the line */
  int i;

  sscanf(line->input, " %[^: \t] :%n", label, &n); /* Using sscanf, it's scans the string for a string and then : or space or tab, after it the var 'n' should have the amount of chars read so far and not 0 if there was a : after the label */

  if (n != 0) /* A label was found because there was a : after the first string in the line */
  {
    if (isblank(*(strchr(line->input, ':') - 1))) /* Check if the char before the : is a blank, it means we need to inform on an error */
    {
      printError(line, "No blanks are allowed between label name and \':\'");
    }

    strcpy(line->symbol.name, label); /* Copy the label to it's location */
    checkIsLabelValid(line); /* Check if the label is valid and print an error if not */

    /* Set all label text to blanks since it was already analyzed */
    for (i = 0; line->input + i <= strchr(line->input, ':'); i++)
      line->input[i] = ' ';
  }
}

/**
* This function checks if a given string is a blank string.
* Parameters: The string.
* Return value: True if it is blank or false otherwise.
*/
bool checkIfEmpty(char str[])
{
  int i = 0;

  for (i = 0; str[i]; i++) /* Go through all the string */
  {
    if (!isblank(str[i]) && str[i] != '\n') /* If the current checked character isn't a blank or a new line, it means the string isn't empty */
    {
      return false;
    }
  }

  return true;
}

/**
* This function checks if a symbol is valid without printing errors
* Parameters: The pointer to the line type and the string that contains the symbol.
* Return value: True if the symbol is valid and false otherwise.
*/
bool checkIsSymbolValid(line_t *line, char *symbol)
{
  int i;

  if (!isalpha(symbol[0]) || strlen(symbol) > MAX_LABEL_LENGTH) /* If the first char in the symbol isn't a letter or if the length of the label is over the max length of it */
    return false;

  for (i = 0; symbol[i]; i++)
    if (!isalnum(symbol[i])) /* If the current char we are checking isn't alphanumberic, all chars must be alphanumberic */
      return false;

  return true;
}

/**
* This function checks if the label in the line is valid and printing errors if needed.
* Parameters: The pointer to the line type.
* Return value: True if the label is valid and false otherwise.
*/
void checkIsLabelValid(line_t *line)
{
  int i;

  if (!isalpha(line->symbol.name[0])) /* If the first char in the label isn't a letter */
  {
    printError(line, "The label \"%s\" must start with an alphabetic letter", line->symbol.name); /* Print error */
  }

  if (strlen(line->symbol.name) > MAX_LABEL_LENGTH) /* If the length of the label is over the max length of it */
  {
    printError(line, "The label \"%s\" must have max length of %d characters", line->symbol.name, MAX_LABEL_LENGTH); /* Print error */
  }

  for (i = 0; line->symbol.name[i]; i++)
  {
    if (!isalnum(line->symbol.name[i])) /* If the current char we are checking isn't alphanumberic, all chars must be alphanumberic */
    {
      printError(line, "The label \"%s\" contains characters that aren't alphanumberic", line->symbol.name); /* Print error */
      break; /* Exit loop */
    }
  }
}
