#ifndef AUTOSAVE_EEPROM_STORAGE_H
#define AUTOSAVE_EEPROM_STORAGE_H

#include <array>
#include <vector>
#include <cstdint>

namespace Autosave {

/** EEPROM-backed storage for configuration (MIDI channel, arp steps, etc.). */
class EepromStorage {
public:
  /** Type for the 3 arp mode step sequences (max 8 steps each). */
  using ArpModeSteps = std::array<std::vector<uint8_t>, 3>;

  /** Max steps per arp mode. */
  static constexpr uint8_t kMaxArpSteps = 8;

  /** EEPROM layout for MIDI channel (addresses 0–1). */
  static constexpr uint8_t kMidiChannelMagic = 0xA5;
  static constexpr uint8_t kMidiChannelAddrMagic = 0;
  static constexpr uint8_t kMidiChannelAddr = 1;
  static constexpr uint8_t kMidiChannelDefault = 1;

  /** EEPROM layout for arp mode steps (addresses 2+). */
  static constexpr uint8_t kArpMagic = 0xA6;
  static constexpr uint8_t kArpAddrMagic = 2;
  static constexpr uint8_t kArpAddrData = 3;

  /**
   * Load MIDI channel from EEPROM (1–16).
   * Returns kMidiChannelDefault if magic invalid or value out of range.
   */
  static uint8_t loadMidiChannel();

  /**
   * Save MIDI channel to EEPROM (1–16). No-op if out of range.
   */
  static void saveMidiChannel(uint8_t channel);

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
