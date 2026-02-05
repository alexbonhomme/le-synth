#include <Bounce2.h>
#include <MIDI.h>
#include <ResponsiveAnalogRead.h>

#include "core/Synth.h"

Autosave::Synth synth;

void setup() { synth.begin(); }

void loop() { synth.process(); }