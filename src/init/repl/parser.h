#ifndef INCLUDED_PARSER
#define INCLUDED_PARSER

#include <init/repl/thunk.h>

typedef struct {
  char* str;
  int depth;
} PState;

#define FREE spaces(pstate)
#define EXPR expr(pstate)
#define IDENT ident(pstate)
#define CUR (*(pstate->str))
#define FORWARD ((pstate->str)++)
#define ATOM atom(pstate)

typedef enum {
  REGULAR, OPAREN, CPAREN, SPACE, DIGIT, QUOTE, OPERATOR, END
} PACKED CharClass;
extern CharClass classes[128];

void spaces(PState* pstate);
Thunk* expr(PState* pstate);
Thunk* atom(PState* pstate);
Thunk* ident(PState* pstate);

#endif
