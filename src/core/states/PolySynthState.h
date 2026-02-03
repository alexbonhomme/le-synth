#ifndef AUTOSAVE_POLY_SYNTH_STATE_H
#define AUTOSAVE_POLY_SYNTH_STATE_H

#include <Arduino.h>

#include "State.h"
#include "core/Audio.h"

namespace Autosave {

class PolySynthState : public State {
private:
  float detune_ = 0.0f;

  byte current_notes_[audio_config::voices_number] = {0};
  byte note_count_ = 0;

public:
  void begin() override;
  void process() override;
  void noteOn(byte note, byte velocity) override;
  void noteOff(byte note, byte velocity) override;
};
} // namespace Autosave

#endif