#include "EepromStorage.h"
#include "lib/Logger.h"

#include <EEPROM.h>
#include <cstdint>

namespace {
// EEPROM layout for MIDI channel (addresses 0–1).
constexpr uint8_t kMidiChannelMagic = 0xA5;
constexpr uint8_t kMidiChannelAddrMagic = 0;
constexpr uint8_t kMidiChannelAddr = 1;
constexpr uint8_t kMidiChannelDefault = 1;
// EEPROM layout for arp mode steps (addresses 2+).
constexpr uint8_t kArpMagic = 0xA6;
constexpr uint8_t kArpAddrMagic = 2;
constexpr uint8_t kArpAddrData = 3;
// EEPROM layout for custom waveform (bank + index).
constexpr uint8_t kCustomWaveformMagic = 0xA7;
constexpr uint8_t kCustomWaveformAddrMagic = 30;
constexpr uint8_t kCustomWaveformAddrBank = 31;
constexpr uint8_t kCustomWaveformAddrIndex = 32;
} // namespace

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
    if (len > EepromStorage::kMaxArpSteps) {
      len = static_cast<uint8_t>(EepromStorage::kMaxArpSteps);
    }
    out[m].clear();
    out[m].reserve(len);
    for (uint8_t i = 0; i < len; i++) {
      out[m].push_back(EEPROM.read(addr + i));
    }
    addr += EepromStorage::kMaxArpSteps;
  }
  AutosaveLib::Logger::debug("Loaded arp mode steps from EEPROM");
}

void EepromStorage::saveArpModeSteps(const ArpModeSteps &data) {
  EEPROM.write(kArpAddrMagic, kArpMagic);
  int addr = kArpAddrData;
  for (size_t m = 0; m < data.size(); m++) {
    uint8_t len = static_cast<uint8_t>(data[m].size());
    if (len > EepromStorage::kMaxArpSteps) {
      len = static_cast<uint8_t>(EepromStorage::kMaxArpSteps);
    }
    EEPROM.write(addr, len);
    addr++;
    for (uint8_t i = 0; i < EepromStorage::kMaxArpSteps; i++) {
      EEPROM.write(addr + i, i < len ? data[m][i] : 0);
    }
    addr += EepromStorage::kMaxArpSteps;
  }
  AutosaveLib::Logger::debug("Saved arp mode steps to EEPROM");
}

void EepromStorage::loadCustomWaveform(uint8_t &out_bank, uint8_t &out_index) {
  if (EEPROM.read(kCustomWaveformAddrMagic) != kCustomWaveformMagic) {
    out_bank = EepromStorage::kCustomWaveformBankDefault;
    out_index = EepromStorage::kCustomWaveformIndexDefault;
    return;
  }
  out_bank = static_cast<uint8_t>(EEPROM.read(kCustomWaveformAddrBank));
  out_index = static_cast<uint8_t>(EEPROM.read(kCustomWaveformAddrIndex));
  if (out_bank > 2) {
    out_bank = EepromStorage::kCustomWaveformBankDefault;
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
