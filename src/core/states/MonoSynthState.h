#ifndef AUTOSAVE_MONO_SYNTH_STATE_H
#define AUTOSAVE_MONO_SYNTH_STATE_H

#include <Arduino.h>

#include "State.h"
#include "lib/Logger.h"
namespace Autosave {

class MonoSynthState : public State {
private:
  float detune_ = 0.0f;
  byte current_note_ = 0;

public:
  void begin() override;
  void process() override;
  void noteOn(byte note, byte velocity) override;
  void noteOff(byte note, byte velocity) override;
};
} // namespace Autosave

#endif