#ifndef Controller_h
#define Controller_h

#include <Arduino.h>

//***********************************************************************
class Mux {
   public:
    Mux(byte outpin_, byte numPins_, bool analog_);
    byte outpin;
    byte numPins;
    bool analog;
};
//************************************************************************
//Button (Pin Number, Command, Note Number, Channel, Debounce Time)
class Button {
   public:
    void init(byte command, byte value, byte channel, byte debounce);
    Button(byte pin, byte command, byte value, byte channel, byte debounce);
    Button(Mux mux, byte muxpin, byte command, byte value, byte channel, byte debounce);
    byte getValue();
    void muxUpdate();
    void newValue(byte command, byte value, byte channel);
    byte Bcommand;
    byte Bvalue;
    byte Bchannel;
    byte Btoggle;

    void Led(bool on);

   private:
    byte _previous;
    byte _current;
    unsigned long _time;
    int _debounce;
    byte _pin;
    byte _muxpin;
    byte _numMuxPins;
    byte _value;
    byte _command;
    bool _busy;
    byte _status;
    byte _last;
    byte _enablepin;

    byte _led_type;  // 0=none; 1=normal; 2=mux;
    byte _led_pin;
    byte _led_muxpin;
    byte _led_numMuxPins;
};
//*************************************************************************
//*************************************************************************
class Pot {
   public:
    void init(byte command, byte control, byte channel);
    Pot(byte pin, byte command, byte control, byte channel);
    Pot(byte pin, byte command, byte control, byte channel, byte led_pin);
    Pot(Mux mux, byte muxpin, byte command, byte control, byte channel);
    void muxUpdate();
    void newValue(byte command, byte value, byte channel);
    byte getValue();
    void calibrate();
    void calLoad(int iAddress);
    void calSave(int iAddress);
    byte _previous;
    byte Pcommand;  // 0=Analogue CC; 1=Button (CC); 2=Toggle Button (CC);
    byte Pcontrol;
    byte Pchannel;
    bool Ptoggle;

    void Led(bool on);

   private:
    byte _pin;
    byte _muxpin;
    byte _numMuxPins;
    byte _control;
    int _value;
    int _oldValue;
    bool _cal;
    int _calHigh;
    int _calLow;
    bool _changed;
    byte _enablepin;

    byte _led_type;  // 0=none; 1=normal; 2=mux;
    byte _led_pin;
    byte _led_muxpin;
    byte _led_numMuxPins;
};

#endif
