#include "types.h"
#include "linked_list.h"
#include "utils.h"
#include "encoder.h"
#include "second_pass.h"
#include <stdio.h>
#include <string.h>

void recheckOperand(line_t *, operand_t *);

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
