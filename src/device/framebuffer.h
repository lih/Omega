#ifndef FRAMEBUFFER
#define FRAMEBUFFER

extern char charMode;

void putChar(char);
void printInt(int);
void printStr(char*);
void printf(char*,...);
void clearFB();
void setCursor();

#define DBG_CHATTY 0
#define DBG_VERBOSE 1
#define DBG_CANDID 2
#define DBG_QUIET  3
#define DBG_MUTE 4

#define DEBUG_LEVEL DBG_CANDID

#if (DBG_CHATTY >= DEBUG_LEVEL)
#define CHATTER(...) printf(__VA_ARGS__)
#else
#define CHATTER(...) ((void)0)
#endif
#if (DBG_VERBOSE >= DEBUG_LEVEL)
#define INFO(...) printf(__VA_ARGS__)
#else
#define INFO(...) ((void)0)
#endif
#if (DBG_CANDID >= DEBUG_LEVEL)
#define WARN(...) printf(__VA_ARGS__)
#else
#define WARN(...) ((void)0)
#endif
#if (DBG_QUIET >= DEBUG_LEVEL)
#define ERR(...) printf(__VA_ARGS__)
#else
#define ERR(...) ((void)0)
#endif

#endif
