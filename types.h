
#define MAX_LINE_LENGTH 81
#define MAX_LABEL_LENGTH 30
#define MAX_ERROR_LENGTH 200
#define MAX_VALUE_LENGTH 80
#define MAX_VALUES 40
#define MAX_FILE_NAME_LENGTH 100
#define AMOUNT_OF_INSTRUCTIONS 16

/* Bool enum, will be used as a boolean var in the program */
typedef enum {
  false = 0,
  true
} bool;

typedef enum {
  nothing,
  unknown,
  instruction,
  ent,
  ext,
  data,
  string,
  strct /* Struct */
} sentenceType;

/* Addressing mode struct, will be used to define the addressing mode of each operand in the different instructions */
typedef struct {
  bool immediateAddress; /* First method - A number, ex: #-1 */
  bool directAddress; /* Second method - A label, ex: x */
  bool structAddress; /* Third method - Accessing struct, ex: s.1 */
  bool registerAddress; /* Forth method - A register, ex: r0 */
} address_t;

typedef struct {
  char *name; /* The name of the instruction */
  int opcode; /* The opcode of the instruction */
  int ops; /* The amount of operands */
  address_t srcAddressingMode; /* The addressing mode of the source operand */
  address_t dstAddressingMode; /* The addressing mode of the destination operand */
} instruction_t;

/* This array contains all of the instructions and their requirements */
static const instruction_t instructions[] = {
/*name   opcode ops  source addressing mode       destination addressing mode */
  {"mov",   0,  2,  {true, true, true, true},     {false, true, true, true}},
  {"cmp",   1,  2,  {true, true, true, true},     {true, true, true, true}},
  {"add",   2,  2,  {true, true, true, true},     {false, true, true, true}},
  {"sub",   3,  2,  {true, true, true, true},     {false, true, true, true}},
  {"not",   4,  1,  {false, false, false, false}, {false, true, true, true}},
  {"clr",   5,  1,  {false, false, false, false}, {false, true, true, true}},
  {"lea",   6,  2,  {false, true, true, false},   {false, true, true, true}},
  {"inc",   7,  1,  {false, false, false, false}, {false, true, true, true}},
  {"dec",   8,  1,  {false, false, false, false}, {false, true, true, true}},
  {"jmp",   9,  1,  {false, false, false, false}, {false, true, true, true}},
  {"bne",   10, 1,  {false, false, false, false}, {false, true, true, true}},
  {"red",   11, 1,  {false, false, false, false}, {false, true, true, true}},
  {"prn",   12, 1,  {false, false, false, false}, {true, true, true, true}},
  {"jsr",   13, 1,  {false, false, false, false}, {false, true, true, true}},
  {"rts",   14, 0,  {false, false, false, false}, {false, false, false, false}},
  {"stop",  15, 0,  {false, false, false, false}, {false, false, false, false}}
};

typedef enum {
    r0,
    r1,
    r2,
    r3,
    r4,
    r5,
    r6,
    r7
} register_t;


/* This struct will be used to hold any symbol */
typedef struct {
  int pointer; /* Where the symbol points to */
  char name[MAX_LABEL_LENGTH]; /* The name of the symbol */
  sentenceType type; /* This contains the type of the symbol, instruction or data */
} symbol_t;

typedef struct {

  /* This enum will hold the type of the operand */
  enum {
    number = 0, /* First method = Immediate Address */
    symbol = 1, /* Second method = Direct Address */
    structField = 2, /* Third method - Struct Address */
    reg = 3, /* Fourth method - Register Access */
    empty /* No operand */
  } type;

  /* This union will hold any data the operand has */
  struct {
    /* This will hold any number that is used as an operand */
    int number;

    /* This will hold any symbol that is used as an operand, might be struct's symbol */
    symbol_t symbol;

    /* This will hold any struct field that is used as an operand */
    int field;

    /* This will hold any register that is used as an operand */
    register_t reg;
  } data;
} operand_t;

typedef struct {
  bool error; /* Error was found in the line */
  int lineNumber;
  char fileName[MAX_FILE_NAME_LENGTH]; /* The name of the file the line is in it */
  char input[MAX_LINE_LENGTH]; /* The line */
  symbol_t symbol; /* The label in the line, if there is one */
  sentenceType type; /* The type of the sentence */
  const instruction_t *instruction;

  union { /* Will hold the data of the line itself like operands or string or numbers */
    struct { /* Instruction data: source operand, destination operand */
      operand_t srcOp;
      operand_t dstOp;
    } instruction;

    struct { /* Data' data: array of integers */
      int count;
      int numbers[MAX_VALUES];
    } data;

    struct { /* String' data: string */
      char str[MAX_LINE_LENGTH];
    } string;

    struct { /* Struct's data: integer and a string */
      int num;
      char str[MAX_LINE_LENGTH];
    } strct;

    struct { /* External and entry, same are used as label holder */
      symbol_t symbol;
    } extent;
  } lineData;
} line_t;

/* This type will be used to store any kind of word */
typedef union {

  /* If the word is an operation(A,R,E, destination address mode, source address mode, opcode) */
  struct {
    unsigned int type:2;
    unsigned int dstAddressMode:2;
    unsigned int srcAddressMode:2;
    unsigned int opcode:4;
  } operation;

  /* If the word is a number */
  struct {
    unsigned int type:2;
    unsigned int value:8;
  } immediateOperand;

  /* If the word is an variable */
  struct {
    unsigned int type:2;
    unsigned int address:8;
  } addressOperand;

  /* If the word is a struct field operand */
  struct {
    unsigned int type:2;
    unsigned int field:8;
  } structOperandField;

  /* If the word is a register number */
  struct {
    unsigned int type:2;
    unsigned int dstReg:4;
    unsigned int srcReg:4;
  } registersOperand;

  /* Will be used to store any data we have(numbers and strings) */
  struct {
    unsigned int value:10;
  } data;

  int value;

} word_t;
