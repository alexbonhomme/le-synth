#include "Hardware.h"

namespace Autosave {

Hardware::Hardware() {}

void Hardware::begin() {
  // Initialize switches
  ((SwitchControl*)controls[hardware_controls::SWITCH_MODE])->begin(defaults::sw_1_1_pin, defaults::sw_1_2_pin);
  ((SwitchControl*)controls[hardware_controls::SWITCH_1])->begin(defaults::sw_2_1_pin, defaults::sw_2_2_pin);
  ((SwitchControl*)controls[hardware_controls::SWITCH_2])->begin(defaults::sw_3_1_pin, defaults::sw_3_2_pin);
  ((SwitchControl*)controls[hardware_controls::SWITCH_3])->begin(defaults::sw_4_pin);

  // Initialize pots
  (controls[hardware_controls::POT_1])->begin(defaults::pot_1_pin);
  (controls[hardware_controls::POT_2])->begin(defaults::pot_2_pin);
  (controls[hardware_controls::POT_3])->begin(defaults::pot_3_pin);
  (controls[hardware_controls::POT_ATTACK])->begin(defaults::pot_attack_pin);
  (controls[hardware_controls::POT_RELEASE])->begin(defaults::pot_release_pin);

  // Initialize CV
  (controls[hardware_controls::CV])->begin(defaults::cv_pin);
}

void Hardware::update() {
  for (byte i = 0; i < sizeof(controls) / sizeof(controls[0]); i++) {
    controls[i]->update();
  }

#ifdef DEBUG
  if ((controls[hardware_controls::CV])->changed()) {
    Serial.println("CV:");
    Serial.println((controls[hardware_controls::CV])->read());
  }
#endif
}

bool Hardware::changed(hardware_controls control) {
  return controls[control]->changed();
}

float Hardware::read(hardware_controls control) {
  return controls[control]->read();
}

} // namespace Autosave