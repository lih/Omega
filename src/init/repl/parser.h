#ifndef INCLUDED_PARSER
#define INCLUDED_PARSER

#include <init/repl/thunk.h>

typedef struct {
  char* str;
  int depth;
} PState;

#define FREE spaces(pstate)
#define EXPR expr(pstate)
#define CUR (*(pstate->str))
#define FORWARD ((pstate->str)++)

void spaces(PState* pstate);
Thunk* expr(PState* pstate);

#endif
