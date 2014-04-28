#include "memory.h"
#include "feature.h"

dword featureDepth = 0;
FeatureDesc features[FEATURE_END];

void initACPI();
void initMemory();
void initInterrupts();
void initExceptions();
void initSyscalls();
void initIRQs();
void initSchedule();
void initUniverse();
void initKeyboard();

void initFeature() {
  SET_STRUCT(FeatureDesc,features[ACPI],{
      .state = DISABLED,
	.label = "ACPI",
	.initialize = initACPI
    });
  SET_STRUCT(FeatureDesc,features[MEMORY],{
      .state = DISABLED,
	.label = "memory",
	.initialize = initMemory
    });
  SET_STRUCT(FeatureDesc,features[INTERRUPTS],{
      .state = DISABLED,
	.label = "interrupts",
	.initialize = initInterrupts
    });
  SET_STRUCT(FeatureDesc,features[IRQS],{
      .state = DISABLED,
	.label = "IRQs",
	.initialize = initIRQs
    });
  SET_STRUCT(FeatureDesc,features[EXCEPTIONS],{
      .state = DISABLED,
	.label = "exceptions",
	.initialize = initExceptions
    });
  SET_STRUCT(FeatureDesc,features[SYSCALLS],{
      .state = DISABLED,
	.label = "syscalls",
	.initialize = initSyscalls
    });
  SET_STRUCT(FeatureDesc,features[UNIVERSE],{
      .state = DISABLED,
	.label = "universe",
	.initialize = initUniverse
    });
  SET_STRUCT(FeatureDesc,features[SCHEDULE],{
      .state = DISABLED,
	.label = "schedule",
	.initialize = initSchedule
    });
  SET_STRUCT(FeatureDesc,features[KEYBOARD],{
      .state = DISABLED,
	.label = "keyboard",
	.initialize = initKeyboard
    });
}
void padLine() {
  int i; 
  for(i=0;i<featureDepth;i++)
    printStr("  ");
  printStr("* ");
}
void require(Feature f) {
  FeatureDesc* feature = &features[(int)f];
  switch(feature->state) {
  case ENABLED:
    break;
  case ENABLING:
    printf("Circular dependencies for feature %s\n",feature->label);
    break;
  case DISABLED: {
    feature->state = ENABLING;
    padLine(); printf("Initializing %s\n",feature->label);
    featureDepth++;
    feature->initialize();
    featureDepth--;
    feature->state = ENABLED;
    break;
  }
  }
}
