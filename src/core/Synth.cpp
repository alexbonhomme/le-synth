#include "Synth.h"
#include "lib/Logger.h"
#include "states/MonoSynthState.h"

namespace Autosave {

Synth::Synth() {
  instance_ = this;

  hardware = new Hardware();
  audio = new Audio();
  midi = new Midi();

  state_ = new MonoSynthState();
  state_->setSynth(this);

  midi->setHandleNoteOn(&Synth::midiNoteOn);
  midi->setHandleNoteOff(&Synth::midiNoteOff);
}

void Synth::begin() {
  AutosaveLib::Logger::begin(AutosaveLib::Logger::LEVEL_INFO);
  AutosaveLib::Logger::info("Initializing Synth module");

  hardware->begin();
  audio->begin();
  midi->begin();
  state_->begin();
}

void Synth::process() {
  midi->read();
  hardware->update();

  handleModeSwitch_();

  state_->process();

  AutosaveLib::Logger::print("Processor: ", AutosaveLib::Logger::LEVEL_DEBUG);
  AutosaveLib::Logger::print(AudioProcessorUsage(),
                             AutosaveLib::Logger::LEVEL_DEBUG);
  AutosaveLib::Logger::print(", ", AutosaveLib::Logger::LEVEL_DEBUG);
  AutosaveLib::Logger::print(AudioProcessorUsageMax(),
                             AutosaveLib::Logger::LEVEL_DEBUG);
  AutosaveLib::Logger::print("(max)", AutosaveLib::Logger::LEVEL_DEBUG);
  AutosaveLib::Logger::println("    ", AutosaveLib::Logger::LEVEL_DEBUG);
  AutosaveLib::Logger::print("Memory: ", AutosaveLib::Logger::LEVEL_DEBUG);
  AutosaveLib::Logger::print(AudioMemoryUsage(),
                             AutosaveLib::Logger::LEVEL_DEBUG);
  AutosaveLib::Logger::print(", ", AutosaveLib::Logger::LEVEL_DEBUG);
  AutosaveLib::Logger::print(AudioMemoryUsageMax(),
                             AutosaveLib::Logger::LEVEL_DEBUG);
  AutosaveLib::Logger::println("(max)", AutosaveLib::Logger::LEVEL_DEBUG);
}

void Synth::handleModeSwitch_() {
  // Handle mode switch
  if (!hardware->changed(hardware::CTRL_SWITCH_MODE)) {
    return;
  }

  AutosaveLib::Logger::debug(
      "Updating mode: " + String(hardware->read(hardware::CTRL_SWITCH_MODE)));

  byte mode = (byte)hardware->read(hardware::CTRL_SWITCH_MODE);

  switch (mode) {
  case 0:
    AutosaveLib::Logger::debug("Changing to mono synth state");
    changeState(new MonoSynthState());
    break;
  case 1:
    // @TODO: Implement polyphonic synth state
    AutosaveLib::Logger::warn("Polyphonic synth state not implemented yet!");
    break;
  case 2:
    // @TODO: Implement arpeggiator synth state
    AutosaveLib::Logger::warn("Arpeggiator synth state not implemented");
    break;
  default:
    AutosaveLib::Logger::warn("Invalid mode: " + String(mode));
    break;
  }
}

void Synth::changeState(State *state) {
  if (state_ != nullptr) {
    delete state_;
  }

  state_ = state;
  state_->setSynth(this);
  state_->begin();
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