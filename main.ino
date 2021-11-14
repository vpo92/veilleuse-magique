/**************************************************************************
 This is an example for our Monochrome OLEDs based on SSD1306 drivers

 Pick one up today in the adafruit shop!
 ------> http://www.adafruit.com/category/63_98

 This example is for a 128x32 pixel display using I2C to communicate
 3 pins are required to interface (two I2C and one reset).

 Adafruit invests time and resources providing this open
 source code, please support Adafruit and open-source
 hardware by purchasing products from Adafruit!

 Written by Limor Fried/Ladyada for Adafruit Industries,
 with contributions from the open source community.
 BSD license, check license.txt for more information
 All text above, and the splash screen below must be
 included in any redistribution.
 **************************************************************************/

#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <DS3231.h>//https://github.com/NorthernWidget/DS3231

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
// The pins for I2C are defined by the Wire-library. 
// On an arduino UNO:       A4(SDA), A5(SCL)
// On an arduino MEGA 2560: 20(SDA), 21(SCL)
// On an arduino LEONARDO:   2(SDA),  3(SCL), ...
#define OLED_RESET     4 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);


//MENU
#define MENU_HOME      0
#define MENU_DAY       1
#define MENU_NIGHT     2
#define MENU_TIME      3


/* Broches */
const byte PIN_BTN_UP = 2;
const byte PIN_BTN_DOWN = 3;
const byte PIN_BTN_OK = 4;

const byte PIN_LED_R = 5;
const byte PIN_LED_B = 7;
const byte PIN_LED_G = 6;



//Variables
byte Year;
byte Month;
byte Date;
byte DoW;
byte Hour;
byte Minute;
byte Second;
bool Century=false;
bool h12;
bool PM;

//App logic
int currentMenu = 0;
int currentCursor = 0;


int btnUpState = 1;
int btnDownState = 1;
int btnOkState = 1;

//night
int nH = 20;
int nM = 30;

//day
int dH = 4;
int dM = 30;


//time
int tH = 0;
int tM = 0;


//Objects
DS3231 Clock;

void setup() {
  Serial.begin(9600);

  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }

  // Initialise les broches
  pinMode(PIN_LED_R, OUTPUT);
  pinMode(PIN_LED_G, OUTPUT);
  pinMode(PIN_LED_B, OUTPUT);
  pinMode(PIN_BTN_UP, INPUT_PULLUP);
  pinMode(PIN_BTN_DOWN, INPUT_PULLUP);
  pinMode(PIN_BTN_OK, INPUT_PULLUP);
  //drawTime();
}

void loop() {

    bool upPush = false;
    bool downPush = false;
    bool okPush = false;

    //UP
    if(digitalRead(PIN_BTN_UP)==0 && btnUpState == 1){
       Serial.println("push up");
       upPush = true;
       btnUpState = 0;
       //menuUp();
       
    //DOWN
    }else if(digitalRead(PIN_BTN_DOWN)==0 && btnDownState == 1){
       Serial.println("push down");
       downPush = true;
       btnDownState = 0;
       //menuDown();
    //OK
    }else if(digitalRead(PIN_BTN_OK)==0 && btnOkState == 1){
       Serial.println("push ok");
       okPush = true;
       btnOkState = 0;
    }
    
    //Release btn
    if(digitalRead(PIN_BTN_UP)==1){
       btnUpState = 1;
    }
    if(digitalRead(PIN_BTN_DOWN)==1){
       btnDownState = 1;
    }
    if(digitalRead(PIN_BTN_OK)==1){
       btnOkState = 1;
    }
    
    Serial.println((String) "UP"+btnUpState);
    Serial.println((String) "DOWN"+btnDownState);
    Serial.println((String) "OK"+btnOkState);
    Serial.println(currentMenu);

    //APP Logik
    switch(currentMenu){

      case MENU_HOME:
        if(upPush){
          menuUp();
        }else if(downPush){
          menuDown();
        }
        break;
      case MENU_DAY:
        switch(currentCursor){
          //Accueil
          case 0:
            if(upPush){
              menuUp();
            }else if(downPush){
              menuDown();
            }else if(okPush){
              cursorUp();
            }
            break;
          //H
          case 1:
            if(upPush){
              dH=incH(dH);
            }else if(downPush){
              dH=decH(dH);
            }else if(okPush){
              cursorUp();
            }
            break;
          //M
          case 2:
            if(upPush){
              dM=incM(dM);
            }else if(downPush){
              dM=decM(dM);
            }else if(okPush){
              cursorReset();
              saveDay();
              //SAVE DAY EPROM
            }
            break;
        }
        break;
      case MENU_NIGHT:
        switch(currentCursor){
          //Accueil
          case 0:
            if(upPush){
              menuUp();
            }else if(downPush){
              menuDown();
            }else if(okPush){
              cursorUp();
            }
            break;
          //H
          case 1:
            if(upPush){
              nH=incH(nH);
            }else if(downPush){
              nH=decH(nH);
            }else if(okPush){
              cursorUp();
            }
            break;
          //M
          case 2:
            if(upPush){
              nM=incM(nM);
            }else if(downPush){
              nM=decM(nM);
            }else if(okPush){
              cursorReset();
              saveNight();
              //SAVE NIGHT EPROM
            }
            break;
        }
        break;
     case MENU_TIME:
        
        switch(currentCursor){
          //Accueil
          case 0:
            if(upPush){
              menuUp();
            }else if(downPush){
              menuDown();
            }else if(okPush){
              cursorUp();
            }
            break;
          //H
          case 1:
            if(upPush){
              tH=incH(tH);
            }else if(downPush){
              tH=decH(tH);
            }else if(okPush){
              cursorUp();
            }
            break;
          //M
          case 2:
            if(upPush){
              tM=incM(tM);
            }else if(downPush){
              tM=decM(tM);
            }else if(okPush){
              cursorReset();
              saveTime();
              //SAVE NIGHT EPROM
            }
            break;
        }
        break;
    }

    //Display
    switch(currentMenu){
      case MENU_HOME:
        drawHome(Clock.getHour(h12, PM),Clock.getMinute(),0,0);
        break;
      case MENU_TIME:
        drawTimeDetail("H", tH,tM,currentCursor);
        break;
      case MENU_DAY:
        drawTimeDetail("J", dH,dM,currentCursor);
        break;
      case MENU_NIGHT:
        //drawNight(nH,nM,currentCursor);
        drawTimeDetail("N", nH,nM,currentCursor);
        break;
    }
    
    delay(50);
    
    //LED
    //digitalWrite(PIN_LED_R, LOW);
    //digitalWrite(PIN_LED_G, LOW);
    //digitalWrite(PIN_LED_B, HIGH);
    ledColor();
}


/***
 * 
 * 
 * BUSINESS
 * 
 * 
 */
void saveNight(){
  //
}

void saveDay(){
  //
}

void saveTime(){
    Clock.setClockMode(false);
    // set to 24h
    Clock.setSecond(0);
    Clock.setMinute(tM);
    Clock.setHour(tH);
}


/**
 * 
 * HELPER
 * 
 */


void menuUp(){
  currentMenu = (currentMenu+1)%4;
  cursorReset();
}

void menuDown(){
  if(currentMenu == 0){
          currentMenu = 3;
  }else{
          currentMenu = (currentMenu-1); 
  }
  cursorReset();
}

void cursorUp(){
  currentCursor++;
}

void cursorReset(){
  currentCursor = 0;
}

//increment h
int incH(int h){
  h++;
  if(h>23)
    h = 0;
  return h;
}

int decH(int h){
  h--;
  if(h<0)
    h = 23;
  return h;
}

int incM(int m){
  m++;
  if(m>59)
    m = 0;
  return m;
}

int decM(int m){
  m--;
  if(m<0)
    m = 59;
  return m;
}

/*********
 * 
 * 
 * LED LOGIK
 * 
 * 
 */

void ledColor(){
  
  //4H30 => 430; 12H12 -> 1212
  int currentTime = Clock.getHour(h12, PM)*100 + Clock.getMinute();
  int startTime = dH*100+dM;
  int endTime = nH*100+nM;
  
  //Day
  if(currentTime > startTime && currentTime < endTime){
    digitalWrite(PIN_LED_R, LOW);
    digitalWrite(PIN_LED_G, HIGH);
    digitalWrite(PIN_LED_B, LOW);
  //Night
  }else{
    digitalWrite(PIN_LED_R, HIGH);
    digitalWrite(PIN_LED_G, LOW);
    digitalWrite(PIN_LED_B, LOW);
  }
  
}


/******
 * 
 * GESTION GRAPHIQUE
 * 
 */
String formatDigit(int d){
  char buf[6];
  sprintf(buf, "%02d", d);
  return buf;
}

//128/8 => 16
void drawHome(int currentH,int currentM, int dayTime, int nightTime){
  display.clearDisplay();

  //UP
  display.fillTriangle(8, 0, 16, 12, 0, 12, SSD1306_WHITE);
  //DOWN
  display.fillTriangle(8, 31, 0, 19, 16, 19, SSD1306_WHITE);

  //TIME
  display.setTextSize(2);      // Normal 1:1 pixel scale
  display.setTextColor(SSD1306_WHITE); // Draw white text
  display.setCursor(22, 8);     // Start at top-left corner
  display.cp437(true);         // Use full 256 char 'Code Page 437' font

  display.print(formatDigit(currentH));
  display.print(":");
  display.print(formatDigit(currentM));

  //NIGHT
  display.setTextSize(1);      // Normal 1:1 pixel scale
  display.setTextColor(SSD1306_WHITE); // Draw white text
  display.setCursor(92, 0);     // Start at top-left corner
  display.cp437(true);         // Use full 256 char 'Code Page 437' font
  display.print("N");
  display.print(formatDigit(nH));
  display.print(":");
  display.print(formatDigit(nM));

  //DAY
   display.setTextSize(1);      // Normal 1:1 pixel scale
  display.setTextColor(SSD1306_WHITE); // Draw white text
  display.setCursor(92, 22);     // Start at top-left corner
  display.cp437(true);         // Use full 256 char 'Code Page 437' font
  display.print("J");
  display.print(formatDigit(dH));
  display.print(":");
  display.print(formatDigit(dM));

  display.display();
}

void drawTimeDetail(String type, int h, int m, int currentCursor){
  display.clearDisplay();

  //UP
  display.fillTriangle(8, 0, 16, 12, 0, 12, SSD1306_WHITE);
  //DOWN
  display.fillTriangle(8, 31, 0, 19, 16, 19, SSD1306_WHITE);

  //ZZ
  display.setTextSize(2);      // Normal 1:1 pixel scale
  display.setTextColor(SSD1306_WHITE); // Draw white text
  display.setCursor(22, 8);     // Start at top-left corner
  display.cp437(true);         // Use full 256 char 'Code Page 437' font
  display.print((String)type+":");
  display.print(formatDigit(h));
  display.print(":");
  display.print(formatDigit(m));

  //Cursor N
  //display.drawLine(20,26,34,26,SSD1306_WHITE);

  //Cursor H
  if(currentCursor == 1){
    display.drawLine(46,26,68,26,SSD1306_WHITE);  
  //Cursor M
  }else if(currentCursor == 2){
    display.drawLine(82,26,104,26,SSD1306_WHITE);  
  }

  display.display();
}
/**
void drawTimeOld(void) {
  display.clearDisplay();

  display.setTextSize(1);      // Normal 1:1 pixel scale
  display.setTextColor(SSD1306_WHITE); // Draw white text
  display.setCursor(0, 0);     // Start at top-left corner
  display.cp437(true);         // Use full 256 char 'Code Page 437' font

  // Not all the characters will fit on the display. This is normal.
  // Library will draw what it can and the rest will be clipped.
  for(int16_t i=0; i<256; i++) {
    if(i == '\n') display.write(' ');
    else          display.write(i);
  }

  display.display();
  delay(2000);
}
*/
/**
void readRTC( ) { / function readRTC /////Read Real Time Clock
  Serial.print(Clock.getYear(), DEC);
  Serial.print("-");
  Serial.print(Clock.getMonth(Century), DEC);
  Serial.print("-");
  Serial.print(Clock.getDate(), DEC);
  Serial.print(" ");
  Serial.print(Clock.getHour(h12, PM), DEC); //24-hr
  Serial.print(":");
  Serial.print(Clock.getMinute(), DEC);
  Serial.print(":");
  Serial.println(Clock.getSecond(), DEC);
  delay(1000);
}*/
