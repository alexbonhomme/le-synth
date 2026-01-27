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

  virtual void noteOn(byte note, byte velocity) = 0;
  virtual void noteOff(byte note, byte velocity) = 0;
  virtual void process() = 0;
};

class MonoSynthState : public State {
public:
  void noteOn(byte note, byte velocity) override;
  void noteOff(byte note, byte velocity) override;
  void process() override;
};
} // namespace Autosave

#endif