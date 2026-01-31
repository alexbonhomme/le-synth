#include <cstring>
#include <EEPROM.h>

#include "Midi.h"
#include "usb_midi.h"

namespace Autosave {

// Create and bind the MIDI interface to the default hardware Serial port
MIDI_CREATE_INSTANCE(HardwareSerial, Serial1, MIDI);

Midi *Midi::instance_ = nullptr;

Midi::Midi() { instance_ = this; }

void Midi::setHandleNoteOn(void (*callback)(byte channel, byte note,
                                            byte velocity)) {
  usbMIDI.setHandleNoteOn(callback);
  MIDI.setHandleNoteOn(callback);
}

void Midi::setHandleNoteOff(void (*callback)(byte channel, byte note,
                                             byte velocity)) {
  usbMIDI.setHandleNoteOff(callback);
  MIDI.setHandleNoteOff(callback);
}

void Midi::setHandleSystemExclusive(void (*callback)(byte *array,
                                                     unsigned size)) {
  usbMIDI.setHandleSystemExclusive(callback);
  MIDI.setHandleSystemExclusive(callback);
}

void Midi::handleSysEx(byte *array, unsigned size) {
  if (instance_ == nullptr) {
    return;
  }

  instance_->handleSysExInternal(array, size);
}

void Midi::handleSysExInternal(byte *array, unsigned size) {
  if (array == nullptr) {
    return;
  }

  // Get channel request: F0 7D 00 03 F7
  if (size == midi_config::SYSEX_GET_CHANNEL_SIZE &&
      memcmp(array, midi_config::SYSEX_GET_CHANNEL_COMMAND, midi_config::SYSEX_GET_CHANNEL_SIZE) == 0) {
    byte ch = getChannel();
    if (ch < 1 || ch > 16)
      ch = 1;
    byte nn = static_cast<byte>(ch - 1);
    const byte reply[] = {0xF0, 0x7D, 0x00, 0x02, nn, 0xF7};
    sendSysEx(reply, sizeof(reply));
    return;
  }

  byte nn;
  if (size == midi_config::SYSEX_SET_CHANNEL_SIZE) {
    // Payload only: 7D 00 01 nn
    if (memcmp(array, midi_config::SYSEX_SET_CHANNEL_COMMAND, sizeof(midi_config::SYSEX_SET_CHANNEL_COMMAND)) != 0) {
      return;
    }
    nn = array[3];
  } else if (size == midi_config::SYSEX_SET_CHANNEL_SIZE_FULL) {
    // Full SysEx: F0 7D 00 01 nn F7 (e.g. from Web MIDI)
    if (array[0] != 0xF0 || array[5] != 0xF7) {
      return;
    }
    if (memcmp(array + 1, midi_config::SYSEX_SET_CHANNEL_COMMAND, sizeof(midi_config::SYSEX_SET_CHANNEL_COMMAND)) != 0) {
      return;
    }
    nn = array[4];
  } else {
    return;
  }

  if (nn > 15) {
    return;
  }

  byte newChannel = nn + 1;
  setChannel(newChannel);

#ifdef DEBUG
  Serial.println("MIDI channel set to " + String(newChannel));
#endif
}

byte Midi::loadChannelFromEeprom() {
  if (EEPROM.read(midi_config::EEPROM_ADDR_CHANNEL_MAGIC) != midi_config::EEPROM_CHANNEL_MAGIC) {
    return midi_config::default_channel;
  }
  byte ch = EEPROM.read(midi_config::EEPROM_ADDR_CHANNEL);
  if (ch < 1 || ch > 16) {
    return midi_config::default_channel;
  }
#ifdef DEBUG
  Serial.println("Loaded MIDI channel from EEPROM: " + String(ch));
#endif
  return ch;
}

void Midi::saveChannelToEeprom(byte channel) {
  if (channel < 1 || channel > 16) {
    return;
  }
  EEPROM.write(midi_config::EEPROM_ADDR_CHANNEL_MAGIC, midi_config::EEPROM_CHANNEL_MAGIC);
  EEPROM.write(midi_config::EEPROM_ADDR_CHANNEL, channel);
}

void Midi::begin() {
  channel_ = loadChannelFromEeprom();
#ifdef DEBUG
  Serial.println("Initializing MIDI to channel " + String(channel_));
#endif
  MIDI.begin(channel_);
}

void Midi::setChannel(byte channel) {
  if (channel < 1 || channel > 16) {
    return;
  }
  channel_ = channel;
  MIDI.begin(channel_);
  saveChannelToEeprom(channel);
}

void Midi::sendSysEx(const byte *data, unsigned size) {
  if (data == nullptr || size == 0)
    return;
  usbMIDI.sendSysEx(size, data, true);
  MIDI.sendSysEx(size, data, true);
}

void Midi::read() {
  usbMIDI.read();
  MIDI.read();
}

} // namespace Autosave
