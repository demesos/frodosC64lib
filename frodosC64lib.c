/********************************************** 
 * Frodo's C64 library 
 * May-2021 V1.4
 * Wilfried Elmenreich
 * Code under CC-BY-4.0 license
 * made as part of #bitbitjam3
 **********************************************/

#include "frodosC64lib.h"

const unsigned char twopotentials[] =
{0x01,0x02,0x04,0x08,0x10,0x20,0x40,0x80};
static unsigned char globalSubN, globalSubY;
static unsigned int globalSubX;

/* Wait delay in ms */
void delayms(int delay)
{
    globalSubX = delay;
    __asm__ ("ldx %v",globalSubX);
    __asm__ ("lda %v+1",globalSubX);
    __asm__ ("dllp0: ldy #$b3");
    __asm__ ("dllp1: dey");
    __asm__ ("bne dllp1");
    __asm__ ("dex");
    __asm__ ("bne dllp0");
    __asm__ ("sec");
    __asm__ ("sbc #$01");
    __asm__ ("bpl dllp0");
}

/* Wait delay in ms (with option to skip delay by pressing fire button 2) */
void delaymsunlessfire(int delay)
{
    globalSubX = delay;

    __asm__ ("ldx %v",globalSubX);
    __asm__ ("lda %v+1",globalSubX);
    __asm__ ("dluflp0: pha");
    __asm__ ("dluflp1: lda $dc00");
    __asm__ ("and #$10");
    __asm__ ("bne dlufskp");
    __asm__ ("pla");
    __asm__ ("lda #$ff");
    __asm__ ("bne dlufexitlp");
    __asm__ ("dlufskp: ldy #$b1");
    __asm__ ("dluflp2: dey");
    __asm__ ("bne dluflp2");
    __asm__ ("dex");
    __asm__ ("bne dluflp1");
    __asm__ ("pla");
    __asm__ ("sec");
    __asm__ ("sbc #$01");
    __asm__ ("bpl dluflp0");
    __asm__ ("dlufexitlp:");
}

/* n being the sprite number (0..7) */
void setSpriteXY(char n,int x,char y)
{
     globalSubN = n;
     globalSubX = x;
     globalSubY = y;
     __asm__ ("lda %v", globalSubN);
     __asm__ ("tay");
     __asm__ ("asl");
     __asm__ ("tax");
     __asm__ ("lda %v,y", twopotentials);
     __asm__ ("tay");
     __asm__ ("lda %v", globalSubY);
     __asm__ ("sta %w,x", VIC_BASE+1); //set y coordinate
     __asm__ ("lda %v", globalSubX); //load low value of x coordinate
     __asm__ ("sta %w,x", VIC_BASE);  //set x coordinate (lower byte)
     __asm__ ("tya");
     __asm__ ("ldx %v+1", globalSubX); //load high value of x coordinate     
     __asm__ ("beq _label1");
     __asm__ ("ora %w",VIC_BASE+0x10);
     __asm__ ("bne _fin");
     __asm__ ("_label1:");
     __asm__ ("eor #$FF");
     __asm__ ("and %w",VIC_BASE+0x10);
     __asm__ ("_fin:");
     __asm__ ("sta %w",VIC_BASE+0x10);
}

void updateSpriteAttributes(char n) {
    globalSubN=peek(SCREEN_BASE+0x3f8+n);
    
     __asm__ ("sei");
     __asm__ ("lda $1");
     __asm__ ("pha");
     __asm__ ("lda #$34"); //make RAM visible in all areas       
     __asm__ ("sta $1");     
     globalSubX=peek(VIC_BANK + 0x3f + 64*globalSubN);
     __asm__ ("pla");     
     __asm__ ("sta $1");              
     __asm__ ("cli");     
     
    if (globalSubX>=128) {
      enableMultiColorSprite(n);
    } else {
      disableMultiColorSprite(n);
    }     
    setSpriteColor(n,globalSubX);     
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
  * 0 $0000–$3FFF Char ROM at $1000-$1FFF
  * 1 $4000–$7FFF Char ROM inactive
  * 2 $8000–$BFFF Char ROM at $9000–$9FFF
  * 3 $C000–$FFFF Char ROM inactive **/
void setVICbank(char bank)
{
     poke(0xDD00, peek(0xDD00) & 0xFC | (3-bank));
}

//void setVICcharset(char index)
//{
     //poke(53272U,(peek(53272U) & 240) | (index*2));
//}

//void setVICscreen(unsigned int index)
//{
     //poke(0xD018, peek(0xD018) & 0x0F | (index*16)); //set VIC
     //poke(648,index*4+(3-peek(0xDD00)&3)*64); //tell the OS where to print
//}


int getkey() {
    int pressedkey=0;
    poke(198,0);
    do
    {
      pressedkey=get();
    }
    while (pressedkey==0);
    return pressedkey;
}

unsigned int input_uint()
{
     unsigned int v=0;
     int digits=0,pressedkey;
     
     cputc(175); //cursor
     asm("lda #$9D");     
     asm("jsr $FFD2"); //cursor left
//     cputc(157); //left
     while(1) {
       pressedkey=getkey();
       if (pressedkey==13) break;
       if ((pressedkey>='0')&&(pressedkey<='9')&&digits<5) {
         v=v*10+pressedkey-'0';
         cputc(pressedkey);
         cputc(175); //cursor
         asm("lda #$9D");     
         asm("jsr $FFD2"); //cursor left
         digits++;
       }
       if ((pressedkey==20) && (digits>0)) {
         v=v/10;
         asm("lda #$14");     
         asm("jsr $FFD2"); //cursor left 
         digits--;       
       }
     }
     cputc(' '); //delete cursor
     return v;
}

void wait_end_of_frame(void)
{
  //wait for raster < 255
  __asm__ ("wtlp00: lda $D011");
  __asm__ ("and #$80");
  __asm__ ("bne wtlp00");
  //wait for raster > 255  
   __asm__ ("wtlp01: lda $D011");
  __asm__ ("and #$80");
  __asm__ ("beq wtlp01"); 
}

void locate_cursor(uint8_t x,uint8_t y)
{
   poke(211,x);
   poke(214,y);
   asm("jsr 58640");
}

