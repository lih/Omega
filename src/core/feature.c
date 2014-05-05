#include <cpu/memory.h>
#include <core/feature.h>

dword featureDepth = 0;

void padLine() {
  int i; 
  for(i=0;i<featureDepth;i++)
    printStr("+-");
  printStr("+ ");
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
    featureDepth++;
    feature->initialize();
    featureDepth--;
    PAD printf("Initialized %s\n",feature->label);
    feature->state = ENABLED;
    break;
  }
  }
}