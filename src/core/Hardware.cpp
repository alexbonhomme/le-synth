#include "Hardware.h"

namespace Autosave {

Hardware::Hardware() {}

void Hardware::begin() {
  // Initialize switches
  switches[controls::sw_mode].begin(defaults::sw_1_1_pin, defaults::sw_1_2_pin);
  switches[controls::sw_1].begin(defaults::sw_2_1_pin, defaults::sw_2_2_pin);
  switches[controls::sw_2].begin(defaults::sw_3_1_pin, defaults::sw_3_2_pin);
  switches[controls::sw_3].begin(defaults::sw_4_pin);

  // Initialize pots
  pots[controls::pot_1].begin(defaults::pot_1_pin, true, 0.001f);
  pots[controls::pot_2].begin(defaults::pot_2_pin, true, 0.001f);
  pots[controls::pot_3].begin(defaults::pot_3_pin, true, 0.001f);
  pots[controls::pot_attack].begin(defaults::pot_attack_pin, true, 0.001f);
  pots[controls::pot_release].begin(defaults::pot_release_pin, true, 0.001f);
}

void Hardware::update() {
  for (unsigned int i = 0; i < sizeof(pots) / sizeof(pots[0]); i++) {
    pots[i].update();
  }

  for (unsigned int i = 0; i < sizeof(switches) / sizeof(switches[0]); i++) {
    switches[i].update();
  }

  cv.update();

#ifdef DEBUG
  if (cv.hasChanged()) {
    Serial.println("CV:");
    Serial.println(cv.getValue());
  }
#endif
}

bool Hardware::changed(hardware_type type, unsigned char index) {
  if (type == SWITCH) {
    return switches[index].changed();
  }

  if (type == POT) {
    return pots[index].hasChanged();
  }

  if (type == CV) {
    return cv.hasChanged();
  }

  return false;
}

float Hardware::read(hardware_type type, unsigned char index) {
  if (type == SWITCH) {
    return (float)switches[index].read();
  }

  if (type == POT) {
    return (float)pots[index].getValue() / 1023.0f;
  }

  if (type == CV) {
    return (float)cv.getValue() / 1023.0f;
  }

  return 0.0f;
}

} // namespace Autosave