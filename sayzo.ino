#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "gfx.h"

#define OLED_RESET 4
Adafruit_SSD1306 display(OLED_RESET);

#define DEBUG            0
#define BUTTON_A_PIN     8
#define BUTTON_B_PIN     9
#define SPRITE_ATIME    40
#define MAX_GM_ACCLRN    7
#define SWITCH_ACCLRN  500
#define SZY_JUMP_H       0
#define SZY_JUMP_OFFST   4
#define ENEMY_TYPES_CNT  4
#define TUTOR_MODE_CNT  80
#define COLLISION_OFFST  1
#define FLAKES_NUM      30

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
char starsDrawn           = 0;
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
  int type; // -1, 0, 1, 2, 3 ...
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
void draw_snow(uint8_t count);

void setup() {

  Serial.begin(9600);
  randomSeed(analogRead(A0));
  pinMode(BUTTON_A_PIN, INPUT);
  pinMode(BUTTON_B_PIN, INPUT);
  
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.clearDisplay();
  display.setTextColor(WHITE, BLACK);
  display.setTextSize(1);

  display.setCursor(0, 0);
  display.println("Ready...\n");
  display.display();
  delay(2000);
  display.clearDisplay();
  display.setCursor(0, 0);
  display.println("2017-2018\nDmitri Smirnov\nAleksei Smirnov\n\nhttp://www.whoop.ee");
  display.display();
  delay(5000);
  display.clearDisplay();
  display.setCursor(0, 0);
  display.drawBitmap(0, 0, intro, 128, 64, WHITE);
  display.display();
  gmreset();
  draw_snow(FLAKES_NUM);
}

void loop() {

  btnreadA = digitalRead(BUTTON_A_PIN);
  btnreadB = digitalRead(BUTTON_B_PIN);

  if (btnreadA == HIGH || btnreadB == HIGH) {
    if (isgmover == 1) {
      gmreset();
      isgmover = 0;
    }
    gmstarted = 1;
  }
  
  if (gmstarted) {
    display.clearDisplay();

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
    

    // Accelerate speed. Longer you play game go faster.
    if (score % SWITCH_ACCLRN == 0 && acclrn <= MAX_GM_ACCLRN) {
      acclrn++;
    }
    gmspeed += acclrn;

    updateScore();
    drawWorld();
    drawEnemy();
    drawSayzo();

    // Game over
    if (detectCollision() == 1) {
      display.drawBitmap(sayzo.x, sayzo.y, ninja[3], 16, 16, WHITE);
      delay(40);
      gmover();
      return;
    }
    
    display.display();
  } // end of game started if
  
} // end of loop


void draw_snow(uint8_t count)
{
  for (i = 0; i < count; ++i) {
     display.drawPixel((uint16_t)random(display.width()), (uint16_t)random(display.height()/2), WHITE);
  }
  //display.display();
  starsDrawn = 1;
}
void gmreset() {
  
  gmspeed = 1;
  acclrn  = 0;
  score   = 0;

  sayzo.w = 16;
  sayzo.h = 16;
  sayzo.x = 16;
  sayzo.y = 32;
  sayzo.jump = 0;
  sayzo.fall = 0;
  sayzo.crawl = 0;
  
  cld1.x = display.width();
  cld1.y = 1;
  cld2.x = display.width() + 96 ;
  cld2.y = 4;

  enemy.x = 0;
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
    
    if (sayzo.y < 32 && sayzo.jump == 0) {
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
      display.drawBitmap(sayzo.x, sayzo.y, ninja[2], 16, 16, WHITE);
    } else if (sayzo.crawl == 1) {
      sayzo.h = 8;
      display.drawBitmap(sayzo.x, sayzo.y + 8, ninja[ninjaSprite++], 16, 16, WHITE);
      if (ninjaSprite > 6 || ninjaSprite < 4) {
        ninjaSprite = 4;
      }
    } else {
      display.drawBitmap(sayzo.x, sayzo.y, ninja[ninjaSprite++], 16, 16, WHITE);
      if (ninjaSprite > 2) {
        ninjaSprite = 0;
      }
    }
  }
}

void drawWorld() {
  if (score > 1500) {
    draw_snow(FLAKES_NUM);
  }
  display.drawBitmap(12, 16, mountains, 96, 16, WHITE);
  display.drawBitmap(cld1.x - gmspeed, cld1.y, cloud, 32, 16, WHITE);
  display.drawBitmap(cld2.x - gmspeed, cld2.y, cloud, 32, 16, WHITE);
  display.drawBitmap(97, 3, moon, 16, 16, WHITE);
  for (i = 384; i >= 0; i -= 16) {
    display.drawBitmap(i - gmspeed, display.height()-16, ground, 16, 16, WHITE);
  }
}

void updateScore() {
  display.setCursor(0, 0);
  display.println(score);
  score++;
  //if (hiscore > 0) {
  //  display.setCursor(80, 0);
  //  display.println(hiscore);
  //}
}

int generateEnemy() {
  int enemy2subtypes[4] = {0, 8, 12, 24};
  int subtype;
  enemy.type = random(ENEMY_TYPES_CNT);
  enemy.x = 112;
  switch (enemy.type) {
    case 0:
      enemy.y = 40;
      enemy.w = 16;
      enemy.h = 8;
      break;
    case 1:
      enemy.y = 32;
      enemy.w = 16;
      enemy.h = 16;
      break;
    case 3:
      enemy.y = 28;
      enemy.w = 16;
      enemy.h = 20;
      break;
    case 2:
      subtype = random(4);
      enemy.y = enemy2subtypes[subtype];
      enemy.w = 16;
      enemy.h = 16;
      break;  
    default:
      break; 
   }

  return enemy.type;
}

void drawEnemy() {
  enemy.x -= acclrn;
  switch (enemy.type) {
    case 0:
      display.drawBitmap(enemy.x, enemy.y, enemy1, 16, 8, WHITE);
      break;
    case 1:
      display.drawBitmap(enemy.x, enemy.y, enemy2, 16, 16, WHITE);
      break;
    case 2:
      display.drawBitmap(enemy.x, enemy.y, enemy3, 16, 16, WHITE);
      break;
    case 3:
      display.drawBitmap(enemy.x, enemy.y, enemy4, 16, 20, WHITE);
      break;
    default:
      break;
  }
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
  gmstarted = 0;
  starsDrawn = 0;
  isgmover = 1;
  if (score > hiscore) {
    hiscore = score;
  }
  display.setCursor(26, 24);
  display.println("GAME OVER");
  display.setCursor(12, 32);
  display.print("High score: ");
  display.print(hiscore);
  display.display();
  delay(3000);
}
