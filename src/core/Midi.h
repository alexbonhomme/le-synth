#ifndef AUTOSAVE_MIDI_H
#define AUTOSAVE_MIDI_H

#include <Arduino.h>
#include <MIDI.h>

namespace Autosave {

namespace midi_config {
// Default MIDI channel when EEPROM is uninitialized
static constexpr byte default_channel = 1;
// EEPROM layout for persisted MIDI channel
static constexpr int EEPROM_CHANNEL_MAGIC = 0xA5;
static constexpr int EEPROM_ADDR_CHANNEL_MAGIC = 0;
static constexpr int EEPROM_ADDR_CHANNEL = 1;
// SysEx: "set MIDI channel" F0 7D 00 01 nn F7 (nn = 0..15 → channel 1..16)
// SysEx: "get channel" request F0 7D 00 03 F7 (5 bytes); reply F0 7D 00 02 nn
// F7 Callback may receive: 7D 00 01 nn (size 4) or full F0 7D 00 01 nn F7 (size
// 6)
static constexpr byte SYSEX_SET_CHANNEL_COMMAND[] = {0x7D, 0x00, 0x01};
static constexpr unsigned SYSEX_SET_CHANNEL_SIZE = 4;
static constexpr unsigned SYSEX_SET_CHANNEL_SIZE_FULL = 6;
static constexpr unsigned SYSEX_GET_CHANNEL_SIZE = 5;
static constexpr byte SYSEX_GET_CHANNEL_COMMAND[] = {0xF0, 0x7D, 0x00, 0x03,
                                                     0xF7};
} // namespace midi_config

class Midi {
public:
  Midi();

  void setHandleNoteOn(void (*callback)(byte channel, byte note,
                                        byte velocity));
  void setHandleNoteOff(void (*callback)(byte channel, byte note,
                                         byte velocity));
  void setHandleClock(void (*callback)(void));
  void setHandleStart(void (*callback)(void));
  void setHandleContinue(void (*callback)(void));
  void setHandleStop(void (*callback)(void));

  void begin();
  void setChannel(byte channel);
  byte getChannel() const { return channel_; }
  void read();

private:
  static Midi *instance_;
  byte channel_ = 1;

  byte loadChannelFromEeprom();
  void saveChannelToEeprom(byte channel);

  /** Static SysEx handler to register with the MIDI library. */
  static void handleSysEx(byte *array, unsigned size);
  void sendSysEx(const byte *data, unsigned size);
};

} // namespace Autosave

#endif