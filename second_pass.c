/**
* Made by © 2018 Almog Hamdani
* second_pass.c
*
* This file is incharge of all the second pass functions.
* This is rechecking all the lines in the code to see that there are no more issues.
* If no issues were found, it sends the line to be encoded.
**/

#include "types.h"
#include "linked_list.h"
#include "utils.h"
#include "encoder.h"
#include "second_pass.h"
#include <stdio.h>
#include <string.h>

void recheckOperand(line_t *, operand_t *);

/**
* This function is rechecking the line in the second pass, if found errors, it will print them
* Parameters: The line.
* Return value: None.
*/
void recheckLine(line_t *line)
{
  symbol_t *data;

  if (line->type == instruction) /* If the line is an instruction */
  {
    recheckOperand(line, &line->lineData.instruction.srcOp); /* Recheck source operand */
    recheckOperand(line, &line->lineData.instruction.dstOp); /* Recheck destination operand */
    encodeInstruction(line); /* Send instruction to be encoded */
  } else if (line->type == ent)
  {
    if ((data = checkIfSymbolExists(line->lineData.extent.symbol)) == NULL) /* If a symbol with that name wasn't found, print error */
      printError(line, "Undefined symbol \"%s\"! Entry must be a label that exists in this file!", line->lineData.extent.symbol.name);
    else
      data->type = ent; /* Set symbol' type to entry */
  }
}

/**
* This function is rechecking an operand from a line in the code, if found errors, it will print them.
* Parameters: The line and the operand to be checked.
* Return value: None.
*/
void recheckOperand(line_t *line, operand_t *op)
{
  symbol_t *data;

  if (op->type == symbol) /* If the operand is a symbol, check if it is valid */
  {
    if ((data = checkIfSymbolExists(op->data.symbol)) == NULL) /* If a symbol with that name wasn't found, print error */
      printError(line, "Undefined symbol \"%s\"!", op->data.symbol.name);
    else {
      op->data.symbol.type = data->type; /* Set symbol' type */
      op->data.symbol.pointer = data->pointer; /* Set symbol' type */
    }
  } else if (op->type == structField) /* If the operand is struct, check if it is valid */
  {
    if ((data = checkIfSymbolExists(op->data.symbol)) == NULL) /* If a symbol with that name wasn't found, print error */
      printError(line, "Undefined symbol \"%s\"!", op->data.symbol.name);
    else {
      if (data->type != strct) /* If the symbol that was found isn't pointing to a struct, print error */
        printError(line, "Symbol \"%s\" isn't pointing to a struct!", op->data.symbol.name);

      op->data.symbol.type = data->type; /* Set symbol' type */
      op->data.symbol.pointer = data->pointer; /* Set symbol' type */
    }
  }
}
