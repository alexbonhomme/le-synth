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
  while (!Serial) {
    delay(100);
  }
  Serial.println("Serial initialized");
#endif

  hardware->begin();
  audio->begin();

  midi->setHandleNoteOn(&Synth::noteOne);
  midi->setHandleNoteOff(&Synth::noteOff);
  midi->begin(defaults::midi_channel);
}

void Synth::process() {
  midi->read();
  hardware->update();
  state_->process();

// #ifdef DEBUG
//   Serial.print("Processor: ");
//   Serial.print(AudioProcessorUsage());
//   Serial.print(", ");
//   Serial.print(AudioProcessorUsageMax());
//   Serial.print("(max)");
//   Serial.print("    ");
//   Serial.print("Memory: ");
//   Serial.print(AudioMemoryUsage());
//   Serial.print(", ");
//   Serial.print(AudioMemoryUsageMax());
//   Serial.print("(max)");
//   Serial.println();
// #endif
}

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