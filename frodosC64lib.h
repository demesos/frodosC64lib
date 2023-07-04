/********************************************** 
 * Frodo's C64 library 
 * May-2021 V1.4
 * Wilfried Elmenreich
 * Code under CC-BY-4.0 license
 **********************************************/

#ifndef _FRODOSC64LIB_H_
#define _FRODOSC64LIB_H_

#include <conio.h>
#include <stdint.h>

#ifndef VIC_BASE
  #define VIC_BASE 0xd000
#endif

#ifndef SCREEN_BASE
  #define SCREEN_BASE 0x0400
#endif

#ifndef VIC_BANK
  #define VIC_BANK 0x0000
#endif

/*IO functions*/
#define get() (asm("jsr $FFE4"),asm("ldx #$00"),__AX__)
#define textcolor(c)	(poke(646,c))

/*general functions for memory access and interrupt control */
#define poke(addr,val)	(*(unsigned char *)(addr)=(val))
#define peek(addr) (*(unsigned char *)(addr))
#define disable_int poke(56334u, peek(56334u) & 254)  //deactivate interrupts
#define enable_int poke(56334u, peek(56334u) | 1)  //activate interrupts
void setVICbank(char bank);

#define setVICcharset(addr) (asm("lda %w+24",VIC_BASE),asm("and #$f0"),asm("ora #%b",(addr & 0x3800)/0x400),asm("sta %w+24",VIC_BASE))
#define setVICscreen(addr) (asm("lda %w+24",VIC_BASE),asm("and #$0f"),asm("ora #%b",(addr & 0x3c00)/0x40),asm("sta %w+24",VIC_BASE))

#define random8() (peek(56324u)) //using low value of CIA timer as true random number generator
#define random16() (asm("lda $DC04"),asm("ldx $DC04"),__AX__)
#define random15() (asm("lda $DC04"),asm("and #$7F"),asm("tax"),asm("lda $DC04"),__AX__)
#define random(n) (random15() % (n))

/* sprite functions */
#define showSprite(n) (poke(VIC_BASE+21,peek(VIC_BASE+21)|(1<<n))) //activate sprite n
#define hideSprite(n) (poke(VIC_BASE+21,peek(VIC_BASE+21)&(0xFF - (1<<n)))) //activate sprite n

#define enableMultiColorSprite(n) (poke(VIC_BASE+0x1c,peek(VIC_BASE+0x1c)|(1<<n))) 
#define disableMultiColorSprite(n) (poke(VIC_BASE+0x1c,peek(VIC_BASE+0x1c)&(0xFF - (1<<n))))

#define enableXexpandSprite(n) (poke(VIC_BASE+0x1d,peek(VIC_BASE+0x1d)|(1<<n))) 
#define disableXexpandSprite(n) (poke(VIC_BASE+0x1d,peek(VIC_BASE+0x1d)&(0xFF - (1<<n))))

#define enableYexpandSprite(n) (poke(VIC_BASE+0x17,peek(VIC_BASE+0x17)|(1<<n))) 
#define disableYexpandSprite(n) (poke(VIC_BASE+0x17,peek(VIC_BASE+0x17)&(0xFF - (1<<n))))

#define setSpriteColor(n,c) (poke(VIC_BASE+0x27+n,c))
#define setSpriteMultiColor1(c) (poke(VIC_BASE+0x25,c))
#define setSpriteMultiColor2(c) (poke(VIC_BASE+0x26,c))
#define setSpriteCostume(n,c) (*(unsigned char *)(SCREEN_BASE+0x3f8+n)=(c))

#define getSpriteCostume(n) (peek(SCREEN_BASE+0x3f8+n))

#define getSpriteXpos(n) ( peek(VIC_BASE+n+n)+((peek(VIC_BASE+16)&(1<<n))<<(8-n)) )
#define getSpriteYpos(n) ( peek(VIC_BASE+1+n+n) )

void setSpriteXY(char n,int x,char y);
void updateSpriteAttributes(char n);

/* character functions */
#define setCharacterMultiColor poke(VIC_BASE+0x16,peek(VIC_BASE+0x16) | 16)
#define setCharacterSingleColor poke(VIC_BASE+0x16,peek(VIC_BASE+0x16) & 239)
#define setCharMultiColor1(c) (poke(VIC_BASE+0x22,c))
#define setCharMultiColor2(c) (poke(VIC_BASE+0x23,c))

/* joystick functions */
#define joystick_up(n) ((*(unsigned char *)(56322u-n) & 0x01)==0)
#define joystick_down(n) ((*(unsigned char *)(56322u-n) & 0x02)==0)
#define joystick_left(n) ((*(unsigned char *)(56322u-n) & 0x04)==0)
#define joystick_right(n) ((*(unsigned char *)(56322u-n) & 0x08)==0)
#define joystick_fire(n) ((*(unsigned char *)(56322u-n) & 0x10)==0)

/* misc functions */
void blinkwait(unsigned int seconds,unsigned char color);
void delayms(int delay);
void delaymsunlessfire(int delay);
void wait_end_of_frame();
int getkey();
void locate_cursor(uint8_t x,uint8_t y);

#define wait_for_rasterline(L) (__AX__ = (L),   \
                         asm("@WTLP_%s:", __LINE__),     \
                         asm("cmp $D012"),\
                         asm("bne @WTLP_%s", __LINE__),  \
                         asm("bit $D011"),\
                         asm("bmi @WTLP_%s", __LINE__)  )

extern const unsigned char twopotentials[];

#endif
