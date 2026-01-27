#ifndef AUTOSAVE_MIDI_H
#define AUTOSAVE_MIDI_H

#include <Arduino.h>
#include <MIDI.h>

namespace Autosave {

class Midi {
public:
  Midi();

  void setHandleNoteOn(void (*callback)(byte channel, byte note,
                                        byte velocity));
  void setHandleNoteOff(void (*callback)(byte channel, byte note,
                                         byte velocity));
  void begin(unsigned char channel);
  void read();
};

} // namespace Autosave

#endif