#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_W 128
#define SCREEN_H 64
#define OLED_RESET -1
Adafruit_SSD1306 display(SCREEN_W, SCREEN_H, &Wire, OLED_RESET);

#define BTN_PIN 18 // ปุ่มสั่งบิน (GPIO 18)

// --- [ปรับปรุงใหม่] ตัวแปรฟิสิกส์นก - บินนุ่มขึ้น ตกช้าลง ---
int birdX = 25;
float birdY = 32;       
float velocityY = 0;
const float GRAVITY = 0.22;       // ลดลงจาก 0.35 นกจะร่อนตกช้าลงอย่างนุ่มนวล
const float FLAP_STRENGTH = -2.6; // ลดลงจาก -3.8 กดแล้วจะค่อยๆ ลอยขึ้น ไม่พุ่งพรวด
bool wingUp = false; 

// --- ตัวแปรท่อ (Pipes) ---
float pipeX = 128;
int pipeGap = 24;       
int pipeGapY = 20;      
float pipeSpd = 1.8;   // ปรับความเร็วท่อให้ช้าลงนิดหน่อยเพื่อให้รับกับฟิสิกส์นกใหม่
const int PIPE_W = 12; 

// --- ตัวแปรสถานะเกม ---
int score = 0;
bool gameOver = false;
bool isNightMode = false; 

// --- บิตแมปดีไซน์นก (ขนาด 9x7 พิกเซล) ---
const unsigned char PROGMEM bird_wing_up[] = {
  0b01111000, 0b0,
  0b10000110, 0b0,
  0b11111010, 0b0,
  0b11011100, 0b0,
  0b10000000, 0b0,
  0b01111100, 0b0,
  0b00100000, 0b0
};
const unsigned char PROGMEM bird_wing_down[] = {
  0b01111000, 0b0,
  0b10000110, 0b0,
  0b11111010, 0b0,
  0b10011100, 0b0,
  0b11100000, 0b0,
  0b01111100, 0b0,
  0b00000000, 0b0
};

void setup() {
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { for(;;); }
  pinMode(BTN_PIN, INPUT_PULLUP);
  display.clearDisplay();
  display.display();
}

void loop() {
  if (!gameOver) {
    // 1. INPUT & ANIMATION 
    if (digitalRead(BTN_PIN) == LOW) {
      velocityY = FLAP_STRENGTH; 
      wingUp = true; 
      delay(40); // ป้องกันปุ่มเบิ้ล
    } else {
      if (velocityY > 0.3) {
        wingUp = false;
      }
    }

    // 2. PHYSICS & DAY/NIGHT LOGIC
    velocityY += GRAVITY;
    birdY += velocityY;
    
    // ชนขอบบนหรือตกขอบล่าง = ตาย
    if (birdY > 58 || birdY < 0) {
      gameOver = true;
    }

    // เลื่อนท่อจากขวาไปซ้าย
    pipeX -= pipeSpd;
    if (pipeX < -PIPE_W) {
      pipeX = 128;
      pipeGapY = random(10, 34); 
      score++;
      
      // ระบบสลับกลางวัน/กลางคืน ทุกๆ 5 คะแนน
      if (score > 0 && score % 5 == 0) {
        isNightMode = !isNightMode;
      }
    }

    // 3. COLLISION DETECTION
    if (birdX < pipeX + PIPE_W && birdX + 9 > pipeX) {
      if (birdY < pipeGapY || birdY + 7 > pipeGapY + pipeGap) {
        gameOver = true;
      }
    }

    // 4. RENDER
    display.clearDisplay();
    
    uint16_t colorTheme = (isNightMode) ? SSD1306_BLACK : SSD1306_WHITE;
    if (isNightMode) {
      display.fillRect(0, 0, 128, 64, SSD1306_WHITE); 
    }

    // วาดสิ่งแวดล้อมสลับตามโหมดเวลา
    if (!isNightMode) {
      display.drawCircle(110, 14, 4, colorTheme); // พระอาทิตย์
    } else {
      display.drawCircle(110, 14, 4, colorTheme); // พระจันทร์เสี้ยว
      display.fillCircle(112, 12, 3, SSD1306_WHITE); 
    }

    // วาดนกบิน
    if (wingUp) {
      display.drawBitmap(birdX, (int)birdY, bird_wing_up, 9, 7, colorTheme);
    } else {
      display.drawBitmap(birdX, (int)birdY, bird_wing_down, 9, 7, colorTheme);
    }
    
    // วาดท่อบน
    display.fillRect(pipeX, 0, PIPE_W, pipeGapY, colorTheme);
    display.drawRect(pipeX - 1, pipeGapY - 3, PIPE_W + 2, 3, colorTheme); 
    
    // วาดท่อล่าง
    display.fillRect(pipeX, pipeGapY + pipeGap, PIPE_W, 64 - (pipeGapY + pipeGap), colorTheme);
    display.drawRect(pipeX - 1, pipeGapY + pipeGap, PIPE_W + 2, 3, colorTheme); 

    // พื้นดิน
    display.drawFastHLine(0, 60, 128, colorTheme);

    // คะแนนและโหมด
    display.setTextSize(1);
    display.setTextColor(colorTheme);
    display.setCursor(4, 3);
    display.print("SCORE: "); display.print(score);
    
    display.setCursor(45, 3);
    display.print(isNightMode ? "[NIGHT]" : "[DAY]");
    
    display.display();

  } else {
    // SCREEN: GAME OVER
    uint16_t txtColor = (isNightMode) ? SSD1306_BLACK : SSD1306_WHITE;
    if (isNightMode) {
      display.fillRect(0, 0, 128, 64, SSD1306_WHITE);
    } else {
      display.clearDisplay();
    }
    
    display.setTextSize(2); 
    display.setCursor(12, 12); 
    display.setTextColor(txtColor);
    display.print("GAME OVER");
    
    display.setTextSize(1); 
    display.setCursor(15, 38); 
    display.print("FINAL SCORE: "); display.print(score);
    display.setCursor(12, 50); 
    display.print("Press Button to Retry");
    display.display();

    if (digitalRead(BTN_PIN) == LOW) {
      birdY = 32; 
      velocityY = 0; 
      pipeX = 128; 
      score = 0; 
      isNightMode = false; 
      gameOver = false;
      delay(300);
    }
  }
  delay(20); 
}
