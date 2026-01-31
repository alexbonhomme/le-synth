/** Returns the first MIDI output whose name contains the given string (case-insensitive). */
export function findOutputByName(midiAccess, nameSubstring) {
  if (!midiAccess || !nameSubstring) return null;
  const lower = nameSubstring.toLowerCase();
  for (const [id, port] of midiAccess.outputs) {
    if ((port.name || '').toLowerCase().includes(lower)) return { id, port };
  }
  return null;
}

export function attachInputListeners(midiAccess, onMidiMessage) {
  if (!midiAccess) return;
  midiAccess.inputs.forEach((port) => {
    port.onmidimessage = onMidiMessage;
  });
}

export function requestMIDIAccess() {
  if (!navigator.requestMIDIAccess) {
    return Promise.reject(new Error('Web MIDI API is not supported in this browser.'));
  }
  return navigator.requestMIDIAccess({ sysex: true });
}
