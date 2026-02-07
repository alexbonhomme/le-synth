/** Expected MIDI device name (partial match, case-insensitive) */
export const MIDI_DEVICE_NAME = 'Icarus MIDI';

/** SysEx header for ICARUS: F0 7D 00 01 nn F7 (nn = channel−1) */
export const SYSEX_HEADER = [0xf0, 0x7d, 0x00, 0x01];
export const SYSEX_END = 0xf7;
/** 5-byte get-channel request: F0 7D 00 03 F7 */
export const SYSEX_GET_REQUEST = new Uint8Array([0xf0, 0x7d, 0x00, 0x03, 0xf7]);
/** 6-byte SysEx: F0 7D 00 01 nn F7 (set) or F0 7D 00 02 nn F7 (get reply) */
export const SYSEX_GET_REPLY = 0x02;

/** Arp steps: get request F0 7D 00 04 F7; reply F0 7D 00 05 [len0][8][len1][8][len2][8] F7; set F0 7D 00 06 [mode][len][0..8 bytes] F7 */
export const SYSEX_ARP_GET_REQUEST = new Uint8Array([0xf0, 0x7d, 0x00, 0x04, 0xf7]);
export const SYSEX_ARP_REPLY_CMD = 0x05;
export const SYSEX_ARP_SET_CMD = 0x06;
export const SYSEX_ARP_MAX_STEPS = 8;
export const SYSEX_ARP_NUM_MODES = 3;

/** Custom waveform: get F0 7D 00 07 F7; reply F0 7D 00 08 bank index F7; set F0 7D 00 09 bank index F7. Bank: 0=FM, 1=Granular, 2=Overtone. */
export const SYSEX_CUSTOM_WAVEFORM_GET_REQUEST = new Uint8Array([0xf0, 0x7d, 0x00, 0x07, 0xf7]);
export const SYSEX_CUSTOM_WAVEFORM_REPLY_CMD = 0x08;
export const SYSEX_CUSTOM_WAVEFORM_SET_CMD = 0x09;

/** Bank labels and waveform counts (must match firmware). */
export const CUSTOM_WAVEFORM_BANKS = [
  { id: 0, label: 'FM', count: 122 },
  { id: 1, label: 'Granular', count: 44 },
  { id: 2, label: 'Overtone', count: 44 },
];

/** JSON waveform data files (one per bank), relative to docs. */
export const WAVEFORM_JSON_FILES = ['fmsynth', 'granular', 'overtone'];
