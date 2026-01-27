
#include "Midi.h"

namespace Autosave {

// Create and bind the MIDI interface to the default hardware Serial port
MIDI_CREATE_INSTANCE(HardwareSerial, Serial1, MIDI);

Midi::Midi() {}

void Midi::setHandleNoteOn(void (*callback)(byte channel, byte note,
                                            byte velocity)) {
  usbMIDI.setHandleNoteOn(callback);
  MIDI.setHandleNoteOn(callback);
}

void Midi::setHandleNoteOff(void (*callback)(byte channel, byte note,
                                             byte velocity)) {
  usbMIDI.setHandleNoteOff(callback);
  MIDI.setHandleNoteOff(callback);
}

void Midi::begin(unsigned char channel) { MIDI.begin(channel); }

void Midi::read() {
  usbMIDI.read();
  MIDI.read();
}

} // namespace Autosave
