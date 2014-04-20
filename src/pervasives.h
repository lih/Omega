#ifndef INCLUDED_PERVASIVES
#define INCLUDED_PERVASIVES

unsigned char	inportb(unsigned short _port);
void		outportb(unsigned short _port,unsigned char _msg);
void		shutdown();
void		nop();

#endif
