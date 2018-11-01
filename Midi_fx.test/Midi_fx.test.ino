/* Halojen Productions */
/* Fx Friend */
/* 2018 */
//const char Version[] = "                Version 0.1a                ";
#define DEBUG 0

/* Buttons */
#define RESETPIN 8 // The button the user must hold during startup to reset all settings
#define LED LED_BUILTIN
#define BUTS 7
int Buttons[3][BUTS] = {
	{10, 7, 8, 9, 17, 18, 19},	// Pins
	{0, 0, 0, 0, 0, 0, 0},		 // States
	{60, 60, 60, 60, 60, 60, -1} // Notes/Instructions
};

/* Settings */
#define SIZ 5
#define VAL 0
#define MIN 1
#define MAX 2
#define MEM 3
const char SettingNames[SIZ][14] = {
	{"Pattern"},
	{"Tempo"},
	{"Octave"},
	{"Legato"},
	{"Velocity"},
};
int Settings[SIZ][4] = {
	{1, 0, 2, 0},
	{140, 60, 220, 1},
	{5, 3, 7, 2},
	{0, 0, 1, 3},
	{100, 1, 127, 4}};

/* Petterns */
#define PATLEN 16
const char PatternNames[3][14] = {{"Single"}, {"Scale"}, {"Arpeggio"}};
const int Patterns[3][2] [PATLEN] = {
	{
		{60, 0, 0, 0, 60, 0, 60, 0, 60, 0, 60, 0, 60, 0, 60, 0}, // note (the off and held note pitches will be ignored)
		{1, 0, -1, 0, 1, 0, -1, 0, 1, 0, -1, 0, 1, 0, -1, 0}	 // on, hold or off
	},
	{{60, 62, 63, 65, 67, 69, 71, 72, 70, 68, 67, 65, 63, 62, 60, 0},
	 {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, -1}},
	{{48, 60, 72, 48, 60, 72, 48, 60, 48, 60, 72, 48, 60, 72, 48, 60},
	 {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1}}};
int PlayNote = 0;
bool PlayPattern = false;
int PreviousNote = -1;

/* MIDI */
# include <MIDI.h>
// Creates and binds the MIDI interface to the default hardware Serial port
MIDI_CREATE_DEFAULT_INSTANCE();
const byte MidiOutChan = 0;

void setup() {

  pinMode(LED_BUILTIN, OUTPUT);
  
	/* MIDI */
	MIDI.begin(MIDI_CHANNEL_OFF); // Don't listen on any channel
	NoteOn(60,100,0);
	/* Serial */
#if DEBUG == 1
	// If Serial is on then MIDI wont work because of the diff baud rates
	// Serial must be begun after midi, to get the baud rate right.
	Serial.begin(115200);
#endif

	bool ResetDown = false;
	/* Buttons */
	for (int i = 0; i < BUTS; i++) {
		pinMode(Buttons[0][i], INPUT);
		digitalWrite(Buttons[0][i], HIGH);
		// If the current button happens to be the reset button, check if its down as well.
		if (Buttons[0][i] == RESETPIN) {
			ResetDown = !digitalRead(RESETPIN);
		}
#if DEBUG == 1
		Serial.print("Pin: ");
		Serial.print(Buttons[0][i], DEC);
		Serial.print("   State: ");
		Serial.println(Buttons[1][i], DEC);
#endif
	}

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
	for (int i = 0; i < BUTS; i++) {
		if (Buttons[2][i] > -1) {
			// It's a midi output button.
			if (digitalRead(Buttons[0][i]) == LOW && Buttons[1][i] == 0) {
				// If the button is on but was not previously on, send noteon.
				int Note = Buttons[2][i] + (Settings[2][VAL] - 5) * 12;
				NoteOn(Note, Settings[4][VAL], MidiOutChan);

				Buttons[1][i] = 1;
#if DEBUG == 1
				Serial.print("Button down: ");
				Serial.print(i, DEC);
				Serial.print("   State: ");
				Serial.println(Buttons[1][i], DEC);
#endif
			} else if (digitalRead(Buttons[0][i]) == HIGH && Buttons[1][i] == 1) {
				// If the button is off but not previously on, send noteoff.
				int Note = Buttons[2][i] + (Settings[2][VAL] - 5) * 12;
				NoteOff(Note, MidiOutChan);
				Buttons[1][i] = 0;
#if DEBUG == 1
				Serial.print("Button up: ");
				Serial.print(i, DEC);
				Serial.print("   State: ");
				Serial.println(Buttons[1][i], DEC);
#endif
			}
		} else {
			// It's not midi out, it needs to do something else...
			if (digitalRead(Buttons[0][i]) == LOW && Buttons[1][i] == 0) {
				switch (Buttons[2][i]) {
					case -1:
						PlayPattern = !PlayPattern;
						break;
				}
				Buttons[1][i] = 1;
#if DEBUG == 1
				Serial.print("Button down: ");
				Serial.print(i, DEC);
				Serial.print("   State: ");
				Serial.println(Buttons[1][i], DEC);
#endif
			} else if (digitalRead(Buttons[0][i]) == HIGH && Buttons[1][i] == 1) {
				Buttons[1][i] = 0;
			}
		}
	}

	if (PreviousNote > -1) {
		// If legato is off it won't hit this much.
#if DEBUG == 1
		Serial.print("Note off: ");
		Serial.println(PreviousNote, DEC);
#else
		MIDI.sendNoteOff(PreviousNote, 0, MidiOutChan);
#endif
		PreviousNote = -1; // No pattern notes currently playing.
		PlayNote = 0;     // Reset note pointer to 0 so it will play from the beginning when next started.
	}
}
