#ifndef AUTOSAVE_EEPROM_STORAGE_H
#define AUTOSAVE_EEPROM_STORAGE_H

#include <array>
#include <cstddef>
#include <cstdint>
#include <vector>

namespace Autosave {

/** EEPROM-backed storage for configuration (arp steps, etc.). */
class EepromStorage {
public:
  /** Type for the 3 arp mode step sequences (max 8 steps each). */
  using ArpModeSteps = std::array<std::vector<uint8_t>, 3>;

  /** Max steps per arp mode. */
  static constexpr uint8_t kMaxArpSteps = 8;

  /** EEPROM layout for arp mode steps. */
  static constexpr int kArpMagic = 0xA6;
  static constexpr int kArpAddrMagic = 2;
  static constexpr int kArpAddrData = 3;

  /**
   * Load arp mode steps from EEPROM into out.
   * If magic is invalid, out is left unchanged.
   */
  static void loadArpModeSteps(ArpModeSteps &out);

  /**
   * Save arp mode steps to EEPROM.
   */
  static void saveArpModeSteps(const ArpModeSteps &data);
};

} // namespace Autosave

#endif
