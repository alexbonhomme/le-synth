#include "EepromStorage.h"
#include "lib/Logger.h"

#include <EEPROM.h>
#include <cstdint>

namespace Autosave {

uint8_t EepromStorage::loadMidiChannel() {
  if (EEPROM.read(kMidiChannelAddrMagic) != kMidiChannelMagic) {
    return kMidiChannelDefault;
  }
  uint8_t ch = static_cast<uint8_t>(EEPROM.read(kMidiChannelAddr));
  if (ch < 1 || ch > 16) {
    return kMidiChannelDefault;
  }
  AutosaveLib::Logger::debug("Loaded MIDI channel from EEPROM: " + String(ch));
  return ch;
}

void EepromStorage::saveMidiChannel(uint8_t channel) {
  if (channel < 1 || channel > 16) {
    return;
  }
  EEPROM.write(kMidiChannelAddrMagic, kMidiChannelMagic);
  EEPROM.write(kMidiChannelAddr, channel);
  AutosaveLib::Logger::debug("Saved MIDI channel to EEPROM");
}

void EepromStorage::loadArpModeSteps(ArpModeSteps &out) {
  if (EEPROM.read(kArpAddrMagic) != kArpMagic) {
    return;
  }
  int addr = kArpAddrData;
  for (size_t m = 0; m < out.size(); m++) {
    uint8_t len = EEPROM.read(addr);
    addr++;
    if (len > kMaxArpSteps) {
      len = static_cast<uint8_t>(kMaxArpSteps);
    }
    out[m].clear();
    out[m].reserve(len);
    for (uint8_t i = 0; i < len; i++) {
      out[m].push_back(EEPROM.read(addr + i));
    }
    addr += kMaxArpSteps;
  }
  AutosaveLib::Logger::debug("Loaded arp mode steps from EEPROM");
}

void EepromStorage::saveArpModeSteps(const ArpModeSteps &data) {
  EEPROM.write(kArpAddrMagic, kArpMagic);
  int addr = kArpAddrData;
  for (size_t m = 0; m < data.size(); m++) {
    uint8_t len = static_cast<uint8_t>(data[m].size());
    if (len > kMaxArpSteps) {
      len = static_cast<uint8_t>(kMaxArpSteps);
    }
    EEPROM.write(addr, len);
    addr++;
    for (uint8_t i = 0; i < kMaxArpSteps; i++) {
      EEPROM.write(addr + i, i < len ? data[m][i] : 0);
    }
    addr += kMaxArpSteps;
  }
  AutosaveLib::Logger::debug("Saved arp mode steps to EEPROM");
}

void EepromStorage::loadCustomWaveform(uint8_t &out_bank, uint8_t &out_index) {
  if (EEPROM.read(kCustomWaveformAddrMagic) != kCustomWaveformMagic) {
    out_bank = kCustomWaveformBankDefault;
    out_index = kCustomWaveformIndexDefault;
    return;
  }
  out_bank = static_cast<uint8_t>(EEPROM.read(kCustomWaveformAddrBank));
  out_index = static_cast<uint8_t>(EEPROM.read(kCustomWaveformAddrIndex));
  if (out_bank > 2) {
    out_bank = kCustomWaveformBankDefault;
  }
  AutosaveLib::Logger::debug("Loaded custom waveform from EEPROM: bank " +
                            String(out_bank) + " index " + String(out_index));
}

void EepromStorage::saveCustomWaveform(uint8_t bank, uint8_t index) {
  if (bank > 2) {
    return;
  }
  EEPROM.write(kCustomWaveformAddrMagic, kCustomWaveformMagic);
  EEPROM.write(kCustomWaveformAddrBank, bank);
  EEPROM.write(kCustomWaveformAddrIndex, index);
  AutosaveLib::Logger::debug("Saved custom waveform to EEPROM");
}

} // namespace Autosave
