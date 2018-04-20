/********************************************** 
 * Frodo's C64 library 
 * Jul-2016 V1.0
 * Wilfried Elmenreich
 * Code under CC-BY-4.0 license
 * made as part of #bitbitjam3
 **********************************************/

#include "frodosC64lib.h"

const unsigned char twoponentials[] =
{0x01,0x02,0x04,0x08,0x10,0x20,0x40,0x80};
static unsigned char globalSubN, globalSubY;
static unsigned int globalSubX;
//static unsigned long cnt;

/* n being the sprite number (0..7) */
void setSpriteXY(char n,int x,char y)
{
//     cnt+=1;
//     if (cnt % 100 == 0) {
//        gotoxy(0,10);
//        cprintf("%d %d",cnt,clock());
//     }
     globalSubN = n;
     globalSubX = x;
     globalSubY = y;
     __asm__ ("lda %v", globalSubN);
     __asm__ ("tay");
     __asm__ ("asl");
     __asm__ ("tax");
     __asm__ ("lda %v,y", twoponentials);
     __asm__ ("tay");
     __asm__ ("lda %v", globalSubY);
     __asm__ ("sta 53249,x"); //set y coordinate
     __asm__ ("lda %v", globalSubX); //load low value of x coordinate
     __asm__ ("sta 53248,x"); //set x coordinateb(lower byte)
     __asm__ ("tya");
     __asm__ ("ldx %v+1", globalSubX); //load high value of x coordinate     
     __asm__ ("beq _label1");
     __asm__ ("ora 53264");
     __asm__ ("bne _fin");
     __asm__ ("_label1:");
     __asm__ ("eor #$FF");
     __asm__ ("and 53264");
     __asm__ ("_fin:");
     __asm__ ("sta 53264"); 
}

void setSpriteXY2(char n,int x,char y)
{
     poke(53248u+n+n+1,y);
     poke(53248u+n+n,x);
     if (x>255)
          poke(53264u,peek(53264u)|(1<<n));
     else
          poke(53264u,peek(53264u)&(0xFF - (1<<n)));     
}

void blinkwait(unsigned int seconds,unsigned char color)
{
    unsigned int i;
    for(i=0;i<seconds*1500;i++) {
            poke (53280L,0);
            poke (53280L,color);
    }
    poke (53280L,0);   
}

/* bank index from 0..3 
  * 3 $0000–$3FFF Char ROM at $1000-$1FFF
  * 2 $4000–$7FFF Char ROM inactive
  * 1 $8000–$BFFF Char ROM at $9000–$9FFF
  * 0 $C000–$FFFF Char ROM inactive **/
void setVICbank(char bank)
{
     poke(0xDD00, peek(0xDD00) & 0xFC | bank);
}

void setVICcharset(char index)
{
     poke(53272U,(peek(53272U) & 240) | (index*2));
}

void setVICscreen(unsigned int index)
{
     poke(0xD018, peek(0xD018) & 0x0F | (index*16)); //set VIC
     poke(648,index*4+(3-peek(0xDD00)&3)*64); //tell the OS where to print
}
