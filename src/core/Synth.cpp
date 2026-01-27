#include "Synth.h"

namespace Autosave {

Synth::Synth() {
  instance_ = this;

  state_ = new MonoSynthState();
  state_->setSynth(this);

  hardware = new Hardware();
  audio = new Audio();
  midi = new Midi();
}

void Synth::begin() {

#ifdef DEBUG
  Serial.begin(115200);
#endif

  hardware->begin();
  audio->begin();

  midi->setHandleNoteOn(&Synth::noteOne);
  midi->setHandleNoteOff(&Synth::noteOff);
  midi->begin(defaults::midi_channel);
}

void Synth::process() { state_->process(); }

void Synth::changeState(State *state) {
  if (state_ != nullptr) {
    delete state_;
  }

  state_ = state;
  state_->setSynth(this);
}

void Synth::noteOne(byte channel, byte note, byte velocity) {
  if (instance_ == nullptr) {
    return;
  }

  instance_->state_->noteOn(note, velocity);
}

void Synth::noteOff(byte channel, byte note, byte velocity) {
  if (instance_ == nullptr) {
    return;
  }

  instance_->state_->noteOff(note, velocity);
}

} // namespace Autosave