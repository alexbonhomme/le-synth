#include <usb_midi.h>

#include "Midi.h"
#include "core/EepromStorage.h"
#include "lib/Logger.h"

#include <cstring>

namespace Autosave {

// Create and bind the MIDI interface to the default hardware Serial port
MIDI_CREATE_INSTANCE(HardwareSerial, Serial1, MIDI);

Midi *Midi::instance_ = nullptr;

Midi::Midi() {
  instance_ = this;

  usbMIDI.setHandleSystemExclusive(&Midi::handleSysEx);
  MIDI.setHandleSystemExclusive(&Midi::handleSysEx);
}

void Midi::begin() {
  channel_ = EepromStorage::loadMidiChannel();

  AutosaveLib::Logger::info("Initializing MIDI module (channel: " + String(channel_) + ")");

  MIDI.begin(channel_);
}

void Midi::setHandleNoteOn(void (*callback)(uint8_t channel, uint8_t note,
                                            uint8_t velocity)) {
  usbMIDI.setHandleNoteOn(callback);
  MIDI.setHandleNoteOn(callback);
}

void Midi::setHandleNoteOff(void (*callback)(uint8_t channel, uint8_t note,
                                             uint8_t velocity)) {
  usbMIDI.setHandleNoteOff(callback);
  MIDI.setHandleNoteOff(callback);
}

void Midi::setHandleClock(void (*callback)(void)) {
  usbMIDI.setHandleClock(callback);
  MIDI.setHandleClock(callback);
}

void Midi::setHandleStart(void (*callback)(void)) {
  usbMIDI.setHandleStart(callback);
  MIDI.setHandleStart(callback);
}

void Midi::setHandleContinue(void (*callback)(void)) {
  usbMIDI.setHandleContinue(callback);
  MIDI.setHandleContinue(callback);
}

void Midi::setHandleStop(void (*callback)(void)) {
  usbMIDI.setHandleStop(callback);
  MIDI.setHandleStop(callback);
}

void Midi::handleSysEx(uint8_t *array, unsigned size) {
  if (instance_ == nullptr || array == nullptr) {
    return;
  }

  // Get channel request: F0 7D 00 03 F7
  if (size == midi_config::SYSEX_GET_CHANNEL_SIZE &&
      memcmp(array, midi_config::SYSEX_GET_CHANNEL_COMMAND,
             midi_config::SYSEX_GET_CHANNEL_SIZE) == 0) {
    uint8_t ch = instance_->getChannel();
    if (ch < 1 || ch > 16)
      ch = 1;
    uint8_t nn = static_cast<uint8_t>(ch - 1);
    const uint8_t reply[] = {0xF0, 0x7D, 0x00, 0x02, nn, 0xF7};
    instance_->sendSysEx(reply, sizeof(reply));
    return;
  }

  // Get arp steps: F0 7D 00 04 F7
  if (size == midi_config::SYSEX_ARP_GET_SIZE && array[0] == 0xF0 &&
      array[1] == 0x7D && array[2] == 0x00 && array[3] == midi_config::SYSEX_ARP_GET_CMD &&
      array[4] == 0xF7 && instance_->arp_steps_getter_ != nullptr) {
    uint8_t reply[midi_config::SYSEX_ARP_REPLY_SIZE];
    reply[0] = 0xF0;
    reply[1] = 0x7D;
    reply[2] = 0x00;
    reply[3] = midi_config::SYSEX_ARP_REPLY_CMD;
    int off = 4;
    for (uint8_t mode = 0; mode < 3; mode++) {
      uint8_t len = 0;
      uint8_t data[midi_config::SYSEX_ARP_MAX_STEPS] = {0};
      instance_->arp_steps_getter_(mode, &len, data);
      if (len > midi_config::SYSEX_ARP_MAX_STEPS) {
        len = midi_config::SYSEX_ARP_MAX_STEPS;
      }
      reply[off++] = len;
      for (uint8_t i = 0; i < midi_config::SYSEX_ARP_MAX_STEPS; i++) {
        reply[off++] = i < len ? data[i] : 0;
      }
    }
    reply[off] = 0xF7;
    instance_->sendSysEx(reply, sizeof(reply));
    return;
  }

  // Set arp steps: F0 7D 00 06 [mode][len][0..8 bytes] F7
  if (size >= 8 && size <= 16 && array[0] == 0xF0 && array[1] == 0x7D &&
      array[2] == 0x00 && array[3] == midi_config::SYSEX_ARP_SET_CMD &&
      instance_->arp_steps_setter_ != nullptr) {
    uint8_t mode = array[4];
    uint8_t len = array[5];
    if (mode > 2 || len > midi_config::SYSEX_ARP_MAX_STEPS ||
        size != (unsigned)(7 + len)) {
      return;
    }
    if (array[6 + len] != 0xF7) {
      return;
    }
    instance_->arp_steps_setter_(mode, len, array + 6);
    return;
  }

  uint8_t nn;
  if (size == midi_config::SYSEX_SET_CHANNEL_SIZE) {
    // Payload only: 7D 00 01 nn
    if (memcmp(array, midi_config::SYSEX_SET_CHANNEL_COMMAND,
               sizeof(midi_config::SYSEX_SET_CHANNEL_COMMAND)) != 0) {
      return;
    }

    nn = array[3];
  } else if (size == midi_config::SYSEX_SET_CHANNEL_SIZE_FULL) {
    // Full SysEx: F0 7D 00 01 nn F7 (e.g. from Web MIDI)
    if (array[0] != 0xF0 || array[5] != 0xF7) {
      return;
    }
    if (memcmp(array + 1, midi_config::SYSEX_SET_CHANNEL_COMMAND,
               sizeof(midi_config::SYSEX_SET_CHANNEL_COMMAND)) != 0) {
      return;
    }
    nn = array[4];
  } else {
    return;
  }

  if (nn > 15) {
    return;
  }

  uint8_t newChannel = nn + 1;
  instance_->setChannel(newChannel);
  return;
}

void Midi::sendSysEx(const uint8_t *data, unsigned size) {
  if (data == nullptr || size == 0) {
    return;
  }

  usbMIDI.sendSysEx(size, data, true);
  MIDI.sendSysEx(size, data, true);
}

void Midi::setChannel(uint8_t channel) {
  if (channel < 1 || channel > 16) {
    return;
  }

  channel_ = channel;
  EepromStorage::saveMidiChannel(static_cast<uint8_t>(channel));
  MIDI.begin(channel);

  AutosaveLib::Logger::debug("MIDI channel set to " + String(channel));
}

void Midi::setArpStepsSysexHandlers(ArpStepsGetter getter,
                                    ArpStepsSetter setter) {
  arp_steps_getter_ = getter;
  arp_steps_setter_ = setter;
}

void Midi::read() {
  usbMIDI.read(channel_);
  MIDI.read(channel_);
}

} // namespace Autosave
