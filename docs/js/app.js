import {
  buildGetArpStepsSysex,
  buildGetChannelSysex,
  buildSetArpStepsSysex,
  buildSysex,
  parseArpStepsFromSysex,
  parseChannelFromSysex,
} from './sysex.js';
import { findOutputByName, attachInputListeners, requestMIDIAccess } from './midi.js';
import { MIDI_DEVICE_NAME } from './constants.js';
import './components/midi-status.js';
import './components/channel-grid.js';
import './components/arp-modes.js';

class IcarusConfigApp extends HTMLElement {
  constructor() {
    super();
    this.midiAccess = null;
    this.icarusOutput = null;

    this.statusEl = null;
    this.channelGrid = null;
    this.currentChannelEl = null;
    this.arpModes = null;

    this.handleMidiMessage = this.handleMidiMessage.bind(this);
    this.handleStateChange = this.handleStateChange.bind(this);
  }

  connectedCallback() {
    this.render();
    this.cacheElements();
    this.buildUi();
    this.initMidi();
  }

  render() {
    this.innerHTML = `
      <h1 class="text-xl font-semibold tracking-wide">ICARUS</h1>
      <p class="text-sm text-gray-500 mb-6">MIDI configuration tool</p>

      <midi-status id="status"></midi-status>

      <div id="currentChannelWrap" class="flex items-baseline justify-between gap-4 px-4 py-3 bg-[#0f0f12] border border-surface-border rounded-lg mb-5">
        <span class="text-xs uppercase tracking-wider text-gray-500">Current MIDI channel</span>
        <span id="currentChannel" class="text-lg font-semibold text-accent">—</span>
      </div>

      <span class="block text-xs uppercase tracking-wider text-gray-500 mb-2">Select MIDI channel</span>
      <channel-grid id="channelGrid" class="grid grid-cols-8 gap-1 mb-6"></channel-grid>

      <section id="arpSection" class="mt-8 pt-6 border-t border-surface-border">
        <h2 class="text-base font-semibold mb-1">ARP modes</h2>
        <p class="text-xs text-gray-500 mb-4 leading-relaxed">
          Click a cell to set each step's value (1–8); length sets how many steps are used.
        </p>
        <arp-modes id="arpModes" class="flex flex-col gap-5"></arp-modes>
      </section>
    `;
  }

  cacheElements() {
    this.statusEl = this.querySelector('#status');
    this.channelGrid = this.querySelector('#channelGrid');
    this.currentChannelEl = this.querySelector('#currentChannel');
    this.arpModes = this.querySelector('#arpModes');
  }

  buildUi() {
    if (!this.channelGrid || !this.arpModes) return;
    this.channelGrid.addEventListener('channel-select', (event) => {
      const channel = event.detail?.channel;
      if (channel) this.sendChannel(channel);
    });
    this.arpModes.addEventListener('arp-change', (event) => {
      const { mode, steps } = event.detail ?? {};
      if (mode != null && Array.isArray(steps)) {
        this.sendArpSteps(mode, steps);
      }
    });
  }

  resolveIcarus() {
    this.icarusOutput = this.midiAccess ? findOutputByName(this.midiAccess, MIDI_DEVICE_NAME) : null;
    return this.icarusOutput;
  }

  sendChannel(channel) {
    if (!this.icarusOutput || !this.channelGrid || !this.currentChannelEl || !this.statusEl) return;

    const data = buildSysex(channel);
    if (!data) return;

    try {
      this.icarusOutput.port.send(data);
      if (typeof this.channelGrid.setActiveChannel === 'function') {
        this.channelGrid.setActiveChannel(channel);
      }
      this.updateCurrentChannelDisplay(channel);
      this.statusEl.setStatus('Sent channel ' + channel + ' to ' + this.icarusOutput.port.name + '.', 'connected');
    } catch (err) {
      this.statusEl.setStatus('Send failed: ' + err.message, 'error');
    }
  }

  handleMidiMessage(event) {
    if (!this.channelGrid || !this.currentChannelEl || !this.statusEl || !this.arpModes) return;

    const channel = parseChannelFromSysex(event.data);
    if (channel != null) {
      this.updateCurrentChannelDisplay(channel);
      if (typeof this.channelGrid.setActiveChannel === 'function') {
        this.channelGrid.setActiveChannel(channel);
      }
      return;
    }

    const arpSteps = parseArpStepsFromSysex(event.data);
    if (arpSteps != null && typeof this.arpModes.setFromData === 'function') {
      this.arpModes.setFromData(arpSteps);
      this.statusEl.setStatus('Arp steps loaded from device.', 'connected');
    }
  }

  requestCurrentChannel() {
    if (!this.currentChannelEl) return;

    if (!this.icarusOutput) {
      this.updateCurrentChannelDisplay(null);
      return;
    }

    this.updateCurrentChannelDisplay(null);
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

  handleStateChange() {
    if (!this.statusEl) return;

    this.resolveIcarus();
    if (this.icarusOutput) {
      this.statusEl.setStatus('Connected to ' + this.icarusOutput.port.name + '.', 'connected');
    } else if (this.midiAccess && this.midiAccess.outputs.size > 0) {
      this.statusEl.setStatus('Icarus MIDI device not found. Connect the device and refresh.', 'error');
    } else {
      this.statusEl.setStatus('No MIDI outputs available. Connect Icarus and refresh.', 'error');
    }
    this.requestCurrentChannel();
    this.requestArpSteps();
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
        } else if (access.outputs.size > 0) {
          this.statusEl.setStatus('Icarus MIDI device not found. Connect the device and refresh.', 'error');
        } else {
          this.statusEl.setStatus('No MIDI device connected. Connect Icarus and refresh.', 'error');
        }
        this.requestCurrentChannel();
        this.requestArpSteps();
      })
      .catch((err) => {
        const msg = err.message.includes('not supported') ? err.message : 'Cannot connect: ' + err.message;
        this.statusEl.setStatus(msg, 'error');
      });
  }

  updateCurrentChannelDisplay(channel) {
    if (!this.currentChannelEl) return;
    if (channel != null) {
      this.currentChannelEl.textContent = String(channel);
      this.currentChannelEl.className = 'text-lg font-semibold text-accent';
    } else {
      this.currentChannelEl.textContent = '—';
      this.currentChannelEl.className = 'text-lg font-normal text-gray-500';
    }
  }
}

customElements.define('icarus-config-app', IcarusConfigApp);

