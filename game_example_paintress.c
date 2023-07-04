//#include <stdio.h>
#include <stdlib.h>
#include <conio.h>

//#define DEBUG

#include "frodosC64lib.c"

/*-----------------------------------------------------------------------*/
#define VIC_BASE_RAM			(0xC000)
#define	SCREEN_RAM				((unsigned char*)VIC_BASE_RAM+0x0000)
#define SPRITEDATAPTRS			((unsigned char*)SCREEN_RAM+0x03f8)
#define CHARMAP_RAM				((unsigned char*)VIC_BASE_RAM+0x1000)
#define	SPRITE_RAM				((unsigned char*)VIC_BASE_RAM+0x0800)
#define LVL_SPRITE_RAM			((unsigned char*)VIC_BASE_RAM+0x3200)
#ifndef COLOR_RAM
#define COLOR_RAM				((unsigned char*)0xd800)
#endif
#define DELAY 50

#include "paintress-assets.inc"

void copy_charset(void);
void titlescreen(void);
void init(void);
void printslowly(char *s);
void spriteRun(void);
void drawTiledScreen(const unsigned char *tileset, const unsigned char *map);
void game_loop(void);
void initSprites(void);
void initPaintData(void);
void showPaintData(unsigned char segment);

unsigned char lives, level, startgame;
unsigned char paintBuffer[1000];
unsigned char *paintData = paintBuffer + 40;	//to be on the safe side
#define SEGMENTS 32
unsigned char cleared[SEGMENTS];
const int dirs[] = { -40, 1, 40, -1 };
const unsigned char spritecolors[] = { 10, 13, 14 };
const unsigned char levelcolors[] = { 2, 5, 6 };
unsigned char paintcolor;

int main(void) {

  init();

  while (1) {
    titlescreen();
    clrscr();
    lives = 3;
    level = 1;
    while (lives > 0 && level < 4)
      game_loop();
    if (lives == 0) {
      displayTitle(gameoverData, 2);
      blinkwait(5, 2);
    }
    if (level == 4) {
      displayTitle(winningData, 1);
      blinkwait(10, 14);
    }
  }
  return EXIT_SUCCESS;
}

#define field(x,y) (40*((y-30)/8)+(x-17)/8)
#define STROLL 0
#define FLEE 1
#define PURSUIT 2 
#define LEVELTIME 201*128
void game_loop(void) {
  int i;
  int px = 172, px2, py = 208, py2;
  int mx = 178, mx2, my = 82, my2;
  unsigned char animation, manimation;
  unsigned char costum0 = 32;
  unsigned char joystick;
  unsigned char mcostum0 = 34;
  unsigned char mdir = 0;
  unsigned char mmode = 0;
  const int dx[] = { 0, 1, 0, -1 };
  const int dy[] = { -1, 0, 1, 0 };
  int mdist;
  unsigned char hitanimation, hitaction = 0;
  unsigned int timer = LEVELTIME;
  unsigned int respawn = 0;
  unsigned int soundptr = 0xFFFF;

  clrscr();
  textcolor(COLOR_WHITE);
  poke(53269u, 0);		//hide all sprites
  poke(53280u, 0);
  i = peek(53278u);
  cprintf("Level: %d        Time: 99        Lives: %d", level, lives);

  drawTiledScreen((const unsigned char *)tilesetData,  (unsigned char*)(levelmaps + (3 - level) * (20 * 12)));
//  drawTiledScreen(tilesetData, levelmaps + (3 - level) * (20 * 12));  
  initPaintData();

  initSprites();

  paintcolor = levelcolors[level - 1];
  setSpriteColor(4, spritecolors[level - 1]);

  setSpriteXY(0, px, py);
  setSpriteXY(4, mx, my);
  showSprite(0);
  showSprite(4);
  //SCREEN_RAM[0]=peek(53278u);

  //delay loop
  //blinkwait(1);

  while (1) {
    unsigned char key_w, key_a, key_s, key_d, key_space;
    unsigned char collision;

    //debug code
#ifdef DEBUG
    //gotoxy (0,0);
    //cprintf("%d %d %d",px,py,peek(56321u));das
#endif
    manimation++;

    //player control
    joystick = peek(56320u);

    //read WASD Space keys
    disable_int;
    poke(0xDC00, 0xFD);
    key_w = 0x02 - (peek(0xDC01) & 0x02);
    key_s = 0x20 - (peek(0xDC01) & 0x20);
    key_a = 0x04 - (peek(0xDC01) & 0x04);
    poke(0xDC00, 0xFB);
    key_d = 0x04 - (peek(0xDC01) & 0x04);
    poke(0xDC00, 0x7F);
    key_space = 0x10 - (peek(0xDC01) & 0x10);
    enable_int;

    px2 = px;
    py2 = py;
    if ((joystick & 0x01) == 0 || key_w) {
      py2 = py - 1;
      animation++;
    }
    if ((joystick & 0x02) == 0 || key_s) {
      py2 = py + 1;
      animation++;
    }
    if ((joystick & 0x04) == 0 || key_a) {
      px2 = px - 1;
      animation++;
      costum0 = PRINCESSLEFT;
    }
    if ((joystick & 0x08) == 0 || key_d) {
      px2 = px + 1;
      animation++;
      costum0 = PRINCESSRIGHT;
    }
    if ((*(SCREEN_RAM + field(px2, py2)) >= WALKABLE)
	&& (*(SCREEN_RAM + field(px2 + 8, py2)) >= WALKABLE)) {
      //*(COLOR_RAM+field(px,py))=TILECOLOR;
      px = px2;
      py = py2;
      SPRITEDATAPTRS[0] = costum0 + ((animation & 16) >> 4);
      setSpriteXY(0, px, py);
      //*(COLOR_RAM+field(px,py))=1;
    }
    //sword
    if ((joystick & 0x10) == 0 || key_space)
      if (hitaction == 0) {
	hitaction = 4;
	animation = 0;
	hitanimation = 0;
	SPRITEDATAPTRS[0] = costum0;
      }
    if (hitaction) {
      if (costum0 == PRINCESSRIGHT)
	setSpriteXY(1, px + 9, py - 1);
      else
	setSpriteXY(1, px - 9, py - 1);
      SPRITEDATAPTRS[1] = costum0 + 8 - hitaction;
      showSprite(1);
      if (hitanimation++ >= 5) {
	hitaction--;
	hitanimation = 0;
      }
      //SCREEN_RAM[0]=hitaction; 
      if (hitaction == 0)
	hideSprite(1);
    }
    //monster control
    mdist = abs(px - mx) + abs(py - my);
    if (mmode == FLEE) {
      if (mdist > 100)
	mmode = STROLL;
      else if (manimation & 0x01) {
	if (px > mx)
	  mdir = 3;
	else if (px < mx)
	  mdir = 1;
      } else {
	if (py > my)
	  mdir = 0;
	else if (py < my)
	  mdir = 2;
      }
    }
    if (mmode == PURSUIT) {
      if (mdist > 100)
	mmode = STROLL;
      else {
	if (manimation & 0x01) {
	  if (px > mx)
	    mdir = 1;
	  else if (px < mx)
	    mdir = 3;
	} else {
	  if (py > my)
	    mdir = 2;
	  else if (py < my)
	    mdir = 0;
	}
      }				//else mdist > 100d
    }				// if PURSUIT
    if (mmode == STROLL) {
      if (mdist < 50)
	mmode = PURSUIT;
    }
    mx2 = mx + dx[mdir];
    my2 = my + dy[mdir];
    if (mdir == 1)
      mcostum0 = MONSTERRIGHT;
    else if (mdir == 3)
      mcostum0 = MONSTERLEFT;

    if ((*(SCREEN_RAM + field(mx2, my2 - 2)) >= WALKABLE)
	&& (*(SCREEN_RAM + field(mx2 + 12, my2 - 2)) >= WALKABLE)) {
      mx = mx2;
      my = my2;
      //*(COLOR_RAM+field(mx+4,my))=random8();
      setSpriteXY(4, mx, my);
      SPRITEDATAPTRS[4] = mcostum0 + ((manimation & 16) >> 4);
    } else
      mdir = (mdir + 1 + (random8() & 0x02)) & 0x03;

    //collision detection
    collision = peek(53278u);	//Sprite-Sprite collision
    if ((collision & 0x12) == 0x12) {
      //sword hit monsters
      i = field(mx2 + 6, my2 - 2) - 6 * 40;
      if (SCREEN_RAM[i] < WALKABLE)
	i += 40;
      paintData[i] += 120;
      (SCREEN_RAM + (6 * 40))[i] = PAINT;
      (COLOR_RAM + (6 * 40))[i] = paintcolor;
      hideSprite(4);
      respawn = timer - 200;
      soundptr = SOUND_HIT;
      //level++;
      //if (level < 4) blinkwait(6);
      //break;
    } else if ((collision & 0x11) == 0x11) {
      //monster caches player
      if (--lives == 0)
	break;
      timer = LEVELTIME;
      SCREEN_RAM[39] = 48 + lives;
      respawn = timer - 10;
      hideSprite(4);
      px = 172, py = 208;
      blinkwait(1, 2);
    }
    //delay loop
    //for(i=1;i<50;i++) ;

    showPaintData(timer & (SEGMENTS - 1));

    if (timer == respawn) {
      //two possible respawn points, choose the one farer away from player
      mx = 178;
      if (py > 145)
	my = 82;
      else
	my = 208;
      mmode = STROLL;
      setSpriteXY(4, mx, my);
      showSprite(4);
    }
#ifdef DEBUG
    gotoxy(0, 0);
    cprintf("                ");
    gotoxy(0, 0);
    cprintf("%d %d", timer, respawn);
#endif

    if ((--timer & 0x7F) == 0x7F) {
      unsigned int seconds = timer / 128, clearedsegments = 0;
      SCREEN_RAM[22] = 48 + seconds / 100;
      SCREEN_RAM[23] = 48 + (seconds / 10) % 10;
      SCREEN_RAM[24] = 48 + seconds % 10;

      //check if game is won
      for(i = 0; i < SEGMENTS; i++)
	clearedsegments += cleared[i];
      if (clearedsegments == SEGMENTS) {
	level++;
	if (level < 4)
	  blinkwait(2, 6);
	break;
      }
      //or lost
      if (seconds == 0) {
	if (--lives == 0)
	  break;
	timer = LEVELTIME;
	SCREEN_RAM[39] = 48 + lives;
	respawn = timer - 10;
	hideSprite(4);
	px = 172, py = 208;
	blinkwait(1, 2);
      }
    }
    if (soundptr < 0xFFFF) {
      while (1) {
	if (sounds[soundptr] == 99) {
	  soundptr += 2;
	  break;
	}
	if (sounds[soundptr] == 100) {
	  soundptr = 0xFFFF;
	  break;
	}
	poke(54272u + sounds[soundptr], sounds[soundptr + 1]);
	soundptr += 2;
      }
    }
  }				//while(1) game loop
}



void init(void) {
  unsigned int i;
  unsigned char *pDest;
  clrscr();
  cprintf("Copying charset...");
  copy_charset();
  cprintf("done\r\n");
  cprintf("Switch charset...");
  setVICbank(3);
  setVICscreen(0xc000);
  setVICcharset(0xd800);
  poke(648,192);
  clrscr();
  cprintf("Copying charset...done\r\n");
  cprintf("Switch charset...done\r\n");
  cprintf("Installing sprites...");
  pDest = (unsigned char *)SPRITE_RAM;
  for(i = 0; i < sizeof(spriteData); ++i) {
    pDest[i] = spriteData[i];
  }
  cprintf("done\r\n");
  cprintf("Enable multi-color character mode...");
  poke(53270u, peek(53270u) | 16);
  poke(53282u, 12);		//multicolor 1 is middle gray
  poke(53283u, 9);		//multicolor 2 is brown    
}

void initSprites(void) {
  setSpriteColor(0, 14);
  setSpriteColor(1, 1);
  setSpriteColor(4, 10);
  setSpriteMultiColor1(7);
  setSpriteMultiColor2(10);
  SPRITEDATAPTRS[0] = 32;
  poke(53276u, 1);		//only sprite 1 is multicolor    
}

void spriteRun(void) {
  unsigned int x, i;
  unsigned char y;
  initSprites();
  showSprite(0);
  y = 210;
  for(x = 0; x < 345; x++) {
    SPRITEDATAPTRS[0] = 32 + ((x >> 2) & 1);
    setSpriteXY(0, x, y);
    if (joystick_fire(2) || peek(197) == 60 || startgame) {
      startgame++;
      break;
    }
    for(i = 1; i < 250; i++);
  }
}

void titlescreen(void) {
  clrscr();
  startgame = 0;
  poke(53269u, 0);		//hide all sprites
  /* Set screen colors */
  (void) textcolor(COLOR_WHITE);
  (void) bordercolor(COLOR_BLACK);
  (void) bgcolor(COLOR_BLACK);

#ifndef DEBUG
  printslowly
    ("\r\n\n\n\n\n\n\n\n\n You are a princess, but even so you\r\n sometimes have to do the dirty work.\r\n\n One day, the mad king (aka your father) orders you to go to the dungeon and\r\n REPAINT it...\r\n\n");
  //(void) textcolor (COLOR_LIGHTRED);
  (void) textcolor(COLOR_RED);
  printslowly(" ...with the blood of the monsters!!!");
  displayTitle(titleData, 2);
  (void) textcolor(3);
  gotoxy(0, 24);
  cprintf("by Wilfried Elmenreich for bitbitjam'16");
  spriteRun();
#endif
}

void printslowly(char *s) {
  unsigned int i;
  while (*s) {
    cprintf("%c", *s++);
    if (joystick_fire(2) || peek(197) == 60 || startgame) {
      startgame++;
      break;
    }
    for(i = 0; i < 2 * DELAY; i++);
  }
}


void copy_charset(void) {
  unsigned int i;
  unsigned char *pDest;

  disable_int;
  poke(1U, peek(1U) & 251);	//show character ROM instead of I/O area
  for(i = 53248u + 2048; i <= 57343U; i++) {
    //poke(i,peek(i)&0x3C); //make the characters slimmer
    poke(i, peek(i));
  }


  //add costum characters beginning with index 128
  pDest = (unsigned char *) 53248u + 3072;
  for(i = 0; i < sizeof(charsetData); i++)
    pDest[i] = charsetData[i];

  poke(1U, peek(1U) | 4);	//enable I/O area again
  enable_int;
}

void initPaintData(void) {
  unsigned int i;
  unsigned char *pScreen = SCREEN_RAM + (6 * 40);

  for(i = 0; i < 1000; i++) {
    paintBuffer[i] = 0xFF;
  }
  for(i = 0; i < 17 * 40; i++) {
    if (pScreen[i] >= WALKABLE) {
      paintData[i] = 0;
      //pScreen[i]=PAINT;
    } else {
      paintData[i] = 0xFF;
    }
  }
  for(i = 0; i < SEGMENTS; i++)
    cleared[i] = 0;
}

void showPaintData(unsigned char segment) {
  unsigned int i;
  unsigned char *pScreen = SCREEN_RAM + (6 * 40);
  unsigned char *pColor = COLOR_RAM + (6 * 40);

  cleared[segment] = 1;
  for(i = segment; i < 17 * 40; i += SEGMENTS) {
    unsigned char data = paintData[i];
    if (data == 0) {
      cleared[segment] = 0;
    } else {
      if (data < 255) {
	//pScreen[i]=PAINT;
	//pColor[i]=paintcolor;
	if (data > 1) {
	  int neighbor = i + dirs[random8() & 0x03];
	  //pColor[i]=6;
	  if (data > paintData[neighbor] + 1) {
	    paintData[neighbor]++;
	    paintData[i]--;
	    pScreen[neighbor] = PAINT;
	    pColor[neighbor] = paintcolor;
	  }
	}
      }
    }				// else
  }				// for
}

void drawTiledScreen(const unsigned char *tileset, const unsigned char *map) {
  unsigned char *pScreen = SCREEN_RAM + 40;
  unsigned char *pColor = COLOR_RAM + 40;
  const unsigned char *tile;
  char tx, ty, x, y;
  unsigned int offset;
  char h = 25 / TILEHEIGHT;
  char w = 40 / TILEWIDTH;
  for(ty = 0; ty < 25 / TILEHEIGHT; ty++)
    for(tx = 0; tx < 40 / TILEWIDTH; tx++) {
      //cprintf ("read %d=%d, ",tx+ty*w,map[tx+ty*w]);
      tile = &tileset[TILEHEIGHT * TILEWIDTH * map[tx + ty * w]];
      for(y = 0; y < TILEHEIGHT; y++)
	for(x = 0; x < TILEWIDTH; x++) {
	  offset = tx * TILEWIDTH + x + 40 * (ty * TILEHEIGHT + y);
	  //cprintf ("off=%d ",offset);
	  pScreen[offset] = tile[x + y * TILEWIDTH] + 0x80;
	  pColor[offset] = TILECOLOR;
	}
    }
}
