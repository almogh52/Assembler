/**
* Made by © 2018 Almog Hamdani
* linked_list.c
*
* This file is incharge of all the linked lists.
* It creates 4 linked lists using a macro that creates a function that adds item to them.
**/

#include <stdio.h>
#include <stdlib.h>
#include "types.h"
#include "linked_list.h"

/* This macro will be used to create the add to list function for each list */
#define CREATE_LINKED_LIST(funcName, nodeType, dataType, counter, head) \
    bool funcName(dataType data) \
    { \
      nodeType *new, *temp; \
\
      new = (nodeType *) malloc(sizeof(nodeType)); /* Allocate memory for the new node */ \
      if (!new) /* If an error accoured */ \
      { \
        fprintf(stderr, "Error allocating memory for linked list\n"); /* Print error */ \
        exit(0); \
      } \
      new->data = data; /* Copy the data to the new node's data */ \
      new->next = NULL; /*Set next item to null since the new item is the last item */ \
\
      if (!head) /* If there is no head to the list */ \
      { \
        head = new; \
      } else { \
        temp = head; /* Set the temp pointer to point to the start of the list */ \
\
        /* Get the last item in the list */ \
        while(temp->next) \
          temp = temp->next; /* Set the temp ptr to be the next item in the list */ \
\
        /* Set the new item as the item after the last item */ \
        temp->next = new; \
      } \
\
      counter ++; \
\
      return true; \
    }

CREATE_LINKED_LIST(addInstructionToList, instruction_node_t, word_t, instructionCnt, instructionHead) /* This will hold the instructions, the code part of the input file */
CREATE_LINKED_LIST(addDataToList, data_node_t, word_t, dataCnt, dataHead) /* This will hold the data, the data part of the input file, such as string, structs and more */
CREATE_LINKED_LIST(addSymbolToList, symbol_node_t, symbol_t, symbolCnt, symbolHead) /* This will hold the symbols that were found in the file */
CREATE_LINKED_LIST(addLineToList, line_node_t, line_t, lineCnt, lineHead) /* This will hold the formatted lines of the file */
