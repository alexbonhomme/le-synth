#ifndef AUTOSAVE_POLY_SYNTH_STATE_H
#define AUTOSAVE_POLY_SYNTH_STATE_H

#include "State.h"
#include "core/Audio.h"
#include "core/Midi.h"

namespace Autosave {

class PolySynthState : public State {
private:
  float detune_ = 0.0f;

  MidiNote current_notes_[audio_config::voices_number];
  uint8_t note_count_ = 0;

public:
  void begin() override;
  void process() override;
  void noteOn(MidiNote note) override;
  void noteOff(MidiNote note) override;
};
} // namespace Autosave

#endif