#include <TVout.h>
#include <fontALL.h>
#include "gfx.h"

// PIN 9 - 1k ohm
// PIN 7 - 470 ohm

#define DEBUG            1
#define BUTTON_A         2
#define BUTTON_B         3
#define SPRITE_ATIME    40
#define MAX_GM_ACCLRN    7
#define SWITCH_ACCLRN  500
#define SZY_JUMP_H      32
#define SZY_JUMP_OFFST   4
#define ENEMY_TYPES_CNT  3
#define TUTOR_MODE_CNT  80
#define COLLISION_OFFST  1

TVout TV;
int i;
unsigned long currentTime;
unsigned long prevTime    = 0;
unsigned char ninjaSprite = 0;
unsigned char gmspeed     = 1;
unsigned int acclrn       = 0;
unsigned int score        = 0;
unsigned int hiscore      = 0;
char gmstarted            = 0;
char isgmover             = 0;
int btnreadA;
int btnreadB;

struct Ninja {
  unsigned char x;
  unsigned char y;
  unsigned char w;
  unsigned char h;
  unsigned char jump;
  unsigned char fall;
  unsigned char crawl;
};

struct Enemy {
  char x;
  char y;
  char w;
  char h;
  int type; // -1, 0, 1, 2 ...
};

struct Cloud {
  unsigned char x;
  unsigned char y;
};

Ninja sayzo;
Enemy enemy;
Cloud cld1;
Cloud cld2;

void drawWorld();
void drawSayzo();
void drawEnemy();
void updateScore();
int generateEnemy();
char detectCollision();
void gmreset();
void gmover();

void setup() {

  //Serial.begin(9600);
  randomSeed(analogRead(A0));
  pinMode(BUTTON_A, INPUT);
  pinMode(BUTTON_B, INPUT);
  
  TV.begin(PAL, 128, 96);
  TV.select_font(font6x8);
  TV.println("Ready\n");
  delay(2000);
  TV.println("2017 Dmitri Smirnov\n");
  TV.println("http://www.whoop.ee\n");
  delay(5000);
  TV.bitmap(0, 0, intro);
  gmreset();
}

void loop() {

  //TV.draw_rect(0, 0, 127, 95, 1);

  btnreadA = digitalRead(BUTTON_A);
  btnreadB = digitalRead(BUTTON_B);

  if (btnreadA == HIGH || btnreadB == HIGH) {
    if (isgmover == 1) {
      gmreset();
      isgmover = 0;
    }
    gmstarted = 1;
  }
  
  if (gmstarted) {
    TV.delay_frame(3);
    TV.clear_screen();

    if (btnreadA == HIGH && sayzo.jump == 0) {
      sayzo.jump = 1;
    }
    if (btnreadB == HIGH && btnreadA != HIGH) {
      sayzo.crawl = 1;
    } else {
      sayzo.crawl = 0;
    }

    if (enemy.x < 0) {
      generateEnemy();
    }
    
    drawWorld();
    drawEnemy();
    drawSayzo();
    
    // Game over
    if (detectCollision() == 1) {
      TV.bitmap(sayzo.x, sayzo.y, ninja[3]);
      gmover(); 
      return;
    }

    // Accelerate speed. Longer you play game go faster.
    if (score % SWITCH_ACCLRN == 0 && acclrn <= MAX_GM_ACCLRN) {
      acclrn++;
    }
    gmspeed += acclrn;

    updateScore();
  } // end of game started if
  
} // end of loop

void gmreset() {
  
  TV.select_font(font4x6);

  gmspeed = 1;
  acclrn  = 0;
  score   = 0;

  sayzo.w = 16;
  sayzo.h = 16;
  sayzo.x = 16;
  sayzo.y = 64;
  sayzo.jump = 0;
  sayzo.fall = 0;
  sayzo.crawl = 0;
  
  cld1.x = 8;
  cld1.y = 1;
  cld2.x = 67;
  cld2.y = 12;

  enemy.x = TUTOR_MODE_CNT;
  enemy.y = 0;
  enemy.w = 0;
  enemy.h = 0;
  enemy.type = -1;
}
void drawSayzo() {
  currentTime = millis();
  if (currentTime - prevTime >= SPRITE_ATIME) {
    prevTime = currentTime;

    // Set height 8 only for crawl mode.
    sayzo.h = 16;
    
    if (sayzo.y < 64 && sayzo.jump == 0) {
      sayzo.y += SZY_JUMP_OFFST;
      sayzo.fall = 1;
    } else if (sayzo.y > SZY_JUMP_H && sayzo.jump == 1) {
      sayzo.y -= SZY_JUMP_OFFST;
      sayzo.fall = 0;
    } else {
      sayzo.fall = 0;
      sayzo.jump = 0;
    }
    
    if (sayzo.jump == 1) {
      if (sayzo.y < SZY_JUMP_H) {
        sayzo.jump = 0;
      }
      TV.bitmap(sayzo.x, sayzo.y, ninja[2]);
    } else if (sayzo.crawl == 1) {
      sayzo.h = 8;
      TV.bitmap(sayzo.x, sayzo.y + 8, ninja[ninjaSprite++]);
      if (ninjaSprite > 6 || ninjaSprite < 4) {
        ninjaSprite = 4;
      }
    } else {
      TV.bitmap(sayzo.x, sayzo.y, ninja[ninjaSprite++]);
      if (ninjaSprite > 2) {
        ninjaSprite = 0;
      }
    }
  }
}

void drawEnemy() {
  enemy.x -= acclrn;
  switch (enemy.type) {
    case 0:
      TV.bitmap(enemy.x, enemy.y, enemy1);
      break;
    case 1:
      TV.bitmap(enemy.x, enemy.y, enemy2);
      break;
    case 2:
      TV.bitmap(enemy.x, enemy.y, enemy3);
      break;
    default:
      break;
  }
}

void drawWorld() {
  TV.bitmap(87, 9, moon);
  TV.bitmap(cld1.x - gmspeed, cld1.y, cloud);
  TV.bitmap(cld2.x - gmspeed, cld2.y, cloud);
  for (i = 128; i >= 0; i -= 16) {
    TV.bitmap(i - gmspeed, 80, ground);
  }
}

void updateScore() {
  TV.println(0, 0, score);
  score++;
  if (hiscore > 0) {
    TV.println(92, 0, "TOP ");
    TV.println(107, 0, hiscore);
  }
}

int generateEnemy() {
  int entype = random(ENEMY_TYPES_CNT);
  enemy.type = entype;
  switch (entype) {
    case 0:
      enemy.x = 112;
      enemy.y = 72;
      enemy.w = 16;
      enemy.h = 8;
      break;
    case 1:
      enemy.x = 112;
      enemy.y = 64;
      enemy.w = 16;
      enemy.h = 16;
      break;
    case 2:
      int subtype = random(4);
      if (subtype == 0) {
        enemy.y = 32;
      }
      else if (subtype == 1) {
        enemy.y = 40;  
      }
      else if (subtype == 2) {
        enemy.y = 48;  
      }
      else {
        enemy.y = 56;
      }
      enemy.x = 112;
      enemy.w = 16;
      enemy.h = 16;
      break;
   }

  return entype;
}

char detectCollision() {
  if (
      (sayzo.x + COLLISION_OFFST) < (enemy.x + enemy.w) - COLLISION_OFFST &&
      (sayzo.x + sayzo.w) - COLLISION_OFFST > (enemy.x + COLLISION_OFFST) &&
      (sayzo.y + COLLISION_OFFST) < (enemy.y + enemy.h) - COLLISION_OFFST &&
      (sayzo.y + sayzo.h) - COLLISION_OFFST > (enemy.y + COLLISION_OFFST)) {
        // Avoid in crawl mode enemy number 2.
        if (sayzo.crawl == 1 && enemy.type == 2 && sayzo.fall != 1 && sayzo.jump != 1) {
          return 0;
        } else {
          return 1;
        }
  } else {
    return 0;
  }
}

void gmover() {
  updateScore();
  TV.select_font(font8x8);
  TV.println(25, 30, "GAME OVER");
  gmstarted = 0;
  isgmover = 1;
  if (score > hiscore) {
    hiscore = score;
  }
  delay(3000);
}

