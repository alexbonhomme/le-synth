#ifndef AUTOSAVE_STATE_H
#define AUTOSAVE_STATE_H

#include "core/Midi.h"

namespace Autosave {
class Synth;

enum WaveformType {
  SYNTH_WAVEFORM_SAWTOOTH = 0,
  SYNTH_WAVEFORM_SQUARE = 1,
  SYNTH_WAVEFORM_CUSTOM = 2,
};
class State {
protected:
  Synth *synth_;

  void loadWaveform(WaveformType waveform_type);

public:
  virtual ~State() {}

  void setSynth(Synth *synth) { this->synth_ = synth; }

  virtual void begin();
  virtual void process();
  virtual void noteOn(MidiNote note) = 0;
  virtual void noteOff(MidiNote note) = 0;
};
} // namespace Autosave

#endif