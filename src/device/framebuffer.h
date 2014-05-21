#ifndef FRAMEBUFFER
#define FRAMEBUFFER

extern char charMode;

void putChar(char);
void printInt(int);
void printStr(char*);
void printf(char*,...);
void clearFB();
void setCursor();

#define DBG_VERBOSE 0
#define DBG_INFO 1
#define DBG_WARN 2
#define DBG_ERR  3
#define DBG_MUTE 4

#define DEBUG_LEVEL DBG_WARN

#if (DBG_VERBOSE >= DEBUG_LEVEL)
#define CHATTER(...) printf(__VA_ARGS__)
#else
#define CHATTER(...) ((void)0)
#endif
#if (DBG_INFO >= DEBUG_LEVEL)
#define INFO(...) printf(__VA_ARGS__)
#else
#define INFO(...) ((void)0)
#endif
#if (DBG_WARN >= DEBUG_LEVEL)
#define WARN(...) printf(__VA_ARGS__)
#else
#define WARN(...) ((void)0)
#endif
#if (DBG_ERR >= DEBUG_LEVEL)
#define ERR(...) printf(__VA_ARGS__)
#else
#define ERR(...) ((void)0)
#endif

#endif
