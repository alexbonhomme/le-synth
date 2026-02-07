import {
  attachInputListeners,
  buildGetArpStepsSysex,
  buildGetChannelSysex,
  buildGetCustomWaveformSysex,
  buildSetArpStepsSysex,
  buildSetCustomWaveformSysex,
  buildSysex,
  findOutputByName,
  parseArpStepsFromSysex,
  parseChannelFromSysex,
  parseCustomWaveformFromSysex,
  requestMIDIAccess,
} from './midi.js';
import { MIDI_DEVICE_NAME } from './constants.js';
import './components/midi-status.js';
import './components/channel-editor.js';
import './components/arp-editor.js';
import './components/waveform-editor.js';

class IcarusConfigApp extends HTMLElement {
  constructor() {
    super();
    this.midiAccess = null;
    this.icarusOutput = null;

    this.statusEl = null;
    this.channelEditor = null;
    this.arpEditor = null;
    this.waveformEditor = null;

    this.handleMidiMessage = this.handleMidiMessage.bind(this);
    this.handleStateChange = this.handleStateChange.bind(this);
  }

  connectedCallback() {
    this.render();
    this.cacheElements();
    this.setEditingVisible(false);
    this.buildUi();
    this.initMidi();
  }

  render() {
    this.innerHTML = `
      <h1 class="text-xl font-semibold tracking-wide">ICARUS</h1>
      <p class="text-sm text-gray-500 mb-6">MIDI configuration tool</p>

      <midi-status id="status"></midi-status>

      <channel-editor id="channelEditor" class="block mb-6"></channel-editor>

      <arp-editor id="arpEditor"></arp-editor>

      <waveform-editor id="waveformEditor"></waveform-editor>
    `;
  }

  cacheElements() {
    this.statusEl = this.querySelector('#status');
    this.channelEditor = this.querySelector('#channelEditor');
    this.arpEditor = this.querySelector('#arpEditor');
    this.waveformEditor = this.querySelector('#waveformEditor');
  }

  buildUi() {
    if (!this.channelEditor || !this.arpEditor) return;
    this.channelEditor.addEventListener('channel-select', (event) => {
      const channel = event.detail?.channel;
      if (channel) this.sendChannel(channel);
    });
    this.arpEditor.addEventListener('arp-change', (event) => {
      const { mode, steps } = event.detail ?? {};
      if (mode != null && Array.isArray(steps)) {
        this.sendArpSteps(mode, steps);
      }
    });
    this.waveformEditor?.addEventListener('waveform-change', (event) => {
      const { bank, index } = event.detail ?? {};
      if (typeof bank === 'number' && typeof index === 'number') {
        this.sendCustomWaveform(bank, index);
      }
    });
  }

  resolveIcarus() {
    this.icarusOutput = this.midiAccess ? findOutputByName(this.midiAccess, MIDI_DEVICE_NAME) : null;
    return this.icarusOutput;
  }

  sendChannel(channel) {
    if (!this.icarusOutput || !this.channelEditor || !this.statusEl) return;

    const data = buildSysex(channel);
    if (!data) return;

    try {
      this.icarusOutput.port.send(data);
      if (typeof this.channelEditor.setChannel === 'function') {
        this.channelEditor.setChannel(channel);
      }
      this.statusEl.setStatus('Sent channel ' + channel + ' to ' + this.icarusOutput.port.name + '.', 'connected');
    } catch (err) {
      this.statusEl.setStatus('Send failed: ' + err.message, 'error');
    }
  }

  handleMidiMessage(event) {
    if (!this.channelEditor || !this.statusEl || !this.arpEditor || !this.waveformEditor) return;

    const channel = parseChannelFromSysex(event.data);
    if (channel != null) {
      if (typeof this.channelEditor.setChannel === 'function') {
        this.channelEditor.setChannel(channel);
      }
      return;
    }

    const arpSteps = parseArpStepsFromSysex(event.data);
    if (arpSteps != null && typeof this.arpEditor.setFromData === 'function') {
      this.arpEditor.setFromData(arpSteps);
      this.statusEl.setStatus('Arp steps loaded from device.', 'connected');
      return;
    }

    const customWaveform = parseCustomWaveformFromSysex(event.data);
    if (customWaveform != null && typeof this.waveformEditor?.setFromData === 'function') {
      this.waveformEditor.setFromData(customWaveform);
      this.statusEl.setStatus('Custom waveform loaded from device.', 'connected');
    }
  }

  requestCurrentChannel() {
    if (!this.channelEditor) return;

    if (!this.icarusOutput) {
      if (typeof this.channelEditor.setChannel === 'function') {
        this.channelEditor.setChannel(null);
      }
      return;
    }

    if (typeof this.channelEditor.setChannel === 'function') {
      this.channelEditor.setChannel(null);
    }
    try {
      this.icarusOutput.port.send(buildGetChannelSysex());
    } catch {
      // ignore send errors here; status updates will reflect connection state
    }
  }

  requestArpSteps() {
    if (!this.icarusOutput) return;
    try {
      this.icarusOutput.port.send(buildGetArpStepsSysex());
    } catch {
      // ignore send errors here; status updates will reflect connection state
    }
  }

  requestCustomWaveform() {
    if (!this.icarusOutput) return;
    try {
      this.icarusOutput.port.send(buildGetCustomWaveformSysex());
    } catch {
      // ignore
    }
  }

  sendArpSteps(mode, steps) {
    if (!this.icarusOutput || !this.statusEl) return;
    const data = buildSetArpStepsSysex(mode, steps);
    if (!data) return;

    try {
      this.icarusOutput.port.send(data);
      this.statusEl.setStatus('Arp steps mode ' + mode + ' sent to device.', 'connected');
    } catch (err) {
      this.statusEl.setStatus('Send arp steps failed: ' + err.message, 'error');
    }
  }

  sendCustomWaveform(bank, index) {
    if (!this.icarusOutput || !this.statusEl) return;
    const data = buildSetCustomWaveformSysex(bank, index);
    if (!data) return;

    try {
      this.icarusOutput.port.send(data);
      this.statusEl.setStatus('Custom waveform sent to device.', 'connected');
    } catch (err) {
      this.statusEl.setStatus('Send custom waveform failed: ' + err.message, 'error');
    }
  }

  handleStateChange() {
    if (!this.statusEl) return;

    this.resolveIcarus();
    if (this.icarusOutput) {
      this.statusEl.setStatus('Connected to ' + this.icarusOutput.port.name + '.', 'connected');
      this.setEditingVisible(true);
    } else if (this.midiAccess && this.midiAccess.outputs.size > 0) {
      this.statusEl.setStatus('Icarus MIDI device not found. Connect the device and refresh.', 'error');
      this.setEditingVisible(false);
    } else {
      this.statusEl.setStatus('No MIDI outputs available. Connect Icarus and refresh.', 'error');
      this.setEditingVisible(false);
    }
    this.requestCurrentChannel();
    this.requestArpSteps();
    this.requestCustomWaveform();
  }

  initMidi() {
    if (!this.statusEl) return;

    this.statusEl.setStatus('Requesting MIDI access…', 'pending');

    requestMIDIAccess()
      .then((access) => {
        this.midiAccess = access;
        attachInputListeners(this.midiAccess, this.handleMidiMessage);
        access.onstatechange = () => {
          this.handleStateChange();
          attachInputListeners(this.midiAccess, this.handleMidiMessage);
        };

        this.resolveIcarus();
        if (this.icarusOutput) {
          this.statusEl.setStatus('Connected to ' + this.icarusOutput.port.name + '.', 'connected');
          this.setEditingVisible(true);
        } else if (access.outputs.size > 0) {
          this.statusEl.setStatus('Icarus MIDI device not found. Connect the device and refresh.', 'error');
          this.setEditingVisible(false);
        } else {
          this.statusEl.setStatus('No MIDI device connected. Connect Icarus and refresh.', 'error');
          this.setEditingVisible(false);
        }
        this.requestCurrentChannel();
        this.requestArpSteps();
        this.requestCustomWaveform();
      })
      .catch((err) => {
        const msg = err.message.includes('not supported') ? err.message : 'Cannot connect: ' + err.message;
        this.statusEl.setStatus(msg, 'error');
        this.setEditingVisible(false);
      });
  }

  setEditingVisible(isVisible) {
    const action = isVisible ? 'remove' : 'add';
    this.channelEditor?.classList[action]('hidden');
    this.arpEditor?.classList[action]('hidden');
    this.waveformEditor?.classList[action]('hidden');
  }
}

customElements.define('icarus-config-app', IcarusConfigApp);

