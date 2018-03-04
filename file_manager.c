/**
* Made by © 2018 Almog Hamdani
* file_manager.c
*
* This file is incharge of all the file functions.
* It includes the main function which manages the assembler.
* It opens each input file and send it to analyzed and encoded.
* After that, he creates the externals file and entries file if needed.
**/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "types.h"
#include "analyzer.h"
#include "linked_list.h"
#include "second_pass.h"
#include "utils.h"

#define START_OF_PROGRAM_IN_MEMORY 100
#define BASE32_STR_RESULT_LEN 3

bool analyzeFile(char *);
bool createObjectFile(char *);
void createEntriesFile(char *);
void createExternalsFile(char *);

int main (int argc, char *argv[])
{
  int i;

  printf("Assembler (c) 2018 Almog Hamdani.\n\n"); /* Printing introduction */

  if (argc < 2) /* Invalid number of command line arguments */
  {
    fprintf(stderr, "Invalid number of arguments send to the assembler.\nEnter at least one file name to compile\nUsage: Assembler.exe [File 1 name] [File 2 name] ...");
    return 1;
  }

  for (i = 1; i < argc; i++) /* Going through all arguments */
  {
    if(analyzeFile(argv[i])) /* Send the file to be analyzed, if the file was analyzed successfully create files */
    {
      if(createObjectFile(argv[i])) /* Create object file, if the file was created successfully, create entry and extern files */
      {
        if (checkIfEntriesFileIsNeeded()) /* If entries file is needed, create it */
          createEntriesFile(argv[i]);

        if (checkIfExternalsFileIsNeeded()) /* If externals file is needed, create it */
          createExternalsFile(argv[i]);
      }
    }

    /* Free all linked lists */
    FREE_LIST(instruction_node_t, instructionHead)
    FREE_LIST(data_node_t, dataHead)
    FREE_LIST(symbol_node_t, symbolHead)
    FREE_LIST(line_node_t, lineHead)

    /* Reset all list's counters */
    instructionCnt = 0;
    dataCnt = 0;
    lineCnt = 0;
    symbolCnt = 0;
  }

  return 0;
}

/**
* This function analyzes a file and encodes it.
* Parameters: The name of the file to be analyzed and encoded.
* Return value: None.
*/
bool analyzeFile(char *name)
{
  bool error = false;
  FILE *fd;
  int currentLineNumber = 1;
  line_t dummyLine = {0};
  line_t *currentLine = &dummyLine; /* This will store the current line we are analyzing */
  char fileName[MAX_FILE_NAME_LENGTH];
  line_node_t *ptr; /* This will store the current line we are rechecking from the line linked list */

  sprintf(fileName, "%s.as", name); /* Format the file name, add to it .as */

  if (!(fd = fopen(fileName, "r"))) /* Try open the file, if the open failed */
  {
    fprintf(stderr, "Cannot open the file \"%s\". It may be corrupted or not exist!\n", fileName); /* Print error */
    return false;
  }

  printf("Compiling \"%s\":\n", fileName);

  /* ========== */
  /* FIRST PASS */
  /* ========== */

  instructionCnt = START_OF_PROGRAM_IN_MEMORY; /* Program' memory starts from 100 */
  strcpy(currentLine->fileName, fileName); /* Copy the current file name to the line type, will be used in printing errors */

  while (!feof(fd))
  {
    currentLine->lineNumber = currentLineNumber; /* Set line number */

    if(fgets(currentLine->input, MAX_LINE_LENGTH, fd)) /* Get a line from the file and store in currentLine */
    {
      if (currentLine->input[strlen(currentLine->input) - 1] == '\n') /* Check if there is a new line at the end of the string */
          currentLine->input[strlen(currentLine->input) - 1] = ' '; /* Remove new line from the end of the line, it is set as a space for a check issue */

      analyzeLine(currentLine); /* Analyze the line */

      if (currentLine->error == true) /* If an error was found in the line, set error to true */
        error = true;

      if (currentLine->type != nothing) /* If line isn't empty, add it to linked list */
        addLineToList(*currentLine);

      currentLineNumber++; /* Increase current line number for next line */
      strcpy(currentLine->symbol.name, ""); /* Clear label for next line */
    }
  }

  fclose(fd); /* Close file */

  updateDataSymbolsWithOffset(instructionCnt); /* Update all data' symbols with the offset of the end of the instructions */

  if (error == true) /* If an error was found during first pass, print compilation faild */
  {
    fprintf(stderr, "Errors were found in the file \"%s\" during first pass!\nCompilation failed!\n\n", fileName); /* Print error */
    return false;
  }

  /* =========== */
  /* SECOND PASS */
  /* =========== */

  instructionCnt = START_OF_PROGRAM_IN_MEMORY; /* Reset instruction counter */
  ptr = lineHead; /* Point ptr to line linked list */

  /* While the item exists, the list didn't end */
  while (ptr)
  {
    recheckLine(&ptr->data); /* Send the line to be rechecked */

    if (ptr->data.error == true) /* If an error was found in the line, set error to true */
      error = true;

    ptr = ptr->next; /* Point ptr to the next item in the list */
  }

  if (error == true) /* If an error was found during second pass, print compilation faild */
  {
    fprintf(stderr, "Errors were found in the file \"%s\" during second pass!\nCompilation failed!\n\n", fileName); /* Print error */
    return false;
  }

  printf("Successfully compiled \"%s\"!\n\n", fileName);

  return true;
}

/**
* This function creates an object file that will contain the encoded file that was created from the original assembly file.
* Parameters: The name of the wanted file.
* Return value: None.
*/
bool createObjectFile(char *name)
{
  FILE *fd;
  char fileName[MAX_FILE_NAME_LENGTH];
  instruction_node_t *instructionPtr;
  data_node_t *dataPtr;
  int address = START_OF_PROGRAM_IN_MEMORY;
  char *result1, *result2;

  /* Try allocate memory for convertion result */
  if ((result1 = (char *)malloc(BASE32_STR_RESULT_LEN * sizeof(char))) == NULL || (result2 = (char *)malloc(BASE32_STR_RESULT_LEN * sizeof(char))) == NULL)
  {
    fprintf(stderr, "Error allocating memory for base 32 result!\n"); /* Print error */
    exit(0);
  }

  sprintf(fileName, "%s.ob", name); /* Format the new file name, add to it .ob */

  if (!(fd = fopen(fileName, "w"))) /* Try open the file, if the open failed */
  {
    fprintf(stderr, "Cannot create the object file \"%s\"!\n", fileName); /* Print error */
    return false;
  }

  fprintf(fd, "%s %s\n\n", convertToBase32(result1, instructionCnt - START_OF_PROGRAM_IN_MEMORY), convertToBase32(result2, dataCnt)); /* Print to the file the length of the instruction' words and the length of the data in base 32 */

  instructionPtr = instructionHead; /* Point ptr to line linked list */
  /* While the item exists, the list didn't end */
  while (instructionPtr)
  {
    fprintf(fd, "%s %s\n", convertToBase32(result1, address), convertToBase32(result2, instructionPtr->data.value)); /* Print the word and it's address in base 32 to the file */

    address++; /* Increase the instruction counter */
    instructionPtr = instructionPtr->next; /* Point ptr to the next item in the list */
  }

  dataPtr = dataHead; /* Point ptr to line linked list */
  /* While the item exists, the list didn't end */
  while (dataPtr)
  {
    fprintf(fd, "%s %s\n", convertToBase32(result1, address), convertToBase32(result2, dataPtr->data.value)); /* Print the word and it's address in base 32 to the file */

    address++; /* Increase the instruction counter */
    dataPtr = dataPtr->next; /* Point ptr to the next item in the list */
  }

  fclose (fd); /* Close file */

  /* Free the allocated memory for the base 32 result */
  free(result1);
  free(result2);

  return true;
}

/**
* This function creates an entry file for the compiled file.
* Parameters: The name of the wanted file.
* Return value: None.
*/
void createEntriesFile(char *name)
{
  FILE *fd;
  char fileName[MAX_FILE_NAME_LENGTH];
  symbol_node_t *symbolPtr;
  char *result;

  /* Try allocate memory for convertion result */
  if ((result = (char *)malloc(BASE32_STR_RESULT_LEN * sizeof(char))) == NULL)
  {
    fprintf(stderr, "Error allocating memory for base 32 result!\n"); /* Print error */
    exit(0);
  }

  sprintf(fileName, "%s.ent", name); /* Format the new file name, add to it .ent */

  if (!(fd = fopen(fileName, "w"))) /* Try open the file, if the open failed */
  {
    fprintf(stderr, "Cannot create the entries file \"%s\"!\n", fileName); /* Print error */
    return;
  }

  symbolPtr = symbolHead; /* Point ptr to line linked list */
  /* While the item exists, the list didn't end */
  while (symbolPtr)
  {
    if (symbolPtr->data.type == ent)
      fprintf(fd, "%s %s\n", symbolPtr->data.name, convertToBase32(result, symbolPtr->data.pointer)); /* Print the symbol and it's address in base 32 to the file */

    symbolPtr = symbolPtr->next; /* Point ptr to the next item in the list */
  }

  free(result); /* Free the allocated memory for the base 32 result */
}

/**
* This function creates an external file for the compiled file.
* Parameters: The name of the wanted file
* Return value: None.
*/
void createExternalsFile(char *name)
{
  FILE *fd;
  char fileName[MAX_FILE_NAME_LENGTH];
  line_node_t *linePtr;
  char *result;

  /* Try allocate memory for convertion result */
  if ((result = (char *)malloc(BASE32_STR_RESULT_LEN * sizeof(char))) == NULL)
  {
    fprintf(stderr, "Error allocating memory for base 32 result!\n"); /* Print error */
    exit(0);
  }

  sprintf(fileName, "%s.ext", name); /* Format the new file name, add to it .ent */

  if (!(fd = fopen(fileName, "w"))) /* Try open the file, if the open failed */
  {
    fprintf(stderr, "Cannot create the externals file \"%s\"!\n", fileName); /* Print error */
    return;
  }

  linePtr = lineHead; /* Point ptr to line linked list */
  /* While the item exists, the list didn't end */
  while (linePtr)
  {
    if (linePtr->data.type == instruction)
    {
      if (linePtr->data.lineData.instruction.srcOp.data.symbol.type == ext) /* If source operand is external */
        fprintf(fd, "%s %s\n", linePtr->data.lineData.instruction.srcOp.data.symbol.name, convertToBase32(result, linePtr->data.lineData.instruction.srcOp.data.symbol.pointer)); /* Print the symbol and it's encode address in base 32 to the file */
      else if (linePtr->data.lineData.instruction.dstOp.data.symbol.type == ext) /* If destination operand is external */
        fprintf(fd, "%s %s\n", linePtr->data.lineData.instruction.dstOp.data.symbol.name, convertToBase32(result, linePtr->data.lineData.instruction.dstOp.data.symbol.pointer)); /* Print the symbol and it's encode address in base 32 to the file */
    }
    linePtr = linePtr->next; /* Point ptr to the next item in the list */
  }

  free(result); /* Free the allocated memory for the base 32 result */
}
