/* Halojen Productions */
/* Fx Friend */
/* 2018 */
const char Version[] = "                Version 0.1a                ";
#define DEBUG 0

/* Buttons */
#define LED LED_BUILTIN
#define BUTT 10
#define POT A0

/* Settings */
#define SIZ 5
#define VAL 0
#define MIN 1
#define MAX 2
#define MEM 3

/* MIDI */
# include <MIDI.h>
// Creates and binds the MIDI interface to the default hardware Serial port
//MIDI_CREATE_DEFAULT_INSTANCE();
MIDI_CREATE_INSTANCE(HardwareSerial, Serial, MIDI);
const byte MidiOutChan = 1;

bool bButtDown = false;
int iPotVal = 0;

void setup() {

  pinMode(LED_BUILTIN, OUTPUT);
  
	/* MIDI */
	MIDI.begin(); // Don't listen on any channel
	/* Serial */
#if DEBUG == 1
	// If Serial is on then MIDI wont work because of the diff baud rates
	// Serial must be begun after midi, to get the baud rate right.
	Serial.begin(115200);
#endif

	/* Buttons */
	pinMode(BUTT, INPUT);
	digitalWrite(BUTT, HIGH);

 
#if DEBUG == 1
		Serial.print("Pin: ");
		Serial.print(BUTT, DEC);
		Serial.print("   State: ");
		Serial.println(digitalRead(BUTT), DEC);
#endif

	//pinMode(PATBUTTONPIN,INPUT);
	//digitalWrite(PATBUTTONPIN,HIGH);
}
void NoteOn(byte  Note, byte  Velocity, byte  Channel) {
	MIDI.sendNoteOn(Note, Velocity, Channel);
	digitalWrite(LED_BUILTIN, HIGH);
}
void NoteOff(byte  Note, byte  Channel) {
	MIDI.sendNoteOff(Note, 0, Channel);
	digitalWrite(LED_BUILTIN, LOW);
}
void loop() {

	// Buttons..
	// It's a midi output button.
  //NoteOn(60, 127, MidiOutChan);
  //MIDI.sendControlChange(3, (byte)100, MidiOutChan);
  //delay(100);
	if (digitalRead(BUTT) == LOW && !bButtDown) {
    bButtDown = true;
		//int Note = 60;
		NoteOn((byte)80, 127, MidiOutChan);
    //delay(100);
		
    #if DEBUG == 1
		/*Serial.print("Button down: ");
		Serial.print(BUTT, DEC);
		Serial.print("   State: ");
		Serial.println(digitalRead(BUTT), DEC);*/
    #endif
	} else if (digitalRead(BUTT) == HIGH && bButtDown) {
    bButtDown = false;
		// If the button is off but not previously on, send noteoff.
		byte byNote = 80;
    //delay(100);
		NoteOff(byNote, MidiOutChan);
    #if DEBUG == 1
		/*Serial.print("Button up: ");
		Serial.print(BUTT, DEC);
		Serial.print("   State: ");
		Serial.println(digitalRead(BUTT), DEC);*/
    #endif
	}

  int iNewPotVal = analogRead(POT);
  iNewPotVal = map(iNewPotVal,0,1023,0,63);
  iNewPotVal = map(iNewPotVal,0,63,0,127);
  iNewPotVal = map(iNewPotVal,22,108,0,127); // Callibrated.
  
  if (iNewPotVal != iPotVal){
    iPotVal=iNewPotVal;
    byte byControl = 3;
    //delay(100);
    MIDI.sendControlChange((byte)1, (byte)iPotVal, (byte)2);
    // delay(100);
    #if DEBUG == 1
    Serial.print("Control change: ");
    Serial.print(POT, DEC);
    Serial.print("   Value: ");
    Serial.println(iPotVal, DEC);
    #endif
  }
  
  #if DEBUG == 1
	/*Serial.print("Note off: ");*/
  #else
	//MIDI.sendNoteOff(60, 0, MidiOutChan);
  //delay(100);
  #endif
}
