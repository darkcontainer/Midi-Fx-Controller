#include "Controller.h"
#include "lib\EEPROM.h"
//#include "lib\MIDIUSB.h"

//****************************************************************************************
Mux::Mux(byte outpin_, byte numPins_, bool analog_) {
    outpin = outpin_;
    // enablepin = enablepin_;
    numPins = numPins_;
    analog = analog_;
    if (analog == false) pinMode(outpin, INPUT_PULLUP);
    // pinMode(enablepin, OUTPUT);
    pinMode(2, OUTPUT);
    pinMode(3, OUTPUT);
    pinMode(4, OUTPUT);
    if (numPins > 8) pinMode(5, OUTPUT);
}
//****************************************************************************************
void Button::init(byte command, byte value, byte channel, byte debounce) {
    _value = value;
    _command = command;
    _debounce = debounce;
    _time = 0;
    _busy = false;
    _status = 0b00000010;
    _last = 1;
    Bcommand = command;
    Bvalue = value;
    Bchannel = channel;
    Btoggle = 0;
}
// Button (Pin Number, Command, Note Number, Channel, Debounce Time)
Button::Button(byte pin, byte command, byte value, byte channel, byte debounce) {
    init(command, value, channel, debounce);
    _pin = pin;
    pinMode(_pin, INPUT_PULLUP);
}

Button::Button(Mux mux, byte muxpin, byte command, byte value, byte channel, byte debounce) {
    init(command, value, channel, debounce);
    _pin = mux.outpin;
    _numMuxPins = mux.numPins;
    _muxpin = muxpin;
}

void Button::muxUpdate() {
    byte temp = _muxpin;
    temp = temp << 2;
    if (_numMuxPins > 8)
        PORTD = PORTD & B11000011;
    else
        PORTD = PORTD & B11100011;
    PORTD = PORTD | temp;
}

byte Button::getValue() {
    // If BUSY bit not set - read button.
    if (bitRead(_status, 0) == false) {  // If busy false
        if (digitalRead(_pin) == _last) {
            return 255;  // If same as last state - exit.
        }
    }

    // If NEW Bit set - Key just pressed, record time.
    if (bitRead(_status, 1) == true) {  // If new is true
        bitSet(_status, 0);             // Set busy TRUE
        bitClear(_status, 1);           // Set New FALSE
        _time = millis();
        return 255;
    }

    // Check if debounce time has passed - If no, exit
    if (millis() - _time < _debounce) {
        return 255;
    }

    // Debounce time has passed. Read pin to see if still set the same.
    // If it has changed back - assume false alarm.
    if (digitalRead(_pin) == _last) {
        bitClear(_status, 0);  // Set busy false.
        bitSet(_status, 1);    // Set new true.
        return 255;
    } else {
        // If this point is reached, event is valid. Return event type.
        bitClear(_status, 0);             // Set busy false
        bitSet(_status, 1);               // Set new true
        _last = ((~_last) & 0b00000001);  // invert _last
        return _last;
    }
}

void Button::newValue(byte command, byte value, byte channel) {
    Bvalue = value;
    Bcommand = command;
    Bchannel = channel;
}
void Button::Led(bool on) {
    if (on) {
        digitalWrite(_led_pin, HIGH);
    } else {
        digitalWrite(_led_pin, LOW);
    }
}

//********************************************************************
void Pot::init(byte command, byte control, byte channel) {
    _control = control;
    _cal = false;
    _calHigh = 1023;
    _calLow = 0;
    _value = 0;
    _value = _value >> 3;
    _oldValue = _value << 3;
    _value = _value << 3;
    _previous = 1;
    Pcommand = command;
    Pcontrol = control;
    Pchannel = channel;
    Ptoggle = false;
}
Pot::Pot(byte pin, byte command, byte control, byte channel) {
   init(command, control, channel);
    _pin = pin;
}
Pot::Pot(byte pin, byte command, byte control, byte channel, byte led_pin) {
    init( command, control, channel);
    _pin = pin;
    _led_pin = led_pin;
    _led_type = 1;  // 0=none; 1=normal; 2=mux;
}

void Pot::muxUpdate() {
    byte temp = _muxpin;
    temp = temp << 2;
    if (_numMuxPins > 8)
        PORTD = PORTD & B11000011;
    else
        PORTD = PORTD & B11100011;
    // PORTD = PORTD & B11000011;
    PORTD = PORTD | temp;
}

Pot::Pot(Mux mux, byte muxpin, byte command, byte control, byte channel) {
    init(command, control, channel);
    _pin = mux.outpin;
    _numMuxPins = mux.numPins;
    _muxpin = muxpin;
    _control = control;
}
void Pot::Led(bool on) {
    switch (_led_type) {
        case 1:
            if (on) {
                digitalWrite(_led_pin, HIGH);
            } else {
                digitalWrite(_led_pin, LOW);
            }
            break;
        case 2:
            // Do mux led. (somehow)
            break;
    }
}
byte Pot::getValue() {
    _value = analogRead(_pin);
    // Apply calibration (this is just a simplified version of the map() function).
    // (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
    //_value = ((_value - _calLow) * (int)1023) / (_calHigh - _calLow); // Does not work. I don't know why.
    _value = map(_value, _calLow, _calHigh, 0, 1023);

    // Apply upper and lower limits for if it overshot (otherwise we get over/under flow).
    if (_value > 1023) {
        _value = 1023;
    } else if (_value < 0) {
        _value = 0;
    }

    int diff = (_oldValue - _value);
    if (diff >= 8 || diff <= -8) {
        _oldValue = _value >> 3;
        _oldValue = _oldValue << 3;
        if (Pcommand == 0) {
			// Regular analogue control behavior.
            return _value >> 3;
        } else {
			// Button behaviour.
            byte newval = _value >> 3;
            if (newval >= 64 && _previous < 64) {
				_previous = newval;
                return 0;  // Pressed.
            } else if (newval < 64 && _previous >= 64) {
				_previous = newval;
                return 1;  // Released.
            }
        }
    }
    return 255;
}

void Pot::calibrate() {
    int tmp = analogRead(_pin);
    int thresh = 511;
    // Ignore all changes below 50% (511 for analogue, 63 for velocity).
    if (tmp > thresh && !_cal) {
        _cal = true;
        _calHigh = tmp;
        _calLow = thresh;
    }
    if (_cal && tmp > _calHigh) {
        _calHigh = tmp;
    } else if (_cal && tmp < _calLow) {
        _calLow = tmp;
    }
    // if (_cal){
    //  Serial.print("Low:  ");
    //  Serial.print(_calLow,DEC);
    //  Serial.print("  High: ");
    //  Serial.print(_calHigh,DEC);
    //  Serial.print("  ");
    // }
}
void Pot::calLoad(int iAddress) {
    int iVal = 0;
    EEPROM.get(iAddress, iVal);
    _calLow = iVal;
    iAddress += sizeof(int);
    EEPROM.get(iAddress, iVal);
    _calHigh = iVal;
}
void Pot::calSave(int iAddress) {
    int iVal = 0;

    EEPROM.get(iAddress, iVal);
    if (iVal != _calLow) {
        EEPROM.put(iAddress, _calLow);
    }

    iAddress += sizeof(int);

    EEPROM.get(iAddress, iVal);
    if (iVal != _calHigh) {
        EEPROM.put(iAddress, _calHigh);
    }
}

void Pot::newValue(byte command, byte value, byte channel) {
    Pcommand = command;
    Pcontrol = value;
    Pchannel = channel;
}
