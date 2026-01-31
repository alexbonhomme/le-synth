import { SYSEX_HEADER, SYSEX_END, SYSEX_GET_REPLY, SYSEX_GET_REQUEST } from './constants.js';

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
