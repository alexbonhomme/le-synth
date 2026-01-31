export function fillOutputs(outputSelect, midiAccess) {
  const selectedId = outputSelect.value;
  outputSelect.innerHTML = '<option value="">— Select port —</option>';
  if (!midiAccess) return;

  midiAccess.outputs.forEach((port, id) => {
    const opt = document.createElement('option');
    opt.value = id;
    opt.textContent = port.name || id;
    outputSelect.appendChild(opt);
  });

  outputSelect.disabled = midiAccess.outputs.size === 0;
  if (midiAccess.outputs.size === 1) {
    outputSelect.selectedIndex = 1;
  } else if (selectedId && midiAccess.outputs.has(selectedId)) {
    outputSelect.value = selectedId;
  }
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
