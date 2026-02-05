#include "ArpSynthState.h"

#include "core/EepromStorage.h"
#include "core/Hardware.h"
#include "core/Synth.h"
#include "lib/Logger.h"

#include <array>
#include <vector>

namespace Autosave {

ArpSynthState *ArpSynthState::instance_ = nullptr;

std::array<std::vector<uint8_t>, 3> ArpSynthState::arp_mode_steps = {};

ArpSynthState::~ArpSynthState() {
  if (synth_ != nullptr && synth_->midi != nullptr) {
    synth_->midi->setHandleClock(nullptr);
    synth_->midi->setHandleStart(nullptr);
    synth_->midi->setHandleContinue(nullptr);
    synth_->midi->setHandleStop(nullptr);
  }

  instance_ = nullptr;
}

void ArpSynthState::begin() {
  AutosaveLib::Logger::debug("ArpSynthState::begin");

  MonoSynthState::begin();

  clock_tick_count_ = 0;
  instance_ = this;
  synth_->midi->setHandleClock(&ArpSynthState::onMidiClock);
  synth_->midi->setHandleStart(&ArpSynthState::onMidiStart);
  synth_->midi->setHandleContinue(&ArpSynthState::onMidiStart);
  synth_->midi->setHandleStop(&ArpSynthState::onMidiStop);
}

void ArpSynthState::onMidiClock() {
  if (instance_ != nullptr) {
    instance_->onClockTick();
  }
}

void ArpSynthState::onMidiStart() {
  if (instance_ != nullptr) {
    instance_->onStart();
  }
}

void ArpSynthState::onMidiStop() {
  if (instance_ != nullptr) {
    instance_->onStop();
  }
}

void ArpSynthState::onClockTick() {
  clock_tick_count_++;

  if (!is_running_) {
    return;
  }

  if (clock_tick_count_ == arp_synth_config::clock_ticks_per_sixteenth - 2) {
    if (current_note_.number == 0) {
      return;
    }

    internalNodeOff_();
  }

  if (clock_tick_count_ >= arp_synth_config::clock_ticks_per_sixteenth) {
    clock_tick_count_ = 0;

    if (notes_.empty()) {
      return;
    }

    internalNodeOn_();
  }
}

void ArpSynthState::onStart() { is_running_ = true; }

void ArpSynthState::onStop() {
  is_running_ = false;
  current_note_ = {0, 0};
  arp_mode_index_ = 0;
  notes_.clear();

  MonoSynthState::noteOff({0, 0});
}

void ArpSynthState::process() {
  MonoSynthState::process();

  if (synth_->hardware->changed(hardware::CTRL_SWITCH_2)) {
    arp_mod_ = (uint8_t)synth_->hardware->read(hardware::CTRL_SWITCH_2);
  }
}

void ArpSynthState::internalNodeOn_() {
  const std::vector<uint8_t> &arp_mode_sequence = ArpSynthState::arp_mode_steps[arp_mod_];

  if (arp_mode_index_ >= arp_mode_sequence.size()) {
    arp_mode_index_ = 0;
  }

  uint8_t mode_index = arp_mode_sequence[arp_mode_index_] % notes_.size();
  current_note_ = notes_[mode_index];
  arp_mode_index_ = (arp_mode_index_ + 1) % arp_mode_sequence.size();

  // @TODO: Handle velocity
  MonoSynthState::noteOn(current_note_);
}

void ArpSynthState::internalNodeOff_() {
  MonoSynthState::noteOff({0, 0});

  current_note_ = {0, 0};
}

void ArpSynthState::noteOn(MidiNote note) { notes_.push_back(note); }

void ArpSynthState::noteOff(MidiNote note) {
  if (notes_.empty()) {
    return;
  }

  for (uint8_t i = 0; i < notes_.size(); i++) {
    if (notes_[i].number == note.number) {
      notes_.erase(notes_.begin() + i);

      if (arp_mode_index_ > 0 && arp_mode_index_ >= i) {
        arp_mode_index_--;
      }

      break;
    }
  }
}

static void arpStepsGetter(uint8_t mode, uint8_t *len, uint8_t *data) {
  if (mode >= 3 || len == nullptr || data == nullptr) {
    return;
  }
  const auto &vec = ArpSynthState::arp_mode_steps[mode];
  *len = static_cast<uint8_t>(vec.size() > EepromStorage::kMaxArpSteps
                                  ? EepromStorage::kMaxArpSteps
                                  : vec.size());
  for (uint8_t i = 0; i < *len; i++) {
    data[i] = vec[i];
  }
}

static void arpStepsSetter(uint8_t mode, uint8_t len, const uint8_t *data) {
  if (mode >= 3 || data == nullptr || len > EepromStorage::kMaxArpSteps) {
    return;
  }

  auto &vec = ArpSynthState::arp_mode_steps[mode];
  vec.clear();
  vec.reserve(len);
  for (uint8_t i = 0; i < len; i++) {
    vec.push_back(data[i]);
  }

  EepromStorage::saveArpModeSteps(ArpSynthState::arp_mode_steps);
}

void ArpSynthState::registerArpStepsWithMidi(Midi *midi) {
  if (midi != nullptr) {
    midi->setArpStepsSysexHandlers(arpStepsGetter, arpStepsSetter);
  }
}
} // namespace Autosave