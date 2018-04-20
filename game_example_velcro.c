/****************************************************************
 * Velcro - a computer game for the Commodore 64 made in 2017
 * made for #bitbitjam4 by Wilfried Elmenreich
 *
 * Development was done with cc65, SpritePad 2.0 and CharPad 2.0
 * Development time was 16 hrs
 ****************************************************************/


//#include <stdio.h>
#include <stdlib.h>
#include <conio.h>

//#define DEBUG
#define GAMEJOLTBUILT
/* the online gamejolt emulator loads the file, but instead of running 
   it issues a sys call to address 4096. To make the game runnable as well
   via RUN command (which does a SYS 2061) and via SYS 4096, the machine
   code for JMP 2061 (4C 0D 08) will be injected at byte 0x0801 in the file
   (because loading address + 0x801 - 2bytes = 4096) 
   and the original three bytes are restored after start. */

#include "frodosC64lib.c"

/*-----------------------------------------------------------------------*/
#define VIC_BASE_RAM			(0xC000)
#define	SCREEN_RAM			((char*)VIC_BASE_RAM+0x0000)
#define SPRITECOSTUM			((char*)SCREEN_RAM+0x03f8)
#define CHARMAP_RAM			((char*)VIC_BASE_RAM+0x1000)
#define	SPRITE_RAM			((char*)VIC_BASE_RAM+0x0800)
#define LVL_SPRITE_RAM			((char*)VIC_BASE_RAM+0x3200)
#ifndef COLOR_RAM
  #define COLOR_RAM			((char*)0xd800)
#endif
#define DELAY 50
#define TIMEUNIT 0x40
#define field(x,y) (40*((y-30)/8)+(x-17)/8)

#define MAXLIVES 3
#define LEVELTIME 30*TIMEUNIT

#include "velcro-assets.c"

void copy_charset(void);
void titlescreen(void);
void init(void);
void printslowly(char *s);
void spriteRun(void);
void drawGameScreen(void);
void drawBarn(int x,int y);
void game_loop(void);
void initSprites(unsigned char nsheep,unsigned char nsnakes);

unsigned char lives,level,startgame,nsheepleft;
unsigned int timer, announceexpire;
const unsigned char levelsheep[] =  {0,7,6,5,4}; //level 0 is a dummy
const unsigned char levelsnakes[] = {0,0,1,2,3};

     
int main (void)
{
	#ifdef GAMEJOLTBUILT
	   /* the arguments must be adjjusted based on what is actually stored at 
	    * offset 0x0801 in the game file */
	   poke(4096u,0x0B);
	   poke(4097u,0xA9);
	   poke(4098u,0x87);
	#endif
    init();
    
    while(1) {
	    titlescreen();
	    clrscr();
	    lives=MAXLIVES;
	    level=4;  //DEBUG
	    while(lives>0 && level<5)
	    	game_loop();
	    if (lives==0) {
	    	displayTitle(gameoverData,2,sizeof gameoverData);
			blinkwait(5,2);		
		}
	    if (level==5) {
			displayTitle(winningData,1,sizeof winningData);
			blinkwait(10,14);
		}
	}
    return EXIT_SUCCESS;
}

void displayinfo()
{
     gotoxy(0,0);
     cprintf("                                       \r Level:%d  Sheep left:%d  Time:%2d  Dogs:%d",level,nsheepleft,timer / TIMEUNIT,lives);
     announceexpire=0;
}

void announce(char *text)
{
     gotoxy(0,0);
     cprintf("                                       \r%s",text);
     announceexpire=timer-2*TIMEUNIT;
}


void game_loop(void)
{
     int i,si;
     int mx=172,my=208;
     int barnx,barny;
     unsigned char malpha=0;
     unsigned char animation=0;
     unsigned char costum0=32;     
     unsigned char mcostum0=34;
     unsigned char mdir=0;
     unsigned char mmode=0;
     unsigned int respawn=0;
     unsigned int soundptr=0xFFFF;
     int sheepx[7],sheepy[7],sheepa[7],sheepst[7];
     int snakex[7],snakey[7],snakea[7];
     unsigned char angle, gotobarn=0;
     unsigned char nsheep=levelsheep[level];
     unsigned char nsnakes=levelsnakes[level];     	
     unsigned char deathbysnake=0;

     timer=LEVELTIME;
     nsheepleft=nsheep;
     barnx=3+random8 % 35;
     barny=5+random8 % 19;
		      
     clrscr();
     textcolor (COLOR_BLACK);
     poke(53269u,0); //hide all sprites
     i=peek(53278u);
     announceexpire=0;

     drawGameScreen();
     initSprites(nsheep,nsnakes);
     setSpriteXY(0,mx,my);
     showSprite(0);  
          
     displayinfo();
     if ((level==1) && (lives==MAXLIVES))
     	announce(" Press space to control dog direction!");
     if (level==2)                    
        announce(" Beware of the snake!");
     if (level==3)                         
        announce(" Oh no! more snakes!"); 
    
     //place sheep       
     for(si=0;si<nsheep;si++) {
        sheepx[si]=60+random8;
        sheepy[si]=56+(random8 & 0xBF);
        sheepa[si]=random8;
        sheepst[si]=0;
        setSpriteXY(si+1,sheepx[si],sheepy[si]);
        showSprite(si+1);
     }
     
     //place snakes       
     for(si=nsheep;si<nsheep+nsnakes;si++) {
        snakex[si]=60+random8;
        snakey[si]=56+(random8 & 0xBF);
        snakea[si]=random8;
        setSpriteXY(si+1,snakex[si],snakey[si]);
        showSprite(si+1);
     }
     
     while(1) {
     	unsigned char key_space;
     	unsigned char collision;
	unsigned char colkey;
		 
        //read Space key
 	disable_int;
        poke (0xDC00, 0x7F);
        key_space = 0x10 - (peek(0xDC01)&0x10);
        enable_int;
        
        //dog control
        if (key_space)
           malpha+=4;
        else
           malpha-=4;
        
        mx += sintable[malpha]>>6;
        my -= costable[malpha]>>6;
        
        //bounce off borders
        if (my<56) {
           malpha=128-malpha;
           my=56;
           }
        else if (my>232) {
           malpha=128-malpha;
           my=232;
           }
        
        if (mx<16) {
           malpha=-malpha;
           mx=16;
           }
        else if (mx>324) {
           malpha=-malpha;
           mx=324;
           }
           
        setSpriteXY(0,mx,my);
        SPRITECOSTUM[0]=DOG+(malpha>>5)+(animation&0x01)*8;
        
        //collision detection
        collision=peek(53278u); //Sprite-Sprite collision
        
        if (gotobarn) {
           int dx=mx-barnx;
           int dy=my-barny;
           if (dx<0) dx=-dx;
           if (dy<0) dy=-dy;
           if (dx+dy<12) { //check if level is won
              announce("Good job!");
              level++;
              if (level < 5) blinkwait(3,5);
              break;
           }
        }
        
        //sheep control
        for(si=0;si<nsheep;si++) {
           if (sheepst[si]==0) {//sheep is uncaught
              
              if ((animation & 0x07)==si) { //load balancing with sprite number
                angle=sheepa[si] + 15-(random8 & 31);
                sheepx[si] += sintable[angle]>>7;
                sheepy[si] -= costable[angle]>>7;
                if (sheepx[si]>320) 
                	angle = angle | 0x80;
                else if (sheepx[si]<16)
                	angle = angle & 0x7f;
                if (sheepy[si]<56) {
           		angle=128-angle;
           		sheepy[si]=56;
           	}
                else if (sheepy[si]>232) {
           		angle=128-angle;
           		sheepy[si]=232;
           	}            	
                sheepa[si]=angle;	
                setSpriteXY(si+1,sheepx[si],sheepy[si]);
                SPRITECOSTUM[si+1]=SHEEP+((animation & 0x10)>>4)+((angle&0x80)>0)*2;
              }
              colkey=(2<<si)+1;
              if (((collision & colkey) == colkey)) {
                int dx=sheepx[si]-mx;
                int dy=sheepy[si]-my;
                if (dx<0) dx=-dx;
                if (dy<0) dy=-dy;
                if (dx+dy<8) { //sheep is caught
                  nsheepleft--;
                  if (announceexpire==0) SCREEN_RAM[21]=48+nsheepleft;
                  if (nsheepleft==0) {
                    drawBarn(barnx,barny);
                    barnx=barnx*8+12;
                    barny=barny*8+40;
                    gotobarn=1;
                    announce("Now go into the barn!");
                    //setSpriteXY(0,barnx,barny);
                    //blinkwait(1,9);
                    //setSpriteXY(0,mx,my);
                  }
                  sheepst[si]=1;
                  sheepx[si] = dx;
                  sheepy[si] = dy;
                }
	      }              
           }
           else { //velcro'ed sheep
              setSpriteXY(si+1,mx+sheepx[si],my+sheepy[si]);
              SPRITECOSTUM[si+1]=SHEEP+((malpha&0x080)>0)*2;
           }
        }
        
        //snake control
        for(si=nsheep;si<nsheep+nsnakes;si++) {
            if ((animation & 0x07)==3) { //load balancing with sprite number
                angle=snakea[si] + 7-(random8 & 15);
                snakex[si] += sintable[angle]>>7;
                snakey[si] -= costable[angle]>>7;
                if (snakex[si]>320) 
                	angle = angle | 0x80;
                else if (snakex[si]<16)
                	angle = angle & 0x7f;
                if (snakey[si]<56) {
           		angle=128-angle;
           		snakey[si]=56;
           	}
                else if (snakey[si]>232) {
           		angle=128-angle;
           		snakey[si]=232;
           	}            	
                snakea[si]=angle;	
                setSpriteXY(si+1,snakex[si],snakey[si]);
                SPRITECOSTUM[si+1]=SNAKE+((animation & 0x10)>>4)+((angle&0x80)>0)*2;
            }
            colkey=(2<<si)+1;
            if (((collision & colkey) == colkey)) {
                int dx=snakex[si]-mx;
                int dy=snakey[si]-my;
                if (dx<0) dx=-dx;
                if (dy<0) dy=-dy;
                if (dx+dy<10) { //snake got us
                   deathbysnake=1;
                   break;
                }
            }
        } // end for loop snake control
        if (deathbysnake) {
              --lives;
              announce("Ouch! That snake was poisonous!");
              blinkwait(1,13);
              break;
        }
	 
        //delay loop
        for(i=1;i<5*DELAY;i++) ;
         
        if ((--timer & (TIMEUNIT-1)) == (TIMEUNIT-1)) {
           unsigned int seconds=timer / TIMEUNIT;
           
           if (announceexpire>=timer)               
                   displayinfo();
           if (announceexpire==0) {
               //SCREEN_RAM[22]=48+seconds/100;
               SCREEN_RAM[29]=48+(seconds/10)%10;
               SCREEN_RAM[30]=48+seconds%10;
           }

           //or lost
           if (seconds==0) {
              --lives;
              displayinfo();
              blinkwait(1,2);
              break;
           }
        }
         
         	          
         if (soundptr < 0xFFFF) {
         	while(1) {
         		if (sounds[soundptr]==99) {
         			soundptr+=2;
         			break;
				 }
         		if (sounds[soundptr]==100) {
         			soundptr=0xFFFF;
         			break;
				 }
				poke(54272u+sounds[soundptr],sounds[soundptr+1]);
				soundptr+=2;
			 }
		 }
		 
         animation++;
     } //while(1) game loop
}



void init(void)
{
    unsigned int i;
    char *pDest;
    clrscr();
    cprintf("Copying charset...");
    copy_charset();
    cprintf("done\r\n");
    cprintf("Switch charset...");
    setVICbank(0); 
    setVICscreen(0);
    setVICcharset(3);
    clrscr();
    cprintf("Copying charset...done\r\n");
    cprintf("Switch charset...done\r\n");
    cprintf("Installing sprites...");
    pDest=SPRITE_RAM;
    for(i = 0; i < sizeof(spriteData); ++i)
		{
			pDest[i] = spriteData[i];
		}
    cprintf("done\r\n"); 
}

void initSprites(unsigned char nsheep, unsigned char nsnakes)
{
    int i;
    setSpriteColor(0,8);
    setSpriteColor(1,1);
    setSpriteColor(2,0);
    setSpriteColor(3,12);
    setSpriteColor(4,11);
    setSpriteColor(5,1);
    setSpriteColor(6,1);
    //setSpriteColor(7,1);  
    
    setSpriteMultiColor1(0);
    setSpriteMultiColor2(15);
    
    SPRITECOSTUM[0]=DOG;
    SPRITECOSTUM[1]=SHEEP;   
    SPRITECOSTUM[2]=SHEEP;  
    SPRITECOSTUM[3]=SHEEP;   
    SPRITECOSTUM[4]=SHEEP;    
    SPRITECOSTUM[5]=SHEEP;  
    SPRITECOSTUM[6]=SHEEP;   
    SPRITECOSTUM[7]=SHEEP;    
             
    poke(53276u,255); //all sprites are multicolor    
    
    //add snakes by overwriting some sheep
    for (i=nsheep;i<nsheep+nsnakes;i++) {
    	setSpriteColor(i+1,13);  
    	SPRITECOSTUM[i+1]=SNAKE;
    }
    
    setSpriteColor(7,1); //the last animal is white
}

void spriteRun(void)
{
    unsigned int x,i;
    unsigned char y,animation;
    initSprites(1,0);
    showSprite(1);
    #define CATCHX 120
 
    y=215;
    animation=0;
    for (x=0; x<110+CATCHX; x++) {
       if (x>CATCHX) {
         showSprite(0);
         setSpriteXY(0,(x-CATCHX)*3,y);
         SPRITECOSTUM[0]=DOG+2+(animation & 0x04)*2;
       }
       if (x+x<3*CATCHX) {
         setSpriteXY(1,x,y);
         SPRITECOSTUM[1]=SHEEP+((animation & 0x08)>0);
       }
       else
         setSpriteXY(1,(x-CATCHX)*3,y-2);
       if (joystick_fire(2) || peek(197)==60 || startgame) { startgame++; break; }
       for(i=1;i<500;i++) ;
       animation++;
    }
}

void titlescreen(void)
{
    clrscr();
    startgame=0;
    poke(53269u,0); //hide all sprites
    poke(53280u,0); //border color is black
    poke(53281u,0); //background color is black
    /* Set screen colors */
    (void) textcolor (COLOR_WHITE);
    (void) bordercolor (COLOR_BLACK);
    (void) bgcolor (COLOR_BLACK);
    
    #ifndef DEBUG
    setCharacterSingleColor;
    gotoxy(0,13);
    (void) textcolor (7);
    printslowly (" Be VELCRO, world's best shepherd dog!");
    (void) textcolor (8);
    printslowly ("\r\n\nVelcro has been born with a unique\r\nmutation: His fur has tiny hooks,\r\nwhich attach to a woolly sheep on\r\ncontact. This way, Velcro can gather\r\nthe sheep faster than any other dog!");
    displayTitle(titleData,8,sizeof titleData);
    (void) textcolor (3);
    gotoxy(0,23);
    cprintf("                    \r\nby Wilfried Elmenreich for bitbitjam'17");
    spriteRun();
    #endif
}

void printslowly(char *s)
{
    unsigned int i; 
    while(*s) {
       cprintf("%c",*s++);
       if (joystick_fire(2) || peek(197)==60 || startgame) { startgame++; break; }
       for(i=0;i<2*DELAY;i++) ;
    }
}


void copy_charset(void)
{
     unsigned int i;
     unsigned char *pDest;
     
     disable_int;
     poke(1U, peek(1U) & 251); //show character ROM instead of I/O area
     for (i=53248u+2048; i<=57343u;i++) {
     	//poke(i,peek(i)&0x3C); //make the characters slimmer
     	if (i<56024u) //everything including Z
     	    //poke(i,peek(i)&0x3C); //make the characters slimmer
     	    if (i & 4) // lower half of character
     	    	poke(i,peek(i)<<1); //shift bits to the left
     	else
	    poke(i,peek(i));
	 }

     //add custom characters beginning with index 128
     pDest=(char*)53248u+3072;
     for (i=0; i<sizeof(charsetData); i++)
         pDest[i]=charsetData[i];

     poke(1U, peek(1U) |4); //enable I/O area again
     enable_int;
}

void drawGameScreen()
{
     char *pScreen=SCREEN_RAM+40;
     char *pColor=COLOR_RAM+40;
     unsigned int i;
     
     poke(53281u,7); //background color is yellow
     for(i=0;i<1024-40;i++) {
        pColor[i]=5; //green
     	if ((random8 & 0x07) == 0) {
     	   if ((random8^i) & 0x01) 
     	      pScreen[i]=FLOWER;
     	   else
     	      pScreen[i]=FLOWER+1;
     	}
     	else {
     	   pScreen[i]=FILLED;
     	   
     	}
     }
}

void drawBarn(int x,int y)
{
     int i;
     char *pScreen=SCREEN_RAM+40*y-160+x-2;
     char *pColor=COLOR_RAM+40*y-160+x-2;
     for (i=0;i<4;i++) {
        pScreen[i]=ROOF;
        pColor[i]=2;
        pScreen[i+40]=ROOF;
        pColor[i+40]=2;
        pScreen[i+80]=BARN;
        pColor[i+80]=9;  
        pScreen[i+120]=BARN;
        pColor[i+120]=9;     
        pScreen[i+160]=BARN;
        pColor[i+160]=9;                     
     }
     pScreen[121]=FILLED;
     pColor[121]=0;     
     pScreen[122]=FILLED;
     pColor[122]=0;     
     pScreen[161]=FILLED;
     pColor[161]=0;                     
     pScreen[162]=FILLED;
     pColor[162]=0;     
}
