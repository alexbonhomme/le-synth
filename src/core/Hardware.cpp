#include "Hardware.h"

namespace Autosave {

Hardware::Hardware() {}

void Hardware::begin() {
  #ifdef DEBUG
  Serial.println("Initializing hardware");
  #endif

  // Initialize switches
  controls[hardware::controls::CTRL_SWITCH_MODE] = new SwitchControl();
  ((SwitchControl*)controls[hardware::controls::CTRL_SWITCH_MODE])->begin(hardware::pins::PIN_SW_1_1, hardware::pins::PIN_SW_1_3);
  controls[hardware::controls::CTRL_SWITCH_1] = new SwitchControl();
  ((SwitchControl*)controls[hardware::controls::CTRL_SWITCH_1])->begin(hardware::pins::PIN_SW_2_1, hardware::pins::PIN_SW_2_3);
  controls[hardware::controls::CTRL_SWITCH_2] = new SwitchControl();
  ((SwitchControl*)controls[hardware::controls::CTRL_SWITCH_2])->begin(hardware::pins::PIN_SW_3_1, hardware::pins::PIN_SW_3_3);
  controls[hardware::controls::CTRL_SWITCH_3] = new SwitchControl();
  controls[hardware::controls::CTRL_SWITCH_3]->begin(hardware::pins::PIN_SW_4_1);

  // Initialize pots
  controls[hardware::controls::CTRL_POT_1] = new AnalogControl();
  controls[hardware::controls::CTRL_POT_1]->begin(hardware::pins::PIN_POT_1);
  controls[hardware::controls::CTRL_POT_2] = new AnalogControl();
  controls[hardware::controls::CTRL_POT_2]->begin(hardware::pins::PIN_POT_2);
  controls[hardware::controls::CTRL_POT_3] = new AnalogControl();
  controls[hardware::controls::CTRL_POT_3]->begin(hardware::pins::PIN_POT_3);
  controls[hardware::controls::CTRL_POT_ATTACK] = new AnalogControl();
  controls[hardware::controls::CTRL_POT_ATTACK]->begin(hardware::pins::PIN_POT_ATTACK);
  controls[hardware::controls::CTRL_POT_RELEASE] = new AnalogControl();
  controls[hardware::controls::CTRL_POT_RELEASE]->begin(hardware::pins::PIN_POT_RELEASE);

  // Initialize CV
  controls[hardware::controls::CTRL_CV] = new AnalogControl();
  controls[hardware::controls::CTRL_CV]->begin(hardware::pins::PIN_CV);
}

void Hardware::update() {
  for (byte i = 0; i < sizeof(controls) / sizeof(controls[0]); i++) {
    controls[i]->update();
  }

  // @TODO: Implement CV input
// #ifdef DEBUG
//   if ((controls[hardware::controls::CTRL_CV])->changed()) {
//     Serial.println("CV:");
//     Serial.println((controls[hardware::controls::CTRL_CV])->read());
//   }
// #endif
}

bool Hardware::changed(hardware::controls control) {
  return controls[control]->changed();
}

float Hardware::read(hardware::controls control) {
  return controls[control]->read();
}

} // namespace Autosave