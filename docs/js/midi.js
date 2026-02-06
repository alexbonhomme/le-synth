import {
  SYSEX_HEADER,
  SYSEX_END,
  SYSEX_GET_REPLY,
  SYSEX_GET_REQUEST,
  SYSEX_ARP_GET_REQUEST,
  SYSEX_ARP_REPLY_CMD,
  SYSEX_ARP_SET_CMD,
  SYSEX_ARP_MAX_STEPS,
  SYSEX_ARP_NUM_MODES,
} from './constants.js';

// ——— Web MIDI API ———

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

// ——— SysEx (ICARUS) ———

export function buildGetChannelSysex() {
  return SYSEX_GET_REQUEST;
}

export function buildSysex(channel) {
  if (channel < 1 || channel > 16) return null;
  const nn = channel - 1;
  return new Uint8Array([...SYSEX_HEADER, nn, SYSEX_END]);
}

export function parseChannelFromSysex(data) {
  if (!data || data.length !== 6) return null;
  if (data[0] !== 0xf0 || data[5] !== 0xf7) return null;
  if (data[1] !== 0x7d || data[2] !== 0x00) return null;
  if (data[3] !== 0x01 && data[3] !== SYSEX_GET_REPLY) return null;
  const nn = data[4];
  if (nn > 15) return null;
  return nn + 1;
}

export function buildGetArpStepsSysex() {
  return SYSEX_ARP_GET_REQUEST;
}

/**
 * Parse arp steps reply: F0 7D 00 05 [len0][8 bytes][len1][8 bytes][len2][8 bytes] F7.
 * Returns [ [step0, ...], [step1, ...], [step2, ...] ] or null.
 */
export function parseArpStepsFromSysex(data) {
  const total = 4 + SYSEX_ARP_NUM_MODES * (1 + SYSEX_ARP_MAX_STEPS) + 1; // 32
  if (!data || data.length !== total) return null;
  if (data[0] !== 0xf0 || data[1] !== 0x7d || data[2] !== 0x00 || data[3] !== SYSEX_ARP_REPLY_CMD || data[data.length - 1] !== 0xf7) return null;
  const result = [];
  let off = 4;
  for (let m = 0; m < SYSEX_ARP_NUM_MODES; m++) {
    const len = Math.min(data[off], SYSEX_ARP_MAX_STEPS);
    off += 1;
    const steps = [];
    for (let i = 0; i < len; i++) {
      steps.push(data[off + i]);
    }
    result.push(steps);
    off += SYSEX_ARP_MAX_STEPS;
  }
  return result;
}

/**
 * Build set arp steps Sysex: F0 7D 00 06 [mode][len][0..8 bytes] F7.
 * @param {number} mode - 0..2
 * @param {number[]} steps - up to 8 uint8 values
 */
export function buildSetArpStepsSysex(mode, steps) {
  if (mode < 0 || mode > 2) return null;
  const len = Math.min(steps.length, SYSEX_ARP_MAX_STEPS);
  // Exact length 7+len so no trailing byte (Web MIDI rejects extra 0 as "running status")
  const arr = new Uint8Array(7 + len);
  arr[0] = 0xf0;
  arr[1] = 0x7d;
  arr[2] = 0x00;
  arr[3] = SYSEX_ARP_SET_CMD;
  arr[4] = mode;
  arr[5] = len;
  for (let i = 0; i < len; i++) {
    arr[6 + i] = steps[i] & 0xff;
  }
  arr[6 + len] = 0xf7;
  return arr;
}
