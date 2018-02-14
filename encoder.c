#include "types.h"
#include "linked_list.h"
#include "encoder.h"
#include <stdio.h>
#include <stdlib.h>

void encodeOperation(line_t *);
void encodeNumber(int);
void encodeRegisters(operand_t, operand_t);
void encodeSymbol(symbol_t *);
void encodeStructField(int);

void addInstructionSize(line_t *line)
{
  instructionCnt++; /* Any instruction must have 1 word which will include the operation itself */

  /* Add words by the source operand type */
  switch(line->lineData.instruction.srcOp.type)
  {
    /* If the operand is a number or a register or a symbol, it means we have to add one word */
    case number:
    case reg:
    case symbol:
      instructionCnt++;
      break;

    /* If the operand is a struct, it means we have to add 2 words */
    case structField:
      instructionCnt += 2;
      break;

    default:
      break;
  }

  /* Add words by the source operand type */
  switch(line->lineData.instruction.dstOp.type)
  {
    /* If the operand is a number or a symbol, it means we have to add one word */
    case number:
    case symbol:
      instructionCnt++;
      break;

    /* If the destination operand is a register and the source isn't a register, add one word */
    case reg:
      if (line->lineData.instruction.srcOp.type != reg)
        instructionCnt++;
      break;

    /* If the operand is a struct, it means we have to add 2 words */
    case structField:
      instructionCnt += 2;

    default:
      break;
  }
}

void encodeData(line_t *line)
{
  int i;
  word_t dataWord;

  /* Encode the different types of data */
  switch(line->type)
  {
    case data: /* If the type is data(numbers) */
      /* Going through the array of number and adding each of them */
      for (i = 0; i < line->lineData.data.count; i++)
      {
        dataWord.data.value = line->lineData.data.numbers[i];
        addDataToList(dataWord);
      }
      break;

    case string: /* If the type is a string */
      /* Going through the string and adding each of them */
      for (i = 0; line->lineData.string.str[i]; i++)
      {
        dataWord.data.value = line->lineData.string.str[i];
        addDataToList(dataWord);
      }

      /* Add 0 at the end of the string */
      dataWord.data.value = 0;
      addDataToList(dataWord);
      break;

    case strct: /* If the type is a struct */
      /* Adding the number to the data */
      dataWord.data.value = line->lineData.strct.num;
      addDataToList(dataWord);

      /* Going through the string and adding each of them */
      for (i = 0; line->lineData.strct.str[i]; i++)
      {
        dataWord.data.value = line->lineData.strct.str[i];
        addDataToList(dataWord);
      }

      /* Add 0 at the end of the string */
      dataWord.data.value = 0;
      addDataToList(dataWord);
      break;

    default:
      break;

  }
}

void encodeInstruction(line_t *line)
{
  encodeOperation(line); /* Start with encoding the operation */

  /* Encode the source operand by it's type */
  switch(line->lineData.instruction.srcOp.type)
  {
    case number: /* If the operand is a number */
      encodeNumber(line->lineData.instruction.srcOp.data.number);
      break;

    case reg: /* If the operand is a register */
      encodeRegisters(line->lineData.instruction.srcOp, line->lineData.instruction.dstOp); /* Sending both registers even tho we don't know if the second operand is a register, the function will check that */
      break;

    case symbol: /* If operand is a symbol */
      encodeSymbol(&line->lineData.instruction.srcOp.data.symbol);
      break;

    case structField: /* If operand is a struct */
      encodeSymbol(&line->lineData.instruction.srcOp.data.symbol);
      encodeStructField(line->lineData.instruction.srcOp.data.field);

    default:
      break;
  }

  /* Encode the destination operand by it's type */
  switch(line->lineData.instruction.dstOp.type)
  {
    case number: /* If the operand is a number */
      encodeNumber(line->lineData.instruction.dstOp.data.number);
      break;

    case reg: /* If the operand is a register */
      if (line->lineData.instruction.srcOp.type != reg) /* If the source operand isn't a register, send them to be encoded */
        encodeRegisters(line->lineData.instruction.srcOp, line->lineData.instruction.dstOp); /* Sending both registers even tho we don't know if the second operand is a register, the function will check that */
      break;

    case symbol: /* If operand is a symbol */
      encodeSymbol(&line->lineData.instruction.dstOp.data.symbol);
      break;

    case structField: /* If operand is a struct */
      encodeSymbol(&line->lineData.instruction.dstOp.data.symbol);
      encodeStructField(line->lineData.instruction.dstOp.data.field);

    default:
      break;
  }
}

void encodeOperation(line_t *line)
{
  word_t word = {{0}};

  word.operation.opcode = line->instruction->opcode; /* Set instruction opcode */

  if (line->lineData.instruction.srcOp.type != empty) /* If the source operand isn't empty, set it's source address mode */
    word.operation.srcAddressMode = line->lineData.instruction.srcOp.type;

  if (line->lineData.instruction.dstOp.type != empty) /* If the source operand isn't empty, set it's source address mode */
    word.operation.dstAddressMode = line->lineData.instruction.dstOp.type;

  word.operation.type = 0; /* Set word' type to absolute, operation word is always absolute */

  addInstructionToList(word);
}

void encodeNumber(int number)
{
  word_t word = {{0}};

  word.immediateOperand.value = number; /* Set operand' value to the number */
  word.immediateOperand.type = 0; /* Set word' type to absolute, immediate operand is always absolute */

  addInstructionToList(word);
}

void encodeRegisters(operand_t srcOp, operand_t dstOp)
{
  word_t word = {{0}};

  if (srcOp.type == reg) /* If source operand is a register, set it's number in the correct location */
    word.registersOperand.srcReg = srcOp.data.reg;

  if (dstOp.type == reg) /* If destination operand is a register, set it's number in the correct location */
    word.registersOperand.dstReg = dstOp.data.reg;

  word.registersOperand.type = 0; /* Set word' type to absolute, registers operand is always absolute */

  addInstructionToList(word);
}

void encodeSymbol(symbol_t *symbol)
{
  word_t word = {{0}};

  word.addressOperand.address = symbol->pointer;

  if (symbol->type == ext) /* If symbol is extern, set word' type to extern and save encode address for entries file, in the pointer of the symbol */
  {
    word.addressOperand.type = 1;
    symbol->pointer = instructionCnt;
  } else /* If symbol is local, set word' type to relocatable */
    word.addressOperand.type = 2;

  addInstructionToList(word);

}

void encodeStructField(int field)
{
  word_t word = {{0}};

  word.structOperandField.field = field; /* Set operand' value to the field */
  word.structOperandField.type = 0; /* Set word' type to absolute, struct field is always absolute */

  addInstructionToList(word);
}
