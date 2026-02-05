#ifndef AUTOSAVE_STATE_H
#define AUTOSAVE_STATE_H

#include <Arduino.h>

#include "core/Midi.h"

namespace Autosave {
class Synth;

class State {
protected:
  Synth *synth_;

public:
  virtual ~State() {}

  void setSynth(Synth *synth) { this->synth_ = synth; }

  virtual void begin() = 0;
  virtual void process();
  virtual void noteOn(MidiNote note) = 0;
  virtual void noteOff(MidiNote note) = 0;
};
} // namespace Autosave

#endif