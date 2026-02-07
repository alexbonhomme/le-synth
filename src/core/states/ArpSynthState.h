#ifndef AUTOSAVE_ARP_SYNTH_STATE_H
#define AUTOSAVE_ARP_SYNTH_STATE_H

#include "MonoSynthState.h"

#include <array>
#include <vector>

namespace Autosave {

class ArpSynthState : public MonoSynthState {
private:
  static ArpSynthState *instance_;

  uint8_t clock_tick_count_ = 0;

  std::vector<MidiNote> notes_;
  MidiNote current_note_ = {0, 0};

  uint8_t arp_mod_ = 0;
  uint8_t arp_mode_index_ = 0;

  bool is_running_ = true;

  void internalNodeOn_();
  void internalNodeOff_();

  void onClockTick();
  void onStart();
  void onStop();

public:
  ~ArpSynthState() override;

  /** Runtime arp step sequences (loaded/saved by Synth via EEPROM and Sysex). */
  static std::array<std::vector<uint8_t>, 3> arp_mode_steps;

  void begin() override;
  void process() override;
  void noteOn(MidiNote note) override;
  void noteOff(MidiNote note) override;

  /** Static callback for MIDI clock; forwards to active instance. */
  static void onMidiClock();
  static void onMidiStart();
  static void onMidiStop();
};

} // namespace Autosave

#endif