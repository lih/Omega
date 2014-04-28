#include "memory.h"
#include "feature.h"

dword featureDepth = 0;

void padLine() {
  int i; 
  for(i=0;i<featureDepth;i++)
    printStr("  ");
  printStr("* ");
}
void require(Feature* feature) {
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
