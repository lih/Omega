#define FB_MEM ((void*)0xB8000)
#define FB_END ((void*)0xB8FA0)
#define LINE_SIZE (80*2)

static char* current;

static void scroll() {
  while(current >= FB_END) {
    char* src = FB_MEM+LINE_SIZE;
    char* dst = FB_MEM;
    while(src < FB_END) {
      *dst = *src; dst++; src++;
    }
    current -= LINE_SIZE;
  }
}

void putChar(char c) {
  switch(c) {
  case '\n':
    current += LINE_SIZE;
    current -= (current-(char*)FB_MEM) % LINE_SIZE;
    break;

  case '\0':
    break;
  default:
    *current=c;
    *(current+1) = 0x0B;
    current+=2;
    break;
  }
  scroll();
}

void printInt(int n) {
  char buf[32] = "";
  int i = 0;

  do { 
    buf[i] = n%10+'0';
    n/=10;
    i++;
  } while(n!=0);
  do {
    i--;
    putChar(buf[i]);
  } while(i>0);
}
void printStr(char* s) {
  while(*s != '\0') {
    putChar(*s); s++;
  }
}

void printf(char* f,...) {
  int* args = (int*)&f;
  args++;
    
  while(*f!='\0') {
    switch(*f) {
    case '%':
      f++;
      switch(*f) {
      case 'd':
	printInt(*args);
	break;
      case 'c':
	putChar(*(char*)args);
	break;
      case 's':
	printStr((char*)*args);
	break;
      }
      args++;
      break;
    default:
      putChar(*f);
      break;
    }
    f++;
  }
}

void clear() {
  for(current=FB_MEM;current<FB_END;current+=2) {
    *current = ' ';
    *(current+1) = 0x0B;
  }
  current=FB_MEM;
}
