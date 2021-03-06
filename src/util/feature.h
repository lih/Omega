#ifndef INCLUDED_FEATURE
#define INCLUDED_FEATURE

typedef struct {
  enum { 
    DISABLED,ENABLING,ENABLED
  } state;
  char label[16];
  void (*initialize)();
} Feature;

#define PAD padLine();

void require(Feature* f);
void padLine();

#endif
