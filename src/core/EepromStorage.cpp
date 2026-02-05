#include "EepromStorage.h"
#include "lib/Logger.h"

#include <EEPROM.h>

namespace Autosave {

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

} // namespace Autosave
