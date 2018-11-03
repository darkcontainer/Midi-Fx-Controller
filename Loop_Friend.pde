/* Halojen Productions */
/* Loop Friend */
const char Version[] = "                Version 0.9a                ";
#define DEBUG 0

/* Buttons */
#define RESETPIN 8  // The button the user must hold during startup to reset all settings
#define BUTS 7
int Buttons[3][BUTS] = {
  { 6,  7,  8,  9, 17, 18, 19}, // Pins
  { 0,  0,  0,  0,  0,  0,  0}, // States
  {60, 60, 60, 60, 60, 60, -1} // Notes/Instructions
};

/* Settings */
#include <EEPROM.h>
#define SIZ 5
#define VAL 0
#define MIN 1
#define MAX 2
#define MEM 3
const char SettingNames[SIZ][14] = {
  {"Pattern"}, {"Tempo"}, {"Octave"}, {"Legato"}, {"Velocity"},
};
int Settings[SIZ][4] = {
 {1, 0, 2, 0},
 {140, 60, 220, 1},
 {5, 3, 7, 2},
 {0, 0, 1, 3},
 {100, 1, 127, 4}
};

/* Petterns */
#define PATLEN 16
const char PatternNames[3][14] = {{"Single"},{"Scale"},{"Arpeggio"}};
const int Patterns[3][2][PATLEN] = {
  {
    {60,0,0,0, 60,0,60,0, 60,0,60,0, 60,0,60,0}, // note (the off and held note pitches will be ignored)
    {1,0,-1,0,   1,0,-1,0,  1,0,-1,0,  1,0,-1,0}  // on, hold or off
  },{
    {60,62,63,65, 67,69,71,72, 70,68,67,65, 63,62,60,0},
    {1,1,1,1,    1,1,1,1,      1,1,1,1,      1,1,1,-1}
  },{
    {48,60,72,48, 60,72,48,60, 48,60,72,48, 60,72,48,60},
    {1,1,1,1,    1,1,1,1,      1,1,1,1,      1,1,1,1}
  }
};
int PlayNote = 0;
bool PlayPattern = false;
int PreviousNote = -1;

/* MIDI */
#include <MIDI.h>
const byte MidiOutChan = 0;

/* Rotary encoder */
#include <RotaryEncoder.h>
#define ENC_A 14
#define ENC_B 15
#define ENC_PORT PINC
#define ENC_SWITCH 16
RotaryEncoder Enc(ENC_A, ENC_B, ENC_PORT,ENC_SWITCH);

/* LCD */
#include <LiquidCrystal.h>
LiquidCrystal lcd(12, 11, 5, 4, 3, 2);
byte ArrowUD[8] = {
  B00100,
  B01010,
  B10001,
  B00000,
  B10001,
  B01010,
  B00100,
};
byte ArrowU[8] = {
  B00100,
  B01010,
  B10001,
  B00000,
  B00000,
  B00000,
  B00000,
};
byte ArrowD[8] = {
  B00000,
  B00000,
  B00000,
  B00000,
  B10001,
  B01010,
  B00100,
};
byte Patt[8] = {
  B11000,
  B10100,
  B11000,
  B10000,
  B10000,
  B00000,
  B00000,
};

/* LCD Light */
#include <LcdLight.h>
#define LCD_TIMEOUT 15000
#define LCD_FADEOUT 2500
LcdLight Backlight(/*pin*/10,LCD_TIMEOUT,LCD_FADEOUT);

void setup()
{
  Backlight.TurnOn();

  /* MIDI */
  MIDI.begin(MIDI_CHANNEL_OFF); // Don't listen on any channel
  
  /* Serial */
  #if DEBUG == 1
  // If Serial is on then MIDI wont work because of the diff baud rates
  // Serial must be begun after midi, to get the baud rate right.
  Serial.begin(115200);
  #endif
  
  bool ResetDown = false;
  /* Buttons */
  for(int i=0; i < BUTS; i++){
    pinMode(Buttons[0][i],INPUT);
    digitalWrite(Buttons[0][i], HIGH);
    // If the current button happens to be the reset button, check if its down as well.
    if(Buttons[0][i] == RESETPIN){
      ResetDown = !digitalRead(RESETPIN);
    }
    #if DEBUG == 1
    Serial.print("Pin: ");
    Serial.print(Buttons[0][i],DEC);
    Serial.print("   State: ");
    Serial.println(Buttons[1][i],DEC);
    #endif
  }
  
  //pinMode(PATBUTTONPIN,INPUT);
  //digitalWrite(PATBUTTONPIN,HIGH);
  
  // set up the LCD's number of rows and columns: 
  lcd.begin(16, 2);
  //lcd.clear();
  
  /* My fucken amazing startup sequence resides below.. */
  #if DEBUG != 1
  //If im debugging i dont need to see this bollocks
  StartupSequence();
  #endif
  
  lcd.createChar(0, ArrowUD);
  lcd.createChar(1, ArrowU);
  lcd.createChar(2, ArrowD);
  lcd.createChar(3, Patt);
  
  /* Load/Reset Settings */
  if(ResetDown && digitalRead(RESETPIN)==LOW){
    ResetSettings();
  }else{
    ReadSettings();
  }
  
  Backlight.TurnOn();
  DrawMenu(0, false);
}

void loop()
{
  Backlight.Update();
  
  // Buttons..
  for(int i = 0; i < BUTS; i++){
    if(Buttons[2][i] > -1){
      // It's a midi output button
      if(digitalRead(Buttons[0][i])==LOW && Buttons[1][i]==0){
        // If the button is on but was not previously on, send noteon
        int Note = Buttons[2][i] + (Settings[2][VAL] - 5) * 12;
        MIDI.sendNoteOn(Note,Settings[4][VAL],MidiOutChan);
        Buttons[1][i] = 1;
        #if DEBUG == 1
        Serial.print("Button down: ");
        Serial.print(i,DEC);
        Serial.print("   State: ");
        Serial.println(Buttons[1][i],DEC);
        #endif
      }else if(digitalRead(Buttons[0][i])==HIGH && Buttons[1][i]==1){
        // If the button is off but not previously on, send noteoff
        int Note = Buttons[2][i] + (Settings[2][VAL] - 5) * 12;
        MIDI.sendNoteOff(Note,0,MidiOutChan);
        Buttons[1][i] = 0;
        #if DEBUG == 1
        Serial.print("Button up: ");
        Serial.print(i,DEC);
        Serial.print("   State: ");
        Serial.println(Buttons[1][i],DEC);
        #endif
      }
    }else{
      // It's not midi out, it needs to do something else...
      if(digitalRead(Buttons[0][i])==LOW && Buttons[1][i]==0){
        switch (Buttons[2][i]){
        case -1:
          PlayPattern = !PlayPattern;
          break;
        }
        Buttons[1][i] = 1;
        #if DEBUG == 1
        Serial.print("Button down: ");
        Serial.print(i,DEC);
        Serial.print("   State: ");
        Serial.println(Buttons[1][i],DEC);
        #endif
      }else if(digitalRead(Buttons[0][i])==HIGH && Buttons[1][i]==1){
        Buttons[1][i] = 0;
      }
    }
  }
  
  if(PlayPattern){
    uint8_t CurrentPattern = Settings[0][VAL]; // The current parrern
    static unsigned long NoteStart = 0;
    
    float WaitTime = ((60/float(Settings[1][0]))*1000)/2;
    

    if( millis()>=NoteStart+WaitTime){
      NoteStart = millis();
      if(Patterns[CurrentPattern][1][PlayNote] == 1){
        // Note on
        int Note = Patterns[CurrentPattern][0][PlayNote] + ((Settings[2][VAL] - 5) * 12);
        #if DEBUG == 1
        Serial.print("Note on: ");
        Serial.println(Note,DEC);
        #else
        if(PreviousNote > -1){
          MIDI.sendNoteOff(PreviousNote,0,MidiOutChan);
        }
        MIDI.sendNoteOn(Note, Settings[4][VAL], MidiOutChan);
        #endif
//        #if DEBUG == 1
//        if(PreviousNote > -1){
//          Serial.print("auto Note off: ");
//          Serial.println(PreviousNote,DEC);
//        }
//        #endif
        PreviousNote = Note;
      }else if(Patterns[CurrentPattern][1][PlayNote] == -1 && Settings[3][VAL] == 0 && PreviousNote > -1){
        // If legato is OFF and there is a note playing
        // Note off
        #if DEBUG == 1
        Serial.print("L-off Note off: ");
        Serial.println(PreviousNote,DEC);
        #else
        MIDI.sendNoteOff(PreviousNote, 0, MidiOutChan);
        #endif
        //PreviousNote = -1; // No pattern notes currently playing. [Wrong, im not necessarily turning that one off.]
        PreviousNote = -1; // Fuck, i cant decide which way to do it
      }
      
      PlayNote++;
      if(PlayNote == PATLEN){
        PlayNote=0;
      }
    }
    lcd.setCursor(0, 0);
    lcd.write(3);
  }else if(PreviousNote > -1){
    // If legato is off it won't hit this much
    #if DEBUG == 1
    Serial.print("Note off: ");
    Serial.println(PreviousNote,DEC);
    #else
    MIDI.sendNoteOff(PreviousNote,0,MidiOutChan);
    #endif
    PreviousNote = -1; // No pattern notes currently playing.
    PlayNote = 0; // Reset note pointer to 0 so it will play from the beginning when next started
    lcd.setCursor(0, 0);
    lcd.print(" ");
  }
  
  // Encoder Button..
  if(Enc.ReadSwitch()){
    Backlight.TurnOn();
    DrawMenu(0, true);
  }
  
  // Encoder Rotary..
  int8_t EncVal;
  EncVal = Enc.Read();
  if(EncVal){
    Backlight.TurnOn();
    DrawMenu(EncVal, false);
  }
}

void DrawMenu(int8_t Move, bool Edit){
  static int MenuItem = 0;
  static bool EditValue = false;
  if(Edit){
    EditValue = !EditValue;
  }
  
  if(Edit && !EditValue){
    // Write setting to eeprom AFTER finished editing value
    EEPROM.write(Settings[MenuItem][MEM],Settings[MenuItem][VAL]);
    #if DEBUG == 1
    Serial.print("Wrote: ");
    Serial.println(EEPROM.read(Settings[MenuItem][MEM]),DEC);
    #endif
  }
  
  if(!EditValue /*&& Move != 0*/){
    MenuItem += Move;
    MenuItem = constrain(MenuItem,0,SIZ - 1);
  }else if(EditValue /*&& Move != 0*/){
    // Adjust and constrain values
    Settings[MenuItem][VAL] += Move;
    Settings[MenuItem][VAL] = constrain(Settings[MenuItem][VAL], Settings[MenuItem][MIN], Settings[MenuItem][MAX]);
  }
  
  lcd.clear();
  lcd.setCursor(1, 0);
  lcd.print(SettingNames[MenuItem]);
  lcd.setCursor(1, 1);
  
  static int DisplayVal = 0; // Not allowed to declare this in a case
  switch (MenuItem) {
  case 0:
    lcd.print(PatternNames[ Settings[MenuItem][VAL] ]);
    break;
  case 1:
    //TODO: some amazing acceleration so it doesnt take fucken ages to go far
    lcd.print(Settings[MenuItem][VAL],DEC);
    lcd.print("bpm");
    break;
  case 2:
    DisplayVal = Settings[MenuItem][VAL] - 5;
    lcd.print(Settings[MenuItem][VAL],DEC);
    if(DisplayVal != 0){
      lcd.print(" (");
      if(DisplayVal > 0){
        lcd.print("+");
      }
      lcd.print(DisplayVal,DEC);
      lcd.print(")");
    }
    break;
  case 3:
    if(Settings[MenuItem][VAL] == true){
      lcd.print("On");
    }else{
      lcd.print("Off");
    }
    break;
  default:
    lcd.print(Settings[MenuItem][VAL],DEC);
    break;
  }
  DrawArrows(MenuItem, EditValue);
}

void ResetSettings(){
  for (int i = 0; i < SIZ; i++) {
    EEPROM.write(Settings[i][MEM],Settings[i][VAL]);
    #if DEBUG == 1
    Serial.println(EEPROM.read(Settings[i][MEM]),DEC);
    #endif
  }
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Reset Complete");
  #if DEBUG == 1
  Serial.println("Reset Complete");
  #endif
  delay(1500);
}
void ReadSettings(){
  for (int i = 0; i < SIZ; i++) {
    Settings[i][VAL] = EEPROM.read(Settings[i][MEM]);
    #if DEBUG == 1
    Serial.println(EEPROM.read(Settings[i][MEM]),DEC);
    #endif
  }
  #if DEBUG == 1
  Serial.println("Settings Loaded");
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Settings Loaded");
  delay(1000);
  #endif
}
void DrawArrows(int MenuItem, bool EditValue){
  char ArrowChar = 0;
  if(!EditValue){
    lcd.setCursor(15, 0); // Last character, top row
    if(MenuItem == 0){
      ArrowChar = 1;
    }else if(MenuItem == SIZ - 1){
      ArrowChar = 2;
    }
  }else{
    lcd.setCursor(15, 1); // Last character, bottom row
    if(Settings[MenuItem][VAL] == Settings[MenuItem][MIN]){
      ArrowChar = 1;
    }else if(Settings[MenuItem][VAL] == Settings[MenuItem][MAX]){
      ArrowChar = 2;
    }
  }
  lcd.write(ArrowChar);
}

#if DEBUG != 1
// If im debugging i dont need to see this bollocks
void StartupSequence(){
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("                "); // WTF the top row doesn show unless it's set up like this
  lcd.print(" Halojen");
  lcd.setCursor(0, 1);
  lcd.print("                Productions");
  for(int i=0; i < 16; i++){
    lcd.scrollDisplayLeft();
    delay(50);
  }
  delay(800);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("                Loop Friend");
  lcd.setCursor(0, 1);
  lcd.print(Version);
  for(int i=0; i < 16; i++){
    lcd.scrollDisplayLeft();
    delay(50);
  }
  delay(1200);
  lcd.clear();
}
#endif
