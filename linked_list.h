/* This macro will be used to create the list headers, any kind of data type */
#define CREATE_LINKED_LIST_HEADERS(funcName, nodeType, dataType, counter, head) \
  typedef struct nodeType { \
      dataType data; \
      struct nodeType *next; \
  } nodeType; \
\
  int counter; \
  nodeType * head; \
\
  bool funcName(dataType data);

/* This macro will be used to free any list */
#define FREE_LIST(nodeType, head) \
  { \
    nodeType *p; \
\
    while(head) \
    { \
        p = head; \
        head = head->next; \
        free(p); \
    } \
  }

  /* Creating headers for each list, the description of the lists are available in the .c file */
  CREATE_LINKED_LIST_HEADERS(addInstructionToList, instruction_node_t, word_t, instructionCnt, instructionHead)
  CREATE_LINKED_LIST_HEADERS(addDataToList, data_node_t, word_t, dataCnt, dataHead)
  CREATE_LINKED_LIST_HEADERS(addSymbolToList, symbol_node_t, symbol_t, symbolCnt, symbolHead)
  CREATE_LINKED_LIST_HEADERS(addLineToList, line_node_t, line_t, lineCnt, lineHead)
