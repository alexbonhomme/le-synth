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

  static void customWaveformSysexGetter(uint8_t *bank, uint8_t *index);
  static void customWaveformSysexSetter(uint8_t bank, uint8_t index);

  static void arpStepsSysexGetter(uint8_t mode, uint8_t *len, uint8_t *data);
  static void arpStepsSysexSetter(uint8_t mode, uint8_t len, const uint8_t *data);
};

} // namespace Autosave

#endif