#ifndef AUTOSAVE_SYNTH_H
#define AUTOSAVE_SYNTH_H

#include "Audio.h"
#include "Hardware.h"
#include "Midi.h"
#include "states/State.h"

namespace Autosave {

class Synth {
private:
  inline static Synth *instance_ = nullptr;
  State *state_;

  void updateMode();
  void debugAudioUsage();

public:
  Synth();

  Audio *audio;
  Midi *midi;
  Hardware *hardware;

  void begin();
  void process();
  void changeState(State *state);

  static void midiNoteOn(uint8_t channel, uint8_t note, uint8_t velocity);
  static void midiNoteOff(uint8_t channel, uint8_t note, uint8_t velocity);
};

} // namespace Autosave

#endif