#include <Arduino.h>
#include <Bounce2.h>
#include <MIDI.h>
#include <ResponsiveAnalogRead.h>

#include "core/Synth.h"

// #define DEBUG

Autosave::Synth synth;

void setup() {
  synth.begin();
}

void loop() { synth.process(); }