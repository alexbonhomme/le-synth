#ifndef AUTOSAVE_SYNTH_H
#define AUTOSAVE_SYNTH_H

#include "Audio.h"
#include "Hardware.h"
#include "Midi.h"
#include "State.h"

namespace Autosave {
namespace defaults {
static constexpr byte midi_channel = 8;
} // namespace defaults

class Synth {
private:
  inline static Synth *instance_ = nullptr;
  State *state_;

public:
  Synth();

  Audio *audio;
  Midi *midi;
  Hardware *hardware;

  void begin();
  void process();
  void changeState(State *state);

  static void noteOne(byte channel, byte note, byte velocity);
  static void noteOff(byte channel, byte note, byte velocity);
};

} // namespace Autosave

#endif