#ifndef AUTOSAVE_MONO_SYNTH_STATE_H
#define AUTOSAVE_MONO_SYNTH_STATE_H

#include "State.h"
#include "core/Midi.h"

namespace Autosave {

class MonoSynthState : public State {
private:
  float detune_ = 0.0f;
  MidiNote current_note_ = {0, 0};

public:
  void begin() override;
  void process() override;
  void noteOn(MidiNote note) override;
  void noteOff(MidiNote note) override;
};
} // namespace Autosave

#endif