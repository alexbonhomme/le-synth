#ifndef AUTOSAVE_SYNTH_H
#define AUTOSAVE_SYNTH_H

#include <Arduino.h>

#include "Audio.h"
#include "Hardware.h"
#include "Midi.h"
#include "State.h"

namespace Autosave {

class Synth {
private:
  inline static Synth *instance_ = nullptr;
  State *state_;
  void handleModeSwitch_();

public:
  Synth();

  Audio *audio;
  Midi *midi;
  Hardware *hardware;

  void begin();
  void process();
  void changeState(State *state);

  static void midiNoteOn(byte channel, byte note, byte velocity);
  static void midiNoteOff(byte channel, byte note, byte velocity);
};

} // namespace Autosave

#endif