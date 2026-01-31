import { buildSysex, buildGetChannelSysex, parseChannelFromSysex } from './sysex.js';
import { setStatus, setActiveChannel, setCurrentChannelDisplay, buildChannelButtons } from './ui.js';
import { findOutputByName, attachInputListeners, requestMIDIAccess } from './midi.js';
import { MIDI_DEVICE_NAME } from './constants.js';

(() => {
  const statusEl = document.getElementById('status');
  const channelGrid = document.getElementById('channelGrid');
  const currentChannelEl = document.getElementById('currentChannel');

  let midiAccess = null;
  let icarusOutput = null;

  function resolveIcarus() {
    icarusOutput = midiAccess ? findOutputByName(midiAccess, MIDI_DEVICE_NAME) : null;
    return icarusOutput;
  }

  function sendChannel(channel) {
    if (!icarusOutput) return;

    const data = buildSysex(channel);
    if (!data) return;

    try {
      icarusOutput.port.send(data);
      setActiveChannel(channelGrid, channel);
      setCurrentChannelDisplay(currentChannelEl, channel);
      setStatus(statusEl, 'Sent channel ' + channel + ' to ' + icarusOutput.port.name + '.', 'connected');
    } catch (err) {
      setStatus(statusEl, 'Send failed: ' + err.message, 'error');
    }
  }

  function onMidiMessage(e) {
    const channel = parseChannelFromSysex(e.data);
    if (channel != null) {
      setCurrentChannelDisplay(currentChannelEl, channel);
      setActiveChannel(channelGrid, channel);
      const name = icarusOutput ? icarusOutput.port.name : 'Icarus';
      // setStatus(statusEl, 'Current channel ' + channel + ' from ' + name + '.', 'connected');
    }
  }

  function requestCurrentChannel() {
    if (!icarusOutput) {
      setCurrentChannelDisplay(currentChannelEl, null);
      return;
    }
    setCurrentChannelDisplay(currentChannelEl, null);
    try {
      icarusOutput.port.send(buildGetChannelSysex());
    } catch (_) {}
  }

  function onStateChange() {
    resolveIcarus();
    if (icarusOutput) {
      setStatus(statusEl, 'Connected to ' + icarusOutput.port.name + '.', 'connected');
    } else if (midiAccess && midiAccess.outputs.size > 0) {
      setStatus(statusEl, 'Icarus MIDI device not found. Connect the device and refresh.', 'error');
    } else {
      setStatus(statusEl, 'No MIDI outputs available. Connect Icarus and refresh.', 'error');
    }
    requestCurrentChannel();
  }

  function init() {
    setStatus(statusEl, 'Requesting MIDI access…', 'pending');

    requestMIDIAccess()
      .then((access) => {
        midiAccess = access;
        attachInputListeners(midiAccess, onMidiMessage);
        access.onstatechange = () => {
          onStateChange();
          attachInputListeners(midiAccess, onMidiMessage);
        };

        resolveIcarus();
        if (icarusOutput) {
          setStatus(statusEl, 'Connected to ' + icarusOutput.port.name + '.', 'connected');
        } else if (access.outputs.size > 0) {
          setStatus(statusEl, 'Icarus MIDI device not found. Connect the device and refresh.', 'error');
        } else {
          setStatus(statusEl, 'No MIDI device connected. Connect Icarus and refresh.', 'error');
        }
        requestCurrentChannel();
      })
      .catch((err) => {
        const msg = err.message.includes('not supported')
          ? err.message
          : 'Cannot connect: ' + err.message;
        setStatus(statusEl, msg, 'error');
      });
  }

  buildChannelButtons(channelGrid, sendChannel);
  init();
})();
