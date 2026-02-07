import './waveform-selector.js';
import { getWaveformImagePath } from '../constants.js';

class WaveformEditor extends HTMLElement {
  constructor() {
    super();
    this.waveformSelectorEl = null;
    this.previewEl = null;
  }

  connectedCallback() {
    this.render();
    this.cacheElements();
    this.bindEvents();
    this.updatePreviewFromSelector();
  }

  render() {
    this.innerHTML = `
      <section class="mt-8 pt-6 border-t border-surface-border">
        <h2 class="text-base font-semibold mb-1">Custom waveform</h2>
        <p class="text-xs text-gray-500 mb-4 leading-relaxed">
          When the synth is set to arbitrary waveform mode, choose the bank and waveform (1-based).
        </p>
        <div class="flex flex-col gap-4">
          <waveform-selector
            id="waveformSelector"
            class="flex flex-wrap gap-4"
          ></waveform-selector>
          <div class="flex flex-col gap-1">
            <span class="text-[0.7rem] uppercase tracking-wider text-gray-500">Preview</span>
            <img
              id="waveformPreview"
              data-role="preview"
              src=""
              alt="Selected waveform"
              class="w-full object-contain bg-[#0f0f12] border border-surface-border rounded-lg"
            />
          </div>
        </div>
      </section>
    `;
  }

  cacheElements() {
    this.waveformSelectorEl = this.querySelector('#waveformSelector');
    this.previewEl = this.querySelector('[data-role="preview"]');
  }

  bindEvents() {
    if (!this.waveformSelectorEl) return;
    this.waveformSelectorEl.addEventListener('waveform-change', (event) => {
      this.updatePreviewFromSelector();
      this.dispatchEvent(
        new CustomEvent('waveform-change', {
          detail: event.detail,
          bubbles: true,
        }),
      );
    });
  }

  updatePreviewFromSelector() {
    if (!this.previewEl || !this.waveformSelectorEl || typeof this.waveformSelectorEl.getValue !== 'function') return;
    const { bank, index } = this.waveformSelectorEl.getValue();
    this.previewEl.src = getWaveformImagePath(bank, index);
  }

  /**
   * Update UI from device data. data = { bank, index } (index 0-based).
   * @param {{ bank: number, index: number }|null} data
   */
  setFromData(data) {
    if (!this.waveformSelectorEl || typeof this.waveformSelectorEl.setFromData !== 'function') return;
    this.waveformSelectorEl.setFromData(data);
    this.updatePreviewFromSelector();
  }
}

customElements.define('waveform-editor', WaveformEditor);
