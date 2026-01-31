import { buildSysex, buildGetChannelSysex, parseChannelFromSysex } from './sysex.js';
import { setStatus, setActiveChannel, setCurrentChannelDisplay, buildChannelButtons } from './ui.js';
import { fillOutputs, attachInputListeners, requestMIDIAccess } from './midi.js';

(() => {
  const statusEl = document.getElementById('status');
  const outputSelect = document.getElementById('output');
  const channelGrid = document.getElementById('channelGrid');
  const currentChannelEl = document.getElementById('currentChannel');

  let midiAccess = null;

  function sendChannel(channel) {
    const outputId = outputSelect.value;
    if (!outputId || !midiAccess) return;

    const output = midiAccess.outputs.get(outputId);
    if (!output) return;

    const data = buildSysex(channel);
    if (!data) return;

    try {
      output.send(data);
      setActiveChannel(channelGrid, channel);
      setCurrentChannelDisplay(currentChannelEl, channel);
      setStatus(statusEl, 'Sent channel ' + channel + ' to ' + output.name + '.', 'connected');
    } catch (err) {
      setStatus(statusEl, 'Send failed: ' + err.message, 'error');
    }
  }

  function onMidiMessage(e) {
    const channel = parseChannelFromSysex(e.data);
    if (channel != null) {
      setCurrentChannelDisplay(currentChannelEl, channel);
      setActiveChannel(channelGrid, channel);
      const output = outputSelect.value && midiAccess ? midiAccess.outputs.get(outputSelect.value) : null;
      const name = output ? output.name : 'device';
      setStatus(statusEl, 'Current channel ' + channel + ' from ' + name + '.', 'connected');
    }
  }

  function requestCurrentChannel() {
    const outputId = outputSelect.value;
    if (!outputId || !midiAccess) {
      setCurrentChannelDisplay(currentChannelEl, null);
      return;
    }
    const output = midiAccess.outputs.get(outputId);
    if (!output) {
      setCurrentChannelDisplay(currentChannelEl, null);
      return;
    }
    setCurrentChannelDisplay(currentChannelEl, null);
    try {
      output.send(buildGetChannelSysex());
    } catch (_) {}
  }

  function onStateChange() {
    fillOutputs(outputSelect, midiAccess);
    if (midiAccess && midiAccess.outputs.size > 0) {
      setStatus(statusEl, 'MIDI access granted. Select an output and a channel.', 'connected');
    } else {
      setStatus(statusEl, 'No MIDI outputs available.', 'pending');
    }
    requestCurrentChannel();
  }

  function init() {
    setStatus(statusEl, 'Requesting MIDI access (Sysex allowed)…', 'pending');

    requestMIDIAccess()
      .then((access) => {
        midiAccess = access;
        fillOutputs(outputSelect, midiAccess);
        attachInputListeners(midiAccess, onMidiMessage);
        access.onstatechange = () => {
          onStateChange();
          attachInputListeners(midiAccess, onMidiMessage);
        };

        if (access.outputs.size > 0) {
          setStatus(statusEl, 'MIDI access granted. Select an output and a channel.', 'connected');
        } else {
          setStatus(statusEl, 'No MIDI outputs found. Connect the device and refresh.', 'pending');
        }
        outputSelect.addEventListener('change', requestCurrentChannel);
        requestCurrentChannel();
      })
      .catch((err) => {
        const msg = err.message.includes('not supported')
          ? err.message
          : 'MIDI access denied: ' + err.message;
        setStatus(statusEl, msg, 'error');
      });
  }

  buildChannelButtons(channelGrid, sendChannel);
  init();
})();
