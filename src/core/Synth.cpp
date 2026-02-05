#include "Synth.h"

#include "EepromStorage.h"
#include "lib/Logger.h"
#include "states/ArpSynthState.h"
#include "states/MonoSynthState.h"
#include "states/PolySynthState.h"

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
  AutosaveLib::Logger::begin(AutosaveLib::Logger::LEVEL_DEBUG);
  AutosaveLib::Logger::info("Initializing Synth module");

  hardware->begin();
  audio->begin();
  midi->begin();

  EepromStorage::loadArpModeSteps(ArpSynthState::arp_mode_steps);
  ArpSynthState::registerArpStepsWithMidi(midi);
  state_->begin();

  // Load the initial mode from the hardware
  updateMode();
}

void Synth::process() {
  midi->read();
  hardware->update();

  // Handle mode switch
  if (hardware->changed(hardware::CTRL_SWITCH_MODE)) {
    updateMode();
  }

  state_->process();

  // #ifdef DEBUG
  //   debugAudioUsage();
  // #endif
}

void Synth::updateMode() {
  uint8_t mode = (uint8_t)hardware->read(hardware::CTRL_SWITCH_MODE);

  switch (mode) {
  case 0:
    AutosaveLib::Logger::info("Initializing Monophonic synth state");
    changeState(new MonoSynthState());
    return;
  case 1:
    AutosaveLib::Logger::info("Initializing Polyphonic synth state");
    changeState(new PolySynthState());
    return;
  case 2:
    AutosaveLib::Logger::info("Initializing Arp synth state");
    changeState(new ArpSynthState());
    return;
  default:
    AutosaveLib::Logger::error("Unknown mode: " + String(mode));
    return;
  }
}

void Synth::debugAudioUsage() {
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

/***
 * Static callbacks
 ***/

void Synth::changeState(State *state) {
  if (state_ != nullptr) {
    delete state_;
  }

  state_ = state;
  state_->setSynth(this);
  state_->begin();
}

uint8_t fixMidiNote(uint8_t note) {
  // MIDI libray seems to add one octave to the note number for no reason
  note = note - 12;

  return note < 0 ? 0 : note;
}

void Synth::midiNoteOn(uint8_t channel, uint8_t note, uint8_t velocity) {
  if (instance_ == nullptr || instance_->state_ == nullptr) {
    return;
  }

  instance_->state_->noteOn({fixMidiNote(note), velocity});
}

void Synth::midiNoteOff(uint8_t channel, uint8_t note, uint8_t velocity) {
  if (instance_ == nullptr || instance_->state_ == nullptr) {
    return;
  }

  instance_->state_->noteOff({fixMidiNote(note), velocity});
}

} // namespace Autosave