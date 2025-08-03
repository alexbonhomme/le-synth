#include <Audio.h>
#include <Wire.h>
// #include <SPI.h>
// #include <SD.h>
// #include <SerialFlash.h>
#include <Bounce.h>

// GUItool: begin automatically generated code
AudioSynthWaveform waveform1;  // xy=687,580
AudioSynthWaveform waveform2;  // xy=688,627
AudioMixer4 mixer1;            // xy=897,623
AudioEffectEnvelope envelope1; // xy=1087,622
AudioOutputI2S i2s1;           // xy=1271,629
AudioConnection patchCord1(waveform1, 0, mixer1, 0);
AudioConnection patchCord2(waveform2, 0, mixer1, 1);
AudioConnection patchCord3(mixer1, envelope1);
AudioConnection patchCord4(envelope1, 0, i2s1, 0);
// GUItool: end automatically generated code

const int BT_1 = 0;
const int BT_2 = 1;

Bounce button_1 = Bounce(BT_1, 15);
Bounce button_2 = Bounce(BT_2, 15);

float pot_1 = 0.0;
float pot_2 = 0.0;
float freq = 0.0;
float freq_2 = 0.0;
float amplitude = 0.0;
float amplitude_2 = 0.0;

int current_waveform = WAVEFORM_SAWTOOTH_REVERSE;
int current_note = 0;
int current_velocity = 0;

void setup()
{
  // Serial.begin(9600);

  pinMode(BT_1, INPUT_PULLUP);
  pinMode(BT_2, INPUT_PULLUP);

  // Audio connections require memory to work.  For more
  // detailed information, see the MemoryAndCpuUsage example
  AudioMemory(10);

  waveform1.begin(current_waveform);
  waveform2.begin(current_waveform);

  waveform1.frequency(440);
  waveform2.frequency(440);

  waveform1.amplitude(1);
  waveform2.amplitude(1);

  envelope1.attack(5);
  envelope1.hold(0);
  envelope1.decay(0);
  envelope1.sustain(1);
  envelope1.release(30);

  mixer1.gain(0, 0.33);
  mixer1.gain(1, 0.33);

  // Set up MIDI callbacks
  usbMIDI.setHandleNoteOn(OnNoteOn);
  usbMIDI.setHandleNoteOff(OnNoteOff);
}

void loop()
{
  // Read the buttons and knobs, scale knobs to 0-1.0
  button_1.update();
  button_2.update();

  // Frequency
  pot_1 = (float)analogRead(A0) / 1023.0;
  pot_2 = (float)analogRead(A1) / 1023.0;
  // pot_3 = (float)analogRead(A2) / 1023.0;

  // 1v/Oct (0 - 10v)
  // float cv = (float)analogRead(A9) / 102.3;

  updateWaveforms();

  // Button 1 changes the waveform type
  if (button_1.fallingEdge())
  {
    switch (current_waveform)
    {
    case WAVEFORM_SINE:
      current_waveform = WAVEFORM_SAWTOOTH_REVERSE;
      // Serial.println("Sawtooth Reverse");
      break;
    case WAVEFORM_SAWTOOTH_REVERSE:
      current_waveform = WAVEFORM_SQUARE;
      // Serial.println("Square");
      break;
    case WAVEFORM_SQUARE:
      current_waveform = WAVEFORM_TRIANGLE;
      // Serial.println("Triangle");
      break;
    case WAVEFORM_TRIANGLE:
      current_waveform = WAVEFORM_SINE;
      // Serial.println("Sine");
      break;
    }

    AudioNoInterrupts();

    waveform1.begin(current_waveform);
    waveform2.begin(current_waveform);

    AudioInterrupts();
  }

  // Handle MIDI messages
  usbMIDI.read();
}

// MIDI event handlers
void OnNoteOn(byte channel, byte note, byte velocity)
{
  current_note = note;
  current_velocity = velocity;

  updateWaveforms();

  // Trigger envelope
  envelope1.noteOn();
}

void OnNoteOff(byte channel, byte note, byte velocity)
{
  // Release envelope
  envelope1.noteOff();
}

/**
 * Update the waveforms based on the current note, pot values, and velocity.
 */
void updateWaveforms()
{
  // update main oscillator
  freq = computeFrequencyFromNote(current_note, 0);
  amplitude = (float)current_velocity / 127.0;

  // update other oscillators
  freq_2 = computeFrequencyFromNote(current_note, pot_1);
  amplitude_2 = amplitude * pot_2;

  AudioNoInterrupts();

  waveform1.frequency(freq);
  waveform2.frequency(freq_2);

  waveform1.amplitude(amplitude);
  waveform2.amplitude(amplitude_2);

  AudioInterrupts();
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
