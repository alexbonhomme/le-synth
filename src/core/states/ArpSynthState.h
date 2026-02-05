#ifndef AUTOSAVE_ARP_SYNTH_STATE_H
#define AUTOSAVE_ARP_SYNTH_STATE_H

#include <array>
#include <vector>

#include "MonoSynthState.h"

namespace Autosave {

namespace arp_synth_config {
constexpr uint8_t clock_ticks_per_sixteenth =
    6; // 24 MIDI clocks per quarter → 6 per 1/16

static std::array<std::vector<uint8_t>, 3> arp_mode_steps{
    {std::vector<uint8_t>({0, 1, 2, 3, 4, 5, 6, 7}),
     std::vector<uint8_t>({4, 3, 2, 1, 0}),
     std::vector<uint8_t>({0, 2, 1, 3})}};
} // namespace arp_synth_config

struct Note {
  uint8_t note;
  uint8_t velocity;
};

class ArpSynthState : public MonoSynthState {
private:
  static ArpSynthState *instance_;

  /** Counts MIDI clock ticks; 24 per quarter note, so 6 = 1/16 note. */
  uint8_t clock_tick_count_ = 0;

  std::vector<Note> notes_;
  Note current_note_ = {0, 0};

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

  void begin() override;
  void process() override;
  void noteOn(uint8_t note, uint8_t velocity) override;
  void noteOff(uint8_t note, uint8_t velocity) override;

  /** Static callback for MIDI clock; forwards to active instance. */
  static void onMidiClock();
  static void onMidiStart();
  static void onMidiStop();
};
} // namespace Autosave

#endif