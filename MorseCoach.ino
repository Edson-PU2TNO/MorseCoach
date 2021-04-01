//******************************
//*Author: Edson Santos
//*Email: pu2tno@pilotbeacon.com
//******************************

#include <EEPROM.h>
#include <Adafruit_RGBLCDShield.h>
#include "morse.h"

/*  
 1 - dah
 0 - dit
*/

const byte CHARS[][3] = {
  {' ',0b0,0}, // Space
  
  {'E',0b0,1},    {'I',0b00,2},   {'S',0b000,3}, 
  {'A',0b01,2},   {'U',0b001,3},  {'T',0b1,1}, 
  {'M',0b11,2},   {'O',0b111,3},  {'N',0b10,2},
  {'D',0b100,3},  {'F',0b0010,4}, {'P',0b0110,4},
  {'J',0b0111,4}, {'L',0b0100,4}, {'Q',0b1101,4},
  {'X',0b1001,4}, {'B',0b1000,4}, {'Y',0b1011,4},
  {'W',0b011,3},  {'R',0b010,3},  {'H',0b0000,4},
  {'Z',0b0011,4}, {'G',0b110,3},  {'K',0b101,3},
  {'V',0b0001,4}, {'C',0b1010,4}, //27 (idx=26) 
  
  {'1',0b01111,5},  {'2',0b00111,5},  {'3',0b00011,5},
  {'4',0b00001,5},  {'5',0b00000,5},  {'6',0b10000,5},
  {'7',0b11000,5},  {'8',0b11100,5},  {'9',0b11110,5}, // 36 (idx=35)
  {'0',0b11111,5}, // 37 (idx=36)
  
  {'/',0b10010,5} // 38 (idx=37)  
};

//byte PARIS[] = {12,4,20,2,3};


const char *const menuOptions[] = { "Words p/ minute","Words p/ min (F)", "Mode", "Word len min", "Word len max", "Buzz (KHz)", "Qty. of letters", "Qty. of numbers" };
const char *const modeOptions[] = {"Letters","Numbers","Mixed"};
// savedData[] = {WPM, WPM(F),Mode, MinWord, MaxWord, Buzz, Qty Letters , Qty Numbers}
int savedData[] = {20, 13, 1, 5, 5, 700, 27, 10};
int savedDataMax[] = {40, 40, 3, 9, 9, 800, 27, 10};
int savedDataMin[]= {5, 5, 1, 4, 4, 700, 1, 1};

byte icons[2][8] = { { 0x04,0x0e,0x15,0x04,0x04,0x04,0x04 },
                     { 0x04,0x04,0x04,0x04,0x15,0x0e,0x04 } };

//! Enum of backlight colors.
enum Icons {UP=0x00, DOWN};
enum BackLightColor { RED=0x1, GREEN, YELLOW, BLUE, VIOLET, TEAL, WHITE };
enum Mode { Letters=0x01, Numbers, Mixed }; //Letter, Numbers, Mix



byte mult = 0;
byte colPos = 0;
byte clicked_buttons;
byte menu_idx = 0;

// Time unit for common method 
unsigned long timeUnit; // = (1000*1.2/savedData[0]);

// Time Unit for Farnworth method
unsigned long timeUnitf;

// 1st Line string
char firstLinestr[16];

// Flag to indicated that saved data is valid
boolean saved = true;

//! The LCD display object.
Adafruit_RGBLCDShield lcd = Adafruit_RGBLCDShield();

//! The current state.
void (*state)() = NULL;


//*****************************************
void setup() {
  int savedDataVal[(sizeof(savedData)/sizeof(savedData[0]))];
  randomSeed(analogRead(0));
  // Serial.begin(9600); // Uncomment to support serial communication  
  lcd.begin(16, 2);


  if (lcd.readButtons() & (BUTTON_UP | BUTTON_DOWN)) state = config_Menu;
  else state = splashScreen;

  lcd.createChar(UP, icons[UP]);
  lcd.createChar(DOWN, icons[DOWN]);  

// Validating savedData

  EEPROM.get (0,savedDataVal);
  for (int i=0; i<(sizeof(savedDataVal)/sizeof(savedDataVal[0])); i++) {
    if ((savedDataVal[i]<savedDataMin[i]) | (savedDataVal[i]>savedDataMax[i])){
      savedData[i] = -1;
      saved = false;
    }  
    if(saved) EEPROM.get (0,savedData);  
  }
}

//*****************************************
void loop() {
  state();
}


//*****************************************
void startMorse(){
  sendSequence((byte)random(savedData[3],savedData[4]+1));
}


//*****************************************
void config_Menu() {
  lcd.setBacklight(YELLOW);
  lcd.clear();
  lcd.print (F("Config use: ")); lcd.write(UP); lcd.write(DOWN); lcd.write(0x7F); lcd.write(0x7E);
  lcd.setCursor(0,1);
  lcd.print(F("SELECT to return"));
  delay(5000);
  state = menuOption;
}

//*****************************************
void menuOption(){
  lcd.clear();
  int menuLen = ((sizeof(menuOptions)/sizeof(menuOptions[0]))-1);
  while(1) {
    read_button_clicks();
    if (clicked_buttons & BUTTON_LEFT) {
      menu_idx = (menu_idx > 0) ? menu_idx - 1 : menuLen;
      lcd.clear();
    }
    else if (clicked_buttons & BUTTON_RIGHT) {
      menu_idx = (menu_idx < menuLen) ? menu_idx + 1 : 0;
      lcd.clear();
    }
    else if (clicked_buttons & (BUTTON_UP|BUTTON_DOWN)) {
      configValue(menu_idx);  
      lcd.setBacklight(YELLOW);
    }
    else if (clicked_buttons & BUTTON_SELECT) {
    state = splashScreen;
    return;
    }
     lcd.setCursor(0,0);
     lcd.print(menuOptions[menu_idx]);
     lcd.setCursor(0,1);
     lcd.print (savedData[menu_idx]);
     if (menu_idx == 2) { 
        lcd.print (" - ");
        lcd.print (modeOptions[savedData[2]-1]);     
     }
  }
}


//*****************************************
void read_button_clicks() {
  static byte last_buttons = 0;
  
  byte buttons = lcd.readButtons();
  clicked_buttons = last_buttons & (~buttons);
  last_buttons = buttons;
}


//*****************************************
void splashScreen(){
  lcd.clear();
  lcd.setBacklight(TEAL);
  lcd.print(F("Morse Gen V0.2"));
  lcd.setCursor(0,1);
  lcd.print("Mode: "); lcd.print(modeOptions[savedData[2]-1]); 
  delay(2000);
  lcd.setBacklight(GREEN);
  lcd.clear();
 // lcd.setCursor(0,0);
  lcd.print(savedData[0]); lcd.print(F(" cWPM ")); lcd.print (savedData[1]); lcd.print(F(" eWPM"));
  lcd.setCursor(0,1);
  if (saved) state = startMorse;
  else state = config_Menu;
}
         

//*****************************************
void sendSequence(byte number){
  byte localnr;
  unsigned long t0,t1,t2 = 0;
  timeUnit = (1000*1.2/savedData[0]); // in mili seconds
  timeUnitf = ((((60*savedData[0]) - (37.2*savedData[1]))/(savedData[0]*savedData[1]))/19)*1000; // in mili seconds
  if ((colPos+number)>16){
    t0 = millis();
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print (firstLinestr);
    lcd.setCursor(0,1);   
    for (int i=0; i<16; i++) firstLinestr[i]=' ';
    colPos = 0;
    t2 = millis() - t0;
  }
  for (int i=0; i<number; i++) {
    switch (savedData[2]) {
      case Letters:
          localnr = random(1,savedData[6]+1);
          break;
      case Numbers:
          localnr = random(27,savedData[7]+27);
	  break;
      case Mixed:
	  localnr = random(1,38);
          break;
      default: //Letters
          localnr = random(1,savedData[6]+1);
	  break;
    }
    playLetter(localnr);    
// display letter and consider time spent to process it
    t0 = millis(); 
    printLetter(localnr);
    t1 = millis() - t0;      
// delay 3 Time Units (inter char space) minus time spent to display the letter. Not applied for the last char in the word
    if (i<number) delay (charSpace - t1);
  }
  t0 = millis(); 
  printLetter(0);
  t1 = millis() - t0;
  if (t2 > 0) {
// delay 7 Time Units (word space) minus time spent to display the letter,
// minus time spent to clear lcd and repeat line 2
    delay (wordSpace - t1 - t2);
    t2 = 0;
  }
// delay 7 Time Units (word space) minus time spent to display the letter
  else delay (wordSpace - t1);
}

//*****************************************
void playLetter(byte idx) {  
  byte sizeLetter = CHARS[idx][2];
  byte letter = CHARS[idx][1] << (8-sizeLetter);
  for (byte i=0; i<sizeLetter; i++){
    if (letter & mask) mult = 3;
    else mult = 1;
    tone (8,savedData[5], (mult*timeUnit));
    if (i<sizeLetter) delay ((mult+1)*timeUnit);
    else delay (mult*timeUnit);
    letter = letter << 1;
    } 
}

//*****************************************
void printLetter(byte idx){
  lcd.print((char)CHARS[idx][0]);
  firstLinestr[colPos] = (char)CHARS[idx][0];
  colPos++;
}

//*****************************************
void configValue(byte idx) {
  lcd.setBacklight(VIOLET);
  while (1){
    read_button_clicks();
    if (clicked_buttons & BUTTON_UP) { 
      if (savedData[idx] == savedDataMax[idx]) savedData[idx] = savedDataMax[idx];
      else savedData[idx]++;
      lcd.setCursor(0,1);
      lcd.print(savedData[idx]); 
      if (idx==2) {
	      lcd.print(" - ");
	      lcd.print(modeOptions[savedData[idx]-1]);
        lcd.print("       ");	
      }
      lcd.setCursor(0,1); 
    }
    else if (clicked_buttons & BUTTON_DOWN)
     {
      if (savedData[idx] == savedDataMin[idx]) savedData[idx] = savedDataMin[idx];
      else savedData[idx]--;
      if (savedData[idx] <= 9) {
        lcd.setCursor (0,1);
        lcd.print("  ");
      }
      lcd.setCursor (0,1);
      lcd.print(savedData[idx]); 
      if (idx==2) {
        lcd.print(" - ");
        lcd.print(modeOptions[savedData[idx]-1]);
        lcd.print("       "); 
      }
      lcd.setCursor(0,1);  
     }
    else if (clicked_buttons & BUTTON_SELECT){
      if (checkInc()) {
         lcd.clear();    
         lcd.setBacklight(VIOLET);
         break;
      }   	
      lcd.clear();
      lcd.setCursor(0,1);
      EEPROM.put(0,savedData);  
      saved = true; 
      return;
    }  
  }
}

//*****************************************
boolean checkInc() {
if ((savedData[0] < savedData[1])|(savedData[3] > savedData[4])) {
   lcd.setBacklight(RED);
   lcd.clear();
   lcd.print(F("Please check"));
   lcd.setCursor(0,1);
   lcd.print(F("Inconsistency !"));
   delay (5000);
   return true;
   } else return false;
}
