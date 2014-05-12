#ifndef INCLUDED_ACPI
#define INCLUDED_ACPI

#include <constants.h>
#include <util/memory.h>
#include <core/feature.h>

typedef struct {
  char signature[8];
  byte checksum;
  char oemID[6];
  byte revision;
  struct SDTable* rsdt;
} RSDP;
typedef struct SDTable {
  union {
    char asS[4];
    dword asN;
  } signature;
  dword length;
  byte revision;
  byte checksum;
  char oemID[6];
  char oemTableID[8];
  dword oemRevision;
  dword creatorID;
  dword creatorRevision;
} SDTable;
typedef struct {
  SDTable  header;

  dword firmwareCtrl;
  dword dsdt;
 
  // field used in ACPI 1.0; no longer in use, for compatibility only
  byte  reserved;
 
  byte  preferredPowerManagementProfile;
  word sciInterrupt;
  dword smiCommandPort;
  byte  acpiEnable;
  byte  acpiDisable;
  byte  S4BIOS_REQ;
  byte  PSTATE_Control;
  dword PM1aEventBlock;
  dword PM1bEventBlock;
  dword PM1aControlBlock;
  dword PM1bControlBlock;
  dword PM2ControlBlock;
  dword PMTimerBlock;
  dword GPE0Block;
  dword GPE1Block;
  byte  PM1EventLength;
  byte  PM1ControlLength;
  byte  PM2ControlLength;
  byte  PMTimerLength;
  byte  GPE0Length;
  byte  GPE1Length;
  byte  GPE1Base;
  byte  CStateControl;
  word WorstC2Latency;
  word WorstC3Latency;
  word FlushSize;
  word FlushStride;
  byte  DutyOffset;
  byte  DutyWidth;
  byte  DayAlarm;
  byte  MonthAlarm;
  byte  Century;
} FADT;

extern Feature _acpi_;

#endif
