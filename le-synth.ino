#include <Audio.h>
#include <MIDI.h>
#include <Bounce.h>
#include <ResponsiveAnalogRead.h>

// #define DEBUG

#define MIDI_CHANNEL 8

#define BT_1 2
#define BT_2 3
#define OCTAVE_SWITCH_1 4
#define OCTAVE_SWITCH_2 5
#define ENVELOPE_SWITCH 6

#define MAIN_MIX_GAIN 0.1
#define SUB_MIX_GAIN 0.45
#define FILTER_ENV_GAIN 0.5

// Create and bind the MIDI interface to the default hardware Serial port
MIDI_CREATE_INSTANCE(HardwareSerial, Serial1, MIDI);

// GUItool: begin automatically generated code
AudioSynthWaveform waveform1;     // xy=687,580
AudioSynthWaveform waveform2;     // xy=688,627
AudioSynthWaveform waveform_sub;  // xy=686,673
AudioFilterStateVariable filter1; // xy=692,703
AudioMixer4 mixer1;               // xy=897,623
AudioEffectEnvelope envelope1;    // xy=1087,622
AudioEffectEnvelope envelope2;    // xy=1087,680
AudioSynthWaveformDc dc_signal;   // xy=687,680
AudioOutputI2S i2s1;              // xy=1271,629
AudioConnection patchCord1(waveform1, 0, mixer1, 0);
AudioConnection patchCord2(waveform2, 0, mixer1, 1);
AudioConnection patchCord3(waveform_sub, 0, filter1, 0);
AudioConnection patchCord4(filter1, 0, mixer1, 2);
AudioConnection patchCord5(mixer1, envelope1);
AudioConnection patchCord6(envelope1, 0, i2s1, 0);
AudioConnection patchCord7(dc_signal, 0, envelope2, 0);
AudioConnection patchCord8(envelope2, 0, i2s1, 1);
// GUItool: end automatically generated code

Bounce button_1 = Bounce(BT_1, 15);
Bounce button_2 = Bounce(BT_2, 15);
Bounce octave_switch_1 = Bounce(OCTAVE_SWITCH_1, 15);
Bounce octave_switch_2 = Bounce(OCTAVE_SWITCH_2, 15);
Bounce envelope_switch = Bounce(ENVELOPE_SWITCH, 15);

ResponsiveAnalogRead pot_freq_2(A0, true);
ResponsiveAnalogRead pot_amplitude_2(A1, true);
ResponsiveAnalogRead pot_amplitude_sub(A2, true);
ResponsiveAnalogRead pot_attack(A3, true);
ResponsiveAnalogRead pot_release(A4, true);
ResponsiveAnalogRead cv_2(A5, true);

int current_waveform = WAVEFORM_SAWTOOTH_REVERSE;
int current_waveform_sub = WAVEFORM_SQUARE;

float octave_divider = 0.0;

float freq = 440.0;
float freq_2 = 440.0;
float freq_sub = 220.0;

float amplitude = 1.0;
float amplitude_2 = 0.0;
float amplitude_sub = 0.0;

int current_note = 0;

int attack_time = 1;
int release_time = 10;

void setup()
{
#ifdef DEBUG
  Serial.begin(115200);
#endif

  // Initialize the buttons and switches
  pinMode(BT_1, INPUT_PULLUP);
  pinMode(BT_2, INPUT_PULLUP);
  pinMode(OCTAVE_SWITCH_1, INPUT_PULLUP);
  pinMode(OCTAVE_SWITCH_2, INPUT_PULLUP);
  pinMode(ENVELOPE_SWITCH, INPUT_PULLUP);

  // Audio connections require memory to work.  For more
  // detailed information, see the MemoryAndCpuUsage example
  AudioMemory(20);

  waveform1.begin(current_waveform);
  waveform2.begin(current_waveform);
  waveform_sub.begin(current_waveform_sub);

  waveform1.frequency(freq);
  waveform2.frequency(freq_2);
  waveform_sub.frequency(freq_sub);

  filter1.frequency(freq_sub * 2);
  filter1.resonance(0.6);

  waveform1.amplitude(amplitude);
  waveform2.amplitude(amplitude_2);
  waveform_sub.amplitude(amplitude_sub);

  // Configure DC signal for envelope2 (constant voltage source)
  dc_signal.amplitude(FILTER_ENV_GAIN);

  // Configure envelope1
  envelope1.attack(attack_time);
  envelope1.hold(0);
  envelope1.decay(0);
  envelope1.sustain(1.0);
  envelope1.release(release_time);
  envelope1.releaseNoteOn(0);

  // Configure envelope2 with the same ADSR values as envelope1
  envelope2.attack(attack_time);
  envelope2.hold(0);
  envelope2.decay(0);
  envelope2.sustain(1.0);
  envelope2.release(release_time);
  envelope2.releaseNoteOn(0);

  mixer1.gain(0, MAIN_MIX_GAIN);
  mixer1.gain(1, MAIN_MIX_GAIN);
  mixer1.gain(2, SUB_MIX_GAIN);

  // USB MIDI
  usbMIDI.setHandleNoteOn(OnNoteOn);
  usbMIDI.setHandleNoteOff(OnNoteOff);

  // Serial MIDI
  MIDI.setHandleNoteOn(OnNoteOn);
  MIDI.setHandleNoteOff(OnNoteOff);
  MIDI.begin(MIDI_CHANNEL);
}

void loop()
{
  // Handle MIDI messages
  usbMIDI.read();
  MIDI.read();

  /*
   * Read the buttons and knobs, scale knobs to 0-1.0
   */
  button_1.update();
  button_2.update();
  pot_freq_2.update();
  pot_amplitude_2.update();
  pot_amplitude_sub.update();
  pot_attack.update();
  pot_release.update();
  cv_2.update();

#ifdef DEBUG
  // @todo impl modulation
  if (cv_2.hasChanged())
  {
    Serial.println("CV 2:");
    Serial.println(cv_2.getValue());
  }
#endif

  // Set sub oscillator to octave -1, -2 or 0 (disabled)
  if (octave_switch_1.update() || octave_switch_2.update())
  {
    octave_divider = computeOctaveDivider(octave_switch_1.read(), octave_switch_2.read());

    if (octave_divider == 0.0)
    {
      mixer1.gain(2, 0.0);
    }
    else
    {
      freq_sub = computeSubFrequency(freq, octave_divider);

      AudioNoInterrupts();

      waveform_sub.frequency(freq_sub);
      filter1.frequency(freq_sub * 2);
      mixer1.gain(2, SUB_MIX_GAIN);

      AudioInterrupts();
    }
  }

  if (envelope_switch.update())
  {
    if (envelope_switch.read() == LOW)
    {
      envelope1.decay(release_time);
      envelope1.sustain(0.0);
      envelope1.release(0);
      envelope2.decay(release_time);
      envelope2.sustain(0.0);
      envelope2.release(0);
    }
    else
    {
      envelope1.decay(0);
      envelope1.sustain(1.0);
      envelope1.release(release_time);
      envelope2.decay(0);
      envelope2.sustain(1.0);
      envelope2.release(release_time);
    }
  }

  // 1v/Oct (0 - 10v)
  // float cv = (float)analogRead(A9) / 102.3;

  // Update the frequency of the second oscillator
  if (pot_freq_2.hasChanged())
  {
    freq_2 = computeFrequencyFromNote(current_note, (float)pot_freq_2.getValue() / 1023.0);

    waveform2.frequency(freq_2);
  }

  // Update the amplitude of the second oscillator
  if (pot_amplitude_2.hasChanged())
  {
    amplitude_2 = (float)pot_amplitude_2.getValue() / 1023.0;

    waveform2.amplitude(amplitude_2);
  }

  // Update the amplitude of the sub oscillator
  if (pot_amplitude_sub.hasChanged())
  {
    amplitude_sub = ((float)pot_amplitude_sub.getValue() / 1023.0) * 0.5; // attenuation to avoid clipping in filter

    waveform_sub.amplitude(amplitude_sub);
  }

  // Button 1 changes the waveform type of the main oscillators
  if (button_1.fallingEdge())
  {
    switch (current_waveform)
    {
    case WAVEFORM_SAWTOOTH_REVERSE:
      current_waveform = WAVEFORM_SQUARE;
      break;
    case WAVEFORM_SQUARE:
      current_waveform = WAVEFORM_SAWTOOTH_REVERSE;
      break;
    }

#ifdef DEBUG
    Serial.println("Osc 1 & 2 waveform:");
    Serial.println(current_waveform);
#endif

    AudioNoInterrupts();

    waveform1.begin(current_waveform);
    waveform2.begin(current_waveform);

    AudioInterrupts();
  }

  // Button 2 changes the waveform type of the sub oscillator
  if (button_2.fallingEdge())
  {
    switch (current_waveform_sub)
    {
    case WAVEFORM_SAWTOOTH_REVERSE:
      current_waveform_sub = WAVEFORM_SQUARE;
      break;
    case WAVEFORM_SQUARE:
      current_waveform_sub = WAVEFORM_TRIANGLE;
      break;
    case WAVEFORM_TRIANGLE:
      current_waveform_sub = WAVEFORM_SAWTOOTH_REVERSE;
      break;
    }

#ifdef DEBUG
    Serial.println("Sub osc waveform:");
    Serial.println(current_waveform_sub);
#endif

    waveform_sub.begin(current_waveform_sub);
  }

  if (pot_attack.hasChanged())
  {
    float pot_normalized = (float)pot_attack.getValue() / 1023.0;
    attack_time = pot_normalized * 149 + 1; // 1 to 150ms

    AudioNoInterrupts();

    envelope1.attack(attack_time);
    envelope2.attack(attack_time);

    AudioInterrupts();
  }

  if (pot_release.hasChanged())
  {
    float pot_normalized = (float)pot_release.getValue() / 1023.0;
    release_time = pot_normalized * 598 + 2; // 2 to 600ms

    AudioNoInterrupts();

    envelope1.release(release_time);
    envelope2.release(release_time);

    AudioInterrupts();
  }
}

// MIDI event handlers
void OnNoteOn(byte channel, byte note, byte velocity)
{
  current_note = note;
  float sustain = (float)velocity / 127.0;

  // update main oscillator
  freq = computeFrequencyFromNote(current_note, 0);

  // update other oscillators
  freq_2 = computeFrequencyFromNote(current_note, (float)pot_freq_2.getValue() / 1023.0);

  AudioNoInterrupts();

  waveform1.frequency(freq);
  waveform2.frequency(freq_2);

  // update sub oscillator
  if (octave_divider > 0.0)
  {
    freq_sub = computeSubFrequency(freq, octave_divider);

    waveform_sub.frequency(freq_sub);
    filter1.frequency(freq_sub * 2);
  }

  envelope1.sustain(sustain);
  envelope2.sustain(sustain * 0.8);

  envelope1.noteOn();
  envelope2.noteOn();

  AudioInterrupts();
}

void OnNoteOff(byte channel, byte note, byte velocity)
{
  envelope1.noteOff();
  envelope2.noteOff();
}

float computeSubFrequency(float freq, float octave_divider)
{
  return octave_divider > 0.0 ? freq / octave_divider : 0.0;
}

/*
 * Compute the octave divider based on the octave switch inputs.
 *
 * The computation is a little bit weird because of the switch schematic.
 */
float computeOctaveDivider(int octave_switch_1, int octave_switch_2)
{
  if (octave_switch_2 == LOW)
  {
    return 0.0;
  }
  else if (octave_switch_1 == LOW)
  {
    return 4.0;
  }

  return 2.0;
}

/**
 * Compute the frequency of the waveform based on the note and pot input.
 *
 * The note input is the MIDI note number, and the pot input is from 0 to 1.
 *
 * @see https://vcvrack.com/manual/VoltageStandards#Pitch-and-Frequencies
 */
float computeFrequencyFromNote(byte note, float pot)
{
  float offset = pot * 16.0 - 8.0; // -8 to 8
  float note_offset = note + offset;

  return 440.0 * powf(2.0, (float)(note_offset - 73) * 0.08333333); // 440.0 = A4
}

/**
 * Compute the frequency of the waveform based on the pot and cv input.
 *
 * The pot input is should be from 0 to 1, and the cv input from 0 to 10.
 *
 * @see https://vcvrack.com/manual/VoltageStandards#Pitch-and-Frequencies
 */
float computeFrequencyFromCV(float cv, float pot)
{
  float offset = pot * 2.0 - 1.0; // -1 to 1
  float cv_offset = cv + offset;

  if (cv_offset < 0)
  {
    cv_offset = 0.0;
  }
  else if (cv_offset > 10)
  {
    cv_offset = 10.0;
  }

  return 32.7032 * pow(2, cv_offset); // f0 = C0 = 32.7032
}
