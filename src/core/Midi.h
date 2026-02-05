#ifndef AUTOSAVE_MIDI_H
#define AUTOSAVE_MIDI_H

#include <MIDI.h>

namespace Autosave {

namespace midi_config {
// SysEx: "set MIDI channel" F0 7D 00 01 nn F7 (nn = 0..15 → channel 1..16)
// SysEx: "get channel" request F0 7D 00 03 F7 (5 bytes); reply F0 7D 00 02 nn
// F7 Callback may receive: 7D 00 01 nn (size 4) or full F0 7D 00 01 nn F7 (size
// 6)
static constexpr uint8_t SYSEX_SET_CHANNEL_COMMAND[] = {0x7D, 0x00, 0x01};
static constexpr unsigned SYSEX_SET_CHANNEL_SIZE = 4;
static constexpr unsigned SYSEX_SET_CHANNEL_SIZE_FULL = 6;
static constexpr unsigned SYSEX_GET_CHANNEL_SIZE = 5;
static constexpr uint8_t SYSEX_GET_CHANNEL_COMMAND[] = {0xF0, 0x7D, 0x00, 0x03,
                                                        0xF7};
// SysEx arp steps: get F0 7D 00 04 F7; reply F0 7D 00 05
// [len0][8][len1][8][len2][8] F7; set F0 7D 00 06 [mode][len][0..8 bytes] F7
static constexpr uint8_t SYSEX_ARP_GET_CMD = 0x04;
static constexpr uint8_t SYSEX_ARP_REPLY_CMD = 0x05;
static constexpr uint8_t SYSEX_ARP_SET_CMD = 0x06;
static constexpr unsigned SYSEX_ARP_GET_SIZE = 5;
static constexpr unsigned SYSEX_ARP_REPLY_SIZE = 4 + 3 * (1 + 8) + 1; // 32
static constexpr unsigned SYSEX_ARP_MAX_STEPS = 8;
} // namespace midi_config

struct MidiNote {
  uint8_t number;
  uint8_t velocity;
};

class Midi {
public:
  Midi();

  void setHandleNoteOn(void (*callback)(uint8_t channel, uint8_t note,
                                        uint8_t velocity));
  void setHandleNoteOff(void (*callback)(uint8_t channel, uint8_t note,
                                         uint8_t velocity));
  void setHandleClock(void (*callback)(void));
  void setHandleStart(void (*callback)(void));
  void setHandleContinue(void (*callback)(void));
  void setHandleStop(void (*callback)(void));

  void begin();
  void setChannel(uint8_t channel);
  uint8_t getChannel() const { return channel_; }
  void read();

  /** Callbacks for arp steps SysEx get/set; set from ArpSynthState. */
  using ArpStepsGetter = void (*)(uint8_t mode, uint8_t *len, uint8_t *data);
  using ArpStepsSetter = void (*)(uint8_t mode, uint8_t len,
                                  const uint8_t *data);
  void setArpStepsSysexHandlers(ArpStepsGetter getter, ArpStepsSetter setter);

private:
  static Midi *instance_;
  uint8_t channel_ = 1;
  ArpStepsGetter arp_steps_getter_ = nullptr;
  ArpStepsSetter arp_steps_setter_ = nullptr;

  /** Static SysEx handler to register with the MIDI library. */
  static void handleSysEx(uint8_t *array, unsigned size);
  void sendSysEx(const uint8_t *data, unsigned size);
};

} // namespace Autosave

#endif