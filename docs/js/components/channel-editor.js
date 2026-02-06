import './channel-grid.js';

class ChannelEditor extends HTMLElement {
  constructor() {
    super();
    this.currentChannelEl = null;
    this.gridEl = null;
  }

  connectedCallback() {
    this.render();
    this.cacheElements();
    this.bindEvents();
  }

  render() {
    this.innerHTML = `
      <div id="currentChannelWrap" class="flex items-baseline justify-between gap-4 px-4 py-3 bg-[#0f0f12] border border-surface-border rounded-lg mb-5">
        <span class="text-xs uppercase tracking-wider text-gray-500">Current MIDI channel</span>
        <span id="currentChannel" class="text-lg font-semibold text-accent">—</span>
      </div>

      <span class="block text-xs uppercase tracking-wider text-gray-500 mb-2">Select MIDI channel</span>
      <channel-grid id="channelGrid" class="grid grid-cols-8 gap-1"></channel-grid>
    `;
  }

  cacheElements() {
    this.currentChannelEl = this.querySelector('#currentChannel');
    this.gridEl = this.querySelector('#channelGrid');
  }

  bindEvents() {
    if (!this.gridEl) return;
    this.gridEl.addEventListener('channel-select', (event) => {
      // Re-dispatch upwards so the app shell can listen on <channel-editor>.
      this.dispatchEvent(
        new CustomEvent('channel-select', {
          detail: event.detail,
          bubbles: true,
        }),
      );
    });
  }

  /**
   * Update the displayed and active channel.
   * @param {number|null} channel
   */
  setChannel(channel) {
    if (!this.currentChannelEl || !this.gridEl) return;

    if (channel != null) {
      this.currentChannelEl.textContent = String(channel);
      this.currentChannelEl.className = 'text-lg font-semibold text-accent';
      if (typeof this.gridEl.setActiveChannel === 'function') {
        this.gridEl.setActiveChannel(channel);
      }
    } else {
      this.currentChannelEl.textContent = '—';
      this.currentChannelEl.className = 'text-lg font-normal text-gray-500';
      if (typeof this.gridEl.setActiveChannel === 'function') {
        this.gridEl.setActiveChannel(null);
      }
    }
  }
}

customElements.define('channel-editor', ChannelEditor);

