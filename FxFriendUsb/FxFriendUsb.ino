#include <MIDIUSB.h>
#include "Controller.h"
#include "lib\EEPROM.h"
#include "lib\MIDI.h"
//#include "lib\MIDI_Interfaces\USBMIDI_Interface.h"
/*************************************************************
  MIDI CONTROLLER

  by Notes and Volts
  www.notesandvolts.com

  Version 1.2 **Arduino UNO ONLY!**
 *************************************************************/
#define USB 1  // 1 = Midi over usb; 0 = Midi over serial.

#if USB != 1
MIDI_CREATE_DEFAULT_INSTANCE();
#endif

#define _CAL 12  // Calibration pin.
#define _LED 13  // Led.

//************************************************************
//***SET THE NUMBER OF CONTROLS USED**************************
//************************************************************
//---How many buttons are connected to a multiplexer?---------
byte NUMBER_MUX_BUTTONS = 0;
//---How many potentiometers are connected to a multiplexer?--
byte NUMBER_MUX_POTS = 0;
//************************************************************

//***ANY MULTIPLEXERS? (74HC4067)************************************
//MUX address pins must be connected to Arduino UNO pins 2,3,4,5
//A0 = PIN2, A1 = PIN3, A2 = PIN4, A3 = PIN5
//*******************************************************************
//Mux NAME (OUTPUT PIN, , How Many Mux Pins?(8 or 16) , Is It Analog?);

//Mux M1(10, 16, false); //Digital multiplexer on Arduino pin 10
//Mux M2(A5, 8, true); //Analog multiplexer on Arduino analog pin A5
//*******************************************************************

//***DEFINE DIRECTLY CONNECTED POTENTIOMETERS************************
//Pot (Pin Number, Command, CC Control, Channel Number)
//Pot (Pin Number, Command, CC Control, Channel Number, Led Pin)
//** Command parameter 0=Analogue CC  1=Button (CC)  2=Toggle Button (CC) **
//** Led Type 0=None; 1=Normal; 2=Mux **

Pot PO1(A0, 1, 1, 1, 5);
Pot PO2(A1, 2, 10, 1, 4);
Pot PO3(A2, 2, 22, 1, 3);
Pot PO4(A3, 2, 118, 1, 2);
//Pot PO5(A4, 0, 30, 1);
//Pot PO6(A5, 0, 31, 1);
//*******************************************************************
//---How many potentiometers are connected directly to pins?--
byte NUMBER_POTS = 4;
//Add pots used to array below like this->  Pot *POTS[] {&PO1, &PO2, &PO3, &PO4, &PO5, &PO6};
Pot *POTS[]{&PO1, &PO2, &PO3, &PO4};
//*******************************************************************

//***DEFINE DIRECTLY CONNECTED BUTTONS*******************************
//Button (Pin Number, Command, Note Number, Channel, Debounce Time)
//** Command parameter 0=NOTE  1=CC  2=Toggle CC **

//Button BU1(2, 2, 60, 1, 5);
//Button BU2(3, 0, 61, 1, 5 );
//Button BU3(4, 0, 62, 1, 5 );
//Button BU4(5, 0, 63, 1, 5 );
//Button BU5(6, 0, 64, 1, 5 );
//Button BU6(7, 0, 65, 1, 5 );
//Button BU7(8, 1, 64, 1, 5 );
//Button BU8(9, 2, 64, 1, 5 );
//Button CAL(12, 0, 60, 1, 5 ); // Calibration button. Don't put in the pointer array.
//*******************************************************************
//---How many buttons are connected directly to pins?---------
byte NUMBER_BUTTONS = 0;
//Add buttons used to array below like this->  Button *BUTTONS[] {&BU1, &BU2, &BU3, &BU4, &BU5, &BU6, &BU7, &BU8};
Button *BUTTONS[]{};
//*******************************************************************

//***DEFINE BUTTONS CONNECTED TO MULTIPLEXER*************************
//Button::Button(Mux mux, byte muxpin, byte command, byte value, byte channel, byte debounce)
//** Command parameter 0=NOTE  1=CC  2=Toggle CC **

//Button MBU1(M1, 0, 0, 70, 1, 5);
//Button MBU2(M1, 1, 1, 71, 1, 5);
//Button MBU3(M1, 2, 2, 72, 1, 5);
//Button MBU4(M1, 3, 0, 73, 1, 5);
//Button MBU5(M1, 4, 0, 74, 1, 5);
//Button MBU6(M1, 5, 0, 75, 1, 5);
//Button MBU7(M1, 6, 0, 76, 1, 5);
//Button MBU8(M1, 7, 0, 77, 1, 5);
//Button MBU9(M1, 8, 0, 78, 1, 5);
//Button MBU10(M1, 9, 0, 79, 1, 5);
//Button MBU11(M1, 10, 0, 80, 1, 5);
//Button MBU12(M1, 11, 0, 81, 1, 5);
//Button MBU13(M1, 12, 0, 82, 1, 5);
//Button MBU14(M1, 13, 0, 83, 1, 5);
//Button MBU15(M1, 14, 0, 84, 1, 5);
//Button MBU16(M1, 15, 0, 85, 1, 5);
//*******************************************************************
////Add multiplexed buttons used to array below like this->  Button *MUXBUTTONS[] {&MBU1, &MBU2, &MBU3, &MBU4, &MBU5, &MBU6.....};
Button *MUXBUTTONS[]{};

//*******************************************************************

//***DEFINE POTENTIOMETERS CONNECTED TO MULTIPLEXER*******************
//Pot::Pot(Mux mux, byte muxpin, byte command, byte control, byte channel)
//**Command parameter is for future use**

//Pot MPO1(M2, 0, 0, 1, 1);
//Pot MPO2(M2, 1, 0, 7, 1);
//Pot MPO3(M2, 2, 0, 50, 1);
//Pot MPO4(M2, 3, 0, 55, 2);
//Pot MPO5(M2, 4, 0, 50, 1);
//Pot MPO6(M2, 5, 0, 55, 2);
//Pot MPO7(M2, 6, 0, 50, 1);
//Pot MPO8(M2, 7, 0, 55, 2);
//Pot MPO9(M2, 8, 0, 50, 1);
//Pot MPO10(M2, 9, 0, 55, 2);
//Pot MPO11(M2, 10, 0, 50, 1);
//Pot MPO12(M2, 11, 0, 55, 2);
//Pot MPO13(M2, 12, 0, 50, 1);
//Pot MPO14(M2, 13, 0, 55, 2);
//Pot MPO15(M2, 14, 0, 50, 1);
//Pot MPO16(M2, 15, 0, 55, 2);
//*******************************************************************
//Add multiplexed pots used to array below like this->  Pot *MUXPOTS[] {&MPO1, &MPO2, &MPO3, &MPO4, &MPO5, &MPO6.....};
Pot *MUXPOTS[]{};
//*******************************************************************

void setup() {
    pinMode(_CAL, INPUT_PULLUP);

#if USB != 1
    MIDI.begin(MIDI_CHANNEL_OFF);
#endif
    //  Serial.begin(115200);
    loadPots();

    bool bButtDown = buttonPushed(_CAL);
    if (bButtDown) {
        // Enter calibration procedure.
        digitalWrite(_LED, HIGH);
        while (true) {
            //	bButtDown = bButtDown && !buttonPushed(_CAL);
            calibratePots();
            if (!buttonPushed(_CAL) && bButtDown) {
                bButtDown = false;                          // Button was released.
            } else if (buttonPushed(_CAL) && !bButtDown) {  //TODO? Debounce?
                break;                                      // Button was pushed a 2nd time. Exit callibration.
            }
        }
        savePots();
        digitalWrite(_LED, LOW);
    }
}

void loop() {
    if (NUMBER_BUTTONS != 0)
        updateButtons();
    if (NUMBER_POTS != 0)
        updatePots();
    if (NUMBER_MUX_BUTTONS != 0)
        updateMuxButtons();
    if (NUMBER_MUX_POTS != 0)
        updateMuxPots();
}
void midiNoteOn(uint8_t inNoteNumber, uint8_t inVelocity, uint8_t inChannel) {
    if (USB) {
        midiEventPacket_t noteOn = {0x09, 0x90 | inChannel, inNoteNumber, inVelocity};
        MidiUSB.sendMIDI(noteOn);
        MidiUSB.flush();
    } else {
#if USB != 1
        MIDI.sendNoteOn(inNoteNumber, inVelocity, inChannel);
#endif
    }
}
void midiNoteOff(uint8_t inNoteNumber, uint8_t inVelocity, uint8_t inChannel) {
    if (USB) {
        midiEventPacket_t noteOff = {0x08, 0x80 | inChannel, inNoteNumber, inVelocity};
        MidiUSB.sendMIDI(noteOff);
        MidiUSB.flush();
    } else {
#if USB != 1
        MIDI.sendNoteOff(inNoteNumber, inVelocity, inChannel);
#endif
    }
}
void midiControlChange(uint8_t inControlNumber, uint8_t inControlValue, uint8_t inChannel) {
    if (USB == 1) {
        midiEventPacket_t change = {0x0B, 0xB0 | inChannel, inControlNumber, inControlValue};
        MidiUSB.sendMIDI(change);
        MidiUSB.flush();
    } else {
#if USB != 1
        MIDI.sendControlChange(inControlNumber, inControlValue, inChannel);
#endif
    }
}

//*****************************************************************
void updateButtons() {
    // Cycle through Button array
    for (int i = 0; i < NUMBER_BUTTONS; i++) {
        byte message = BUTTONS[i]->getValue();

        //  Button is pressed.
        if (message == 0) {
            switch (BUTTONS[i]->Bcommand) {
                case 0:  //Note
                    midiNoteOn(BUTTONS[i]->Bvalue, 127, BUTTONS[i]->Bchannel);
                    break;
                case 1:  //CC
                    midiControlChange(BUTTONS[i]->Bvalue, 127, BUTTONS[i]->Bchannel);
                    break;
                case 2:  //Toggle
                    if (BUTTONS[i]->Btoggle == 0) {
                        midiControlChange(BUTTONS[i]->Bvalue, 127, BUTTONS[i]->Bchannel);
                        BUTTONS[i]->Btoggle = 1;
                    } else if (BUTTONS[i]->Btoggle == 1) {
                        midiControlChange(BUTTONS[i]->Bvalue, 0, BUTTONS[i]->Bchannel);
                        BUTTONS[i]->Btoggle = 0;
                    }
                    break;
            }
        }

        //  Button is not released.
        if (message == 1) {
            switch (BUTTONS[i]->Bcommand) {
                case 0:
                    midiNoteOff(BUTTONS[i]->Bvalue, 0, BUTTONS[i]->Bchannel);
                    break;
                case 1:
                    midiControlChange(BUTTONS[i]->Bvalue, 0, BUTTONS[i]->Bchannel);
                    break;
            }
        }
    }
}
//*******************************************************************
void updateMuxButtons() {
    // Cycle through Mux Button array
    for (int i = 0; i < NUMBER_MUX_BUTTONS; i++) {
        MUXBUTTONS[i]->muxUpdate();
        byte message = MUXBUTTONS[i]->getValue();

        //  Button is pressed
        if (message == 0) {
            switch (MUXBUTTONS[i]->Bcommand) {
                case 0:  //Note
                    midiNoteOn(MUXBUTTONS[i]->Bvalue, 127, MUXBUTTONS[i]->Bchannel);
                    break;
                case 1:  //CC
                    midiControlChange(MUXBUTTONS[i]->Bvalue, 127, MUXBUTTONS[i]->Bchannel);
                    break;
                case 2:  //Toggle
                    if (MUXBUTTONS[i]->Btoggle == 0) {
                        midiControlChange(MUXBUTTONS[i]->Bvalue, 127, MUXBUTTONS[i]->Bchannel);
                        MUXBUTTONS[i]->Btoggle = 1;
                    } else if (MUXBUTTONS[i]->Btoggle == 1) {
                        midiControlChange(MUXBUTTONS[i]->Bvalue, 0, MUXBUTTONS[i]->Bchannel);
                        MUXBUTTONS[i]->Btoggle = 0;
                    }
                    break;
            }
        }
        //  Button is not pressed
        if (message == 1) {
            switch (MUXBUTTONS[i]->Bcommand) {
                case 0:
                    midiNoteOff(MUXBUTTONS[i]->Bvalue, 0, MUXBUTTONS[i]->Bchannel);
                    break;
                case 1:
                    midiControlChange(MUXBUTTONS[i]->Bvalue, 0, MUXBUTTONS[i]->Bchannel);
                    break;
            }
        }
    }
}
//***********************************************************************
void updatePots() {
    for (int i = 0; i < NUMBER_POTS; i++) {
        byte potmessage = POTS[i]->getValue();

        if (potmessage != 255) {
            switch (POTS[i]->Pcommand) {
                case 0:  // Analogue
                    midiControlChange(POTS[i]->Pcontrol, potmessage, POTS[i]->Pchannel);
                    POTS[i]->Led(potmessage > 0);
                    break;

                case 1:  // Button
                    if (potmessage == 0) {
                        midiControlChange(POTS[i]->Pcontrol, 127, POTS[i]->Pchannel);
                        POTS[i]->Led(true);
                    } else {
                        POTS[i]->Led(false);
                        midiControlChange(POTS[i]->Pcontrol, 0, POTS[i]->Pchannel);
                    }
                    POTS[i]->Led(potmessage == 0);
                    break;

                case 2:  // Toggle Button
                    if (potmessage == 0) {
                        POTS[i]->Ptoggle = !POTS[i]->Ptoggle;

                        if (POTS[i]->Ptoggle) {
                            midiControlChange(POTS[i]->Pcontrol, 127, POTS[i]->Pchannel);
                        } else {
                            midiControlChange(POTS[i]->Pcontrol, 0, POTS[i]->Pchannel);
                        }
                        POTS[i]->Led(POTS[i]->Ptoggle);
                    }
                    break;
            }
        }
    }
}
//***********************************************************************
void updateMuxPots() {
    for (int i = 0; i < NUMBER_MUX_POTS; i = i + 1) {
        MUXPOTS[i]->muxUpdate();
        byte potmessage = MUXPOTS[i]->getValue();
        if (potmessage != 255)
            midiControlChange(MUXPOTS[i]->Pcontrol, potmessage, MUXPOTS[i]->Pchannel);
    }
}
void calibratePots() {
    for (int i = 0; i < NUMBER_POTS; i++) {
        POTS[i]->calibrate();
    }
}
void loadPots() {
    for (int i = 0; i < NUMBER_POTS; i++) {
        // Each pot has two ints (_calLow and _calHigh).
        // Each int is two bytes.
        // Therefore each pot occupies 4 bytes/addresses.
        POTS[i]->calLoad(i * sizeof(int) * 2);
    }
}
void savePots() {
    for (int i = 0; i < NUMBER_POTS; i++) {
        // Each pot has two ints (_calLow and _calHigh).
        // Each int is two bytes.
        // Therefore each pot occupies 4 bytes/addresses.
        POTS[i]->calSave(i * sizeof(int) * 2);
    }
}

bool buttonPushed(byte buttonPin) {
    return (digitalRead(buttonPin) == LOW);
}
