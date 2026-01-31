/** SysEx header for ICARUS: F0 7D 00 01 nn F7 (nn = channel−1) */
export const SYSEX_HEADER = [0xf0, 0x7d, 0x00, 0x01];
export const SYSEX_END = 0xf7;
/** 5-byte get-channel request: F0 7D 00 03 F7 */
export const SYSEX_GET_REQUEST = new Uint8Array([0xf0, 0x7d, 0x00, 0x03, 0xf7]);
/** 6-byte SysEx: F0 7D 00 01 nn F7 (set) or F0 7D 00 02 nn F7 (get reply) */
export const SYSEX_GET_REPLY = 0x02;
