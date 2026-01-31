#include "Synth.h"

namespace Autosave {

Synth::Synth() {
  instance_ = this;

  state_ = new MonoSynthState();
  state_->setSynth(this);

  hardware = new Hardware();
  audio = new Audio();
  midi = new Midi();
  
  midi->setHandleNoteOn(&Synth::midiNoteOn);
  midi->setHandleNoteOff(&Synth::midiNoteOff);
}

void Synth::begin() {

#ifdef DEBUG
  Serial.begin(115200);
  while (!Serial) {
    delay(100);
  }

  Serial.println("Initializing synth");
#endif

  hardware->begin();
  audio->begin();
  midi->begin();
}

void Synth::process() {
  midi->read();
  hardware->update();

  handleModeSwitch_();

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

void Synth::handleModeSwitch_() {
  // Handle mode switch
  if (!hardware->changed(hardware::CTRL_SWITCH_MODE)) {
    return;
  }

#ifdef DEBUG
  Serial.println("Updating mode: " +
                 String(hardware->read(hardware::CTRL_SWITCH_MODE)));
#endif

  byte mode = (byte)hardware->read(hardware::CTRL_SWITCH_MODE);

  switch (mode) {
  case 0:
#ifdef DEBUG
    Serial.println("Changing to mono synth state");
#endif
    changeState(new MonoSynthState());
    break;
  case 1:
// @TODO: Implement polyphonic synth state
#ifdef DEBUG
    Serial.println("Polyphonic synth state not implemented yet!");
#endif
    break;
  case 2:
// @TODO: Implement arpeggiator synth state
#ifdef DEBUG
    Serial.println("Arpeggiator synth state not implemented");
#endif
    break;
  default:
    break;
  }
}

void Synth::changeState(State *state) {
  if (state_ != nullptr) {
    delete state_;
  }

  state_ = state;
  state_->setSynth(this);
}

void Synth::midiNoteOn(byte channel, byte note, byte velocity) {
  if (instance_ == nullptr) {
    return;
  }

  instance_->state_->noteOn(note, velocity);
}

void Synth::midiNoteOff(byte channel, byte note, byte velocity) {
  if (instance_ == nullptr) {
    return;
  }

  instance_->state_->noteOff(note, velocity);
}

} // namespace Autosave