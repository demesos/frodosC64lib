/********************************************** 
 * Frodo's C64 library 
 * Jul-2016 V1.0
 * Wilfried Elmenreich
 * Code under CC-BY-4.0 license
 **********************************************/

#ifndef FRODOSC64LIB
#define FRODOSC64LIB

/*general functions for memory access and interrupt control */
#define poke(addr,val)	(*(unsigned char *)(addr)=(val))
#define peek(addr) (*(unsigned char *)(addr))
#define disable_int poke(56334u, peek(56334u) & 254)  //deactivate interrupts
#define enable_int poke(56334u, peek(56334u) | 1)  //activate interrupts
void setVICbank(char bank);
void setVICcharset(char index);
void setVICscreen(unsigned int index);
#define random8 (peek(56324u)) //using low value of CIA timer as true random number generator

/* sprite functions */
#define showSprite(n) (poke(53269u,peek(53269u)|(1<<n))) //activate sprite n
#define hideSprite(n) (poke(53269u,peek(53269u)&(0xFF - (1<<n)))) //activate sprite n
#define setSpriteColor(n,c) (poke(53287u+n,c))
#define setSpriteMultiColor1(c) (poke(53285u,c))
#define setSpriteMultiColor2(c) (poke(53286u,c))
void setSpriteXY(char n,int x,char y);

/* character functions */
#define setCharacterMultiColor poke(53270u,peek(53270u) | 16)
#define setCharacterSingleColor poke(53270u,peek(53270u) & 239)

/* joystick functions */
#define joystick_up(n) ((*(unsigned char *)(56322u-n) & 0x01)==0)
#define joystick_down(n) ((*(unsigned char *)(56322u-n) & 0x02)==0)
#define joystick_left(n) ((*(unsigned char *)(56322u-n) & 0x04)==0)
#define joystick_right(n) ((*(unsigned char *)(56322u-n) & 0x08)==0)
#define joystick_fire(n) ((*(unsigned char *)(56322u-n) & 0x10)==0)

/* misc functions */
void blinkwait(unsigned int seconds,unsigned char color);

#endif
