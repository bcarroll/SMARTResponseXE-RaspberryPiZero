#include "SmartResponseXE.h"
#include "logo_rle.h"

int TEXT_WIDTH = 384;
int TEXT_HEIGHT = 136;
int DRAW_WIDTH = 128;
int DRAW_HEIGHT = 136;


const static uint8_t DEBUG = 10;
const static uint8_t BUZZER = 3;
const static uint8_t POWER_BUTTON = 20;
const static uint8_t VBATT = 26;

unsigned long lastState;
unsigned long lastKey;
const unsigned long timeState = 1000;
const unsigned long timeKey = 100;
byte ledState = LOW;

unsigned int counter = 0;
char buf[15];



// DAC configuration
// Select 1.6V ref voltage
// Select A0 as input
void setBatStat(void) {
  ADMUX = 0xC0; // Int ref 1.6V
  ADCSRA = 0x87; // Enable ADC
  ADCSRB = 0x00; // MUX5= 0, freerun
  ADCSRC = 0x54; // Default value
  ADCSRA = 0x97; // Enable ADC
  //delay(5);
  ADCSRA |= (1 << ADSC); // start conversion
}

// getBatStat
// returns battery's voltage in millivolts
// Battery is connected through a resistor divider (825k and 300k) with gain of 0.266666
// ADC ref is 1.6V with 1024 steps
// to simplify the calculation we will neglect 1024 and round it to 1000
// so we will use a conversion factor of 1.6/0.26666 which is 6
// This will give an error of 24/1000 which is acceptable in this case
uint16_t getBatStat(void) {
  uint16_t low, high;
  ADCSRA |= (1 << ADSC);
  while (ADCSRA & (1 << ADSC));
  low = ADCL;
  high = ADCH;
  return ((high << 8) + low) * 6;
}

// Draw display background
void initAppDisplay(void) {

  SRXEFill(0);
  // Next, you can draw some text or rectangles. The color can be 0-3 (0=off, 3=fully on)
  SRXEWriteString(60, 2, "Raspberry Pi Zero W v1.3", FONT_MEDIUM, 3, 0); // (int x, int y, char *szMsg, int iSize, int iFGColor, int iBGColor)
  SRXEWriteString(60, 22, "Battery:", FONT_LARGE, 3, 0);
  SRXEWriteString(60, 42, "Uptime :", FONT_LARGE, 3, 0);
  SRXEWriteString(60, 62, "Key    :", FONT_LARGE, 3, 0);

  // Draw a rectanle on each side of the screen
  SRXERectangle(0, 0, 17, 135, 0x1, 0x1); // (int x, int y, int cx, int cy, byte color, byte bFilled)
  SRXERectangle(110, 0, 17, 135, 0x1, 0x1);
  SRXEVerticalLine(17,0,135,3);
  SRXEVerticalLine(110,0,135,3);

  // Draw arrows pointing to the soft keys
  SRXEWriteString(0, 2,  "< A", FONT_LARGE, 3, 1);
  SRXEWriteString(0, 32, "< B", FONT_LARGE, 3, 1);
  SRXEWriteString(0, 62, "< C", FONT_LARGE, 3, 1);
  SRXEWriteString(0, 92, "< D", FONT_LARGE, 3, 1);
  SRXEWriteString(0, 122, "< E", FONT_LARGE, 3, 1);

  int charnum = 3;
  int rightColX = TEXT_WIDTH - (16 * charnum);
  SRXEWriteString(rightColX, 2,  "F >", FONT_LARGE, 3, 1);
  SRXEWriteString(rightColX, 32, "G >", FONT_LARGE, 3, 1);
  SRXEWriteString(rightColX, 62, "H >", FONT_LARGE, 3, 1);
  SRXEWriteString(rightColX, 92, "I >", FONT_LARGE, 3, 1);
  SRXEWriteString(rightColX, 122, "J >", FONT_LARGE, 3, 1);

  // Draw a vertical and horizontal line
  SRXEVerticalLine(DRAW_WIDTH / 2, 107, TEXT_HEIGHT - 84, 0x3); // int x, int y, int height, byte color // color options: 0x0 - 0x3
  SRXEHorizontalLine(17, 107, 93, 0x3, 2); // (int x, int y, int length, byte color, int thickness)
}

// Function that put the terminal in sleep mode
// When waking up restore the display and the ADC configuration
void goToSleep(void) {
  SRXESleep(); // go into sleep mode and wait for an event (power button)
  // returning from sleep
  initAppDisplay(); // restore screen background
  setBatStat(); // restore ADC configuration
  lastState = millis();
  lastKey = lastState;
}

// testText
// Test how long it takes to fill the screen with text
// using either one of the font
void testText(void) {
  unsigned long startTime;
  unsigned long timeItTook[4] = {0, 0, 0, 0};
  char* myString = {"abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789!?"};
  uint8_t line;
  uint8_t nbLine;
  
  for (uint8_t font = 0; font < 4; font++) {
    uint8_t inc = (font & 2) ? 16 : 8; // line increment
    SRXEFill(0); // clear screen
    startTime = micros();
    for (line = 0, nbLine = 0; line < (TEXT_HEIGHT - inc + 1); line += inc, nbLine++) {
      SRXEWriteString(0, line, myString, font, 3, 0);
    }
    timeItTook[font] = micros() - startTime;
    SRXERectangle(8, 16, 48, 54, 0, 1);
    SRXERectangle(8, 16, 48, 54, 3, 0);
    snprintf(buf, sizeof(buf) - 1, "%2d", nbLine);
    SRXEWriteString(33, 20, buf, FONT_MEDIUM, 3, 0);
    SRXEWriteString(69, 20, "Lines", FONT_MEDIUM, 3, 0);
    snprintf(buf, sizeof(buf) - 1, "%lu us", timeItTook[font]);
    SRXEWriteString(33, 36, buf, FONT_MEDIUM, 3, 0);
    SRXEWriteString(30, 52, "Press a key", FONT_MEDIUM, 3, 0);

    while (SRXEGetKey() == 0);
  }
  SRXEFill(0);
  SRXEWriteString(30, 20, "Normal: ", FONT_MEDIUM, 3, 0);
  snprintf(buf, sizeof(buf) - 1, "%lu us", timeItTook[0]);
  SRXEWriteString(150, 20, buf, FONT_MEDIUM, 3, 0);
  SRXEWriteString(30, 40, "Small : ", FONT_MEDIUM, 3, 0);
  snprintf(buf, sizeof(buf) - 1, "%lu us", timeItTook[1]);
  SRXEWriteString(150, 40, buf, FONT_MEDIUM, 3, 0);
  SRXEWriteString(30, 60, "Medium: ", FONT_MEDIUM, 3, 0);
  snprintf(buf, sizeof(buf) - 1, "%lu us", timeItTook[2]);
  SRXEWriteString(150, 60, buf, FONT_MEDIUM, 3, 0);
  SRXEWriteString(30, 80, "Large : ", FONT_MEDIUM, 3, 0);
  snprintf(buf, sizeof(buf) - 1, "%lu us", timeItTook[3]);
  SRXEWriteString(150, 80, buf, FONT_MEDIUM, 3, 0);
  while (SRXEGetKey() == 0);
}

// test scroll
// fill screen with text
// set scrolling area
// scroll part of the screen
//
void testScroll(void){
  int lineNum;
  char* myString = {"abcdefghijklmnopqrstuvwxyz"};
  char* string2={"  That's all  "};

  SRXEFill(0); // clear screen
  // fill screen with text
  for(lineNum=0; lineNum<17; lineNum++){
    snprintf(buf, sizeof(buf) - 1, "%u", lineNum);
    SRXEWriteString(0, lineNum*8, buf, FONT_NORMAL, 3, 0);
    SRXEWriteString(27, lineNum*8, myString, FONT_NORMAL, 3, 0);
  }
  delay(3000);
  // scroll text
  SRXEScrollArea(0,128,32); // set 16 text lines on top and 1 fixed at bottom
  for(lineNum=0; lineNum<128; lineNum++){
    SRXEScroll(1);
    delay(lineNum%16==0?150:20);
  }
  SRXEWriteString(0, 128, string2, FONT_NORMAL, 3, 0);
  SRXEScrollArea(0,160,0); // restore default values
  SRXEScrollReset();
  delay(3000);
}

//   host_splashscreen
//  displays a greeting screen
//
void host_splashscreen(void) {
  const int startHit = 138;
  const int lengthArrow = 84;
  const int startArrow = startHit + 132;
  int pos = startArrow;
  unsigned long pauseArrow = 500ul;
  unsigned long lastTimeArrow = 0;
  unsigned long currenTime = millis();

  SRXELoadBitmapRLE(0, 0, bitmap_logo_rle);
  SRXEWriteString(startHit, 110, "Hit a key         ", FONT_MEDIUM, 3, 1);
  while (!SRXEGetKey()) {
      if (Serial.available() > 0){
      String serialData = Serial.readStringUntil('\n');
      SRXEWriteString(0, 115, serialData, FONT_SMALL, 3, 0);
    }

    currenTime = millis();
    if (currenTime - lastTimeArrow >= pauseArrow) {
      SRXEWriteString(pos, 110, " ", FONT_MEDIUM, 3, 1);
      pos += 12;
      if (pos > startHit + 120 + lengthArrow) pos = startArrow;
      SRXEWriteString(pos, 110, ">", FONT_MEDIUM, 3, 1);
      lastTimeArrow = currenTime;
    }
  };
}


// initialize system
// display
void setup() {
  Serial.begin(115200);

  setBatStat(); // ADC setup

  SRXEInit(0xe7, 0xd6, 0xa2); // initialize and clear display // CS, D/C, RESET

  Serial.println("SMART_Response_XE_initialized");

  host_splashscreen();  // displays the splashscreen

  initAppDisplay();

  lastState = millis();
  lastKey = lastState;

  // set Power button input
  pinMode(POWER_BUTTON, INPUT_PULLUP);
}

void loop() {
  // if power button pressed go into sleep mode
  if (digitalRead(POWER_BUTTON) == 0) {
    goToSleep();
  }

  if (Serial.available() > 0){
    String serialData = Serial.readStringUntil('\n');
    SRXEWriteString(0, 115, serialData, FONT_SMALL, 3, 0);
  }

  // Every timeState (1s )
  //  get battery state
  //  increment counter
  //
  if ((millis() - lastState) > timeState) {
    lastState = millis();
    // Battery state
    memset (buf, 0, sizeof(buf));
    snprintf(buf, sizeof(buf) - 1, "%u", getBatStat());
    SRXEWriteString(189, 22, "      ", FONT_LARGE, 3, 0);
    SRXEWriteString(189, 22, buf, FONT_LARGE, 3, 0);

    // clear the previous text and draw new uptime
    memset (buf, 0, sizeof(buf));
    snprintf(buf, sizeof(buf) - 1, "%u", counter);
    SRXEWriteString(189, 42, "      ", FONT_LARGE, 3, 0);
    SRXEWriteString(189, 42, buf, FONT_LARGE, 3, 0);

    counter++;
  }

  // Every timeKey (100ms)
  // Scan keyboard
  // and test for some "special" keypress
  if ((millis() - lastKey) > timeKey) {
    lastKey = millis();
    // clear the previous text and draw new key
    if (char key = SRXEGetKey()) {
      memset (buf, 0, sizeof(buf));
      snprintf(buf, sizeof(buf) - 1, "%c, %02X", key < 0x20 ? 0x20 : key, (byte)key);
      SRXEWriteString(189, 62, "      ", FONT_LARGE, 3, 0);
      SRXEWriteString(189, 62, buf, FONT_LARGE, 3, 0);
      
      // action linked to some keypress
      // if key is t go for a screen text filling test
      if (key == 't') {
        testText();
        initAppDisplay();
        lastState = millis();
        lastKey = lastState;
      }
      
      // test screen scroll
      if (key=='s'){
        testScroll();
        initAppDisplay();
        lastState = millis();
        lastKey = lastState;
      }

    }
  }
}
