/**
* Made by © 2018 Almog Hamdani
* encoder.h
*
* This is the header file of the encoder.c
**/

/**
* This function increases the instruction counter by the size that is needed by the line.
* Parameters: The line.
* Return value: None.
*/
void addInstructionSize(line_t *);

/**
* This function encodes instruction that is specificed in a certain line.
* Parameters: The line that contains the instruction to be encoded.
* Return value: None.
*/
void encodeInstruction(line_t *);

/**
* This function encodes data that is specificed in a certain line.
* Parameters: The line that contains the data to be encoded.
* Return value: None.
*/
void encodeData(line_t *);
