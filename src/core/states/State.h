#ifndef AUTOSAVE_STATE_H
#define AUTOSAVE_STATE_H

#include <Arduino.h>

namespace Autosave {
class Synth;

class State {
protected:
  Synth *synth_;

public:
  virtual ~State() {}

  void setSynth(Synth *synth) { this->synth_ = synth; }

  virtual void process();
  virtual void noteOn(byte note, byte velocity) = 0;
  virtual void noteOff(byte note, byte velocity) = 0;
};
} // namespace Autosave

#endif