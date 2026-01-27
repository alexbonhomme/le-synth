#include "Hardware.h"

namespace Autosave {

Hardware::Hardware() {}

void Hardware::begin() {
  // Initialize switches
  switches[0].begin(defaults::sw_1_1_pin, defaults::sw_1_2_pin);
  switches[1].begin(defaults::sw_2_1_pin, defaults::sw_2_2_pin);
  switches[2].begin(defaults::sw_3_1_pin, defaults::sw_3_2_pin);
  switches[3].begin(defaults::sw_4_pin);

  // Initialize pots
  pots[0].begin(defaults::pot_1_pin, true);
  pots[1].begin(defaults::pot_2_pin, true);
  pots[2].begin(defaults::pot_3_pin, true);
  pots[3].begin(defaults::pot_attack_pin, true);
  pots[4].begin(defaults::pot_release_pin, true);
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
  if (cv_2.hasChanged()) {
    Serial.println("CV 2:");
    Serial.println(cv_2.getValue());
  }
#endif
}

bool Hardware::changed(hardware_type type, int index) {
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

unsigned char Hardware::read(hardware_type type, int index) {
  if (type == SWITCH) {
    return switches[index].read();
  }

  if (type == POT) {
    return pots[index].getValue();
  }

  if (type == CV) {
    return cv.getValue();
  }

  return false;
}

} // namespace Autosave