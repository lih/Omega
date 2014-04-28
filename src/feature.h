#ifndef INCLUDED_FEATURE
#define INCLUDED_FEATURE

typedef struct {
  enum { 
    DISABLED,ENABLING,ENABLED
  } state;
  char label[16];
  void (*initialize)();
} FeatureDesc;

typedef enum {
  ACPI, MEMORY, INTERRUPTS, SYSCALLS, IRQS, EXCEPTIONS, UNIVERSE, SCHEDULE, KEYBOARD,
  FEATURE_END
} Feature;

void initFeature();
void require(Feature f);
void padLine();

#endif
