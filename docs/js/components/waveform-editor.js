import './waveform-selector.js';
import { WAVEFORM_JSON_FILES } from '../constants.js';

const WAVEFORM_BG = '#0f0f12';
const WAVEFORM_STROKE = '#7d9cd4';
const WAVEFORM_ZERO_LINE = '#2a2a32';
const WAVEFORM_INT16_RANGE = 65536;
const WAVEFORM_PADDING_PX = 16;
const WAVEFORM_LINE_WIDTH_PX = 2;

class WaveformEditor extends HTMLElement {
  constructor() {
    super();
    this.waveformSelectorEl = null;
    this.canvasEl = null;
    this.bankDataCache = /** @type {Record<number, number[][]>} */ ({});
  }

  connectedCallback() {
    this.render();
    this.cacheElements();
    this.bindEvents();
    this.resizeObserver = new ResizeObserver(() => this.resizeAndDraw());
    if (this.canvasEl) this.resizeObserver.observe(this.canvasEl);
    this.updatePreviewFromSelector();
  }

  disconnectedCallback() {
    this.resizeObserver?.disconnect();
  }

  render() {
    this.innerHTML = `
      <section class="mt-8 pt-6 border-t border-surface-border">
        <h2 class="text-base font-semibold mb-1">Custom waveform</h2>
        <p class="text-xs text-gray-500 mb-4 leading-relaxed">
          Please select a waveform you want to use in the custom mode. <br/>Those come from the <a href="https://www.adventurekid.se/akrt/waveforms/" target="_blank" rel="noopener noreferrer">Adventure Kid Wave Forms (AKWF)</a> library by Kristoffer Ekstrand.
        </p>
        <div class="flex flex-col gap-4">
          <waveform-selector
            id="waveformSelector"
            class="flex flex-wrap gap-4"
          ></waveform-selector>
          <div class="flex flex-col gap-1">
            <span class="text-[0.7rem] uppercase tracking-wider text-gray-500">Preview</span>
            <canvas
              id="waveformPreview"
              data-role="preview"
              class="block w-full min-w-0 h-64 rounded-lg border border-surface-border"
              style="background: ${WAVEFORM_BG};"
            ></canvas>
          </div>
        </div>
      </section>
    `;
  }

  cacheElements() {
    this.waveformSelectorEl = this.querySelector('#waveformSelector');
    this.canvasEl = this.querySelector('[data-role="preview"]');
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

  /**
   * @param {number} bank
   * @returns {Promise<number[][]>}
   */
  async getBankWaveforms(bank) {
    const id = Math.max(0, Math.min(2, bank));
    if (this.bankDataCache[id]) return this.bankDataCache[id];
    const name = WAVEFORM_JSON_FILES[id];
    const res = await fetch(`assets/waveforms/${name}.json`);
    if (!res.ok) return [];
    const data = await res.json();
    this.bankDataCache[id] = data;
    return data;
  }

  resizeAndDraw() {
    if (!this.canvasEl) return;
    const rect = this.canvasEl.getBoundingClientRect();
    if (rect.width <= 0) {
      requestAnimationFrame(() => this.resizeAndDraw());
      return;
    }
    const dpr = window.devicePixelRatio || 1;
    const cssW = rect.width;
    const cssH = Math.max(1, rect.height || 96);
    const w = Math.max(1, Math.floor(cssW * dpr));
    const h = Math.max(1, Math.floor(cssH * dpr));
    if (this.canvasEl.width !== w || this.canvasEl.height !== h) {
      this.canvasEl.width = w;
      this.canvasEl.height = h;
    }
    const { bank, index } = this.waveformSelectorEl?.getValue?.() ?? { bank: 0, index: 0 };
    const waveforms = this.bankDataCache[Math.max(0, Math.min(2, bank))];
    const samples = waveforms?.[index];
    if (samples?.length) this.drawWaveform(this.canvasEl, samples, dpr);
    else {
      const ctx = this.canvasEl.getContext('2d');
      if (ctx) {
        ctx.fillStyle = WAVEFORM_BG;
        ctx.fillRect(0, 0, this.canvasEl.width, this.canvasEl.height);
      }
    }
  }

  /**
   * @param {HTMLCanvasElement} canvas
   * @param {number[]} samples
   * @param {number} dpr
   */
  drawWaveform(canvas, samples, dpr = 1) {
    const ctx = canvas.getContext('2d');
    if (!ctx || !samples.length) return;

    const w = canvas.width;
    const h = canvas.height;
    const padding = Math.round(WAVEFORM_PADDING_PX * dpr);
    const drawW = w - padding * 2;
    const drawH = h - padding * 2;

    ctx.fillStyle = WAVEFORM_BG;
    ctx.fillRect(0, 0, w, h);

    const min = Math.min(...samples);
    const max = Math.max(...samples);
    const range = max - min || WAVEFORM_INT16_RANGE;
    const mid = (max + min) / 2;
    const scale = (drawH / 2) / (range / 2);

    const zeroY = padding + drawH / 2;
    ctx.strokeStyle = WAVEFORM_ZERO_LINE;
    ctx.lineWidth = Math.max(1, 1 * dpr);
    ctx.setLineDash([4 * dpr, 4 * dpr]);
    ctx.beginPath();
    ctx.moveTo(padding, zeroY);
    ctx.lineTo(padding + drawW, zeroY);
    ctx.stroke();
    ctx.setLineDash([]);

    ctx.strokeStyle = WAVEFORM_STROKE;
    ctx.lineWidth = Math.max(1, WAVEFORM_LINE_WIDTH_PX * dpr);
    ctx.lineJoin = 'round';
    ctx.lineCap = 'round';
    ctx.beginPath();

    const n = samples.length;
    for (let i = 0; i < n; i++) {
      const x = padding + (i / (n - 1)) * drawW;
      const y = padding + drawH / 2 - (samples[i] - mid) * scale;
      if (i === 0) ctx.moveTo(x, y);
      else ctx.lineTo(x, y);
    }
    ctx.stroke();
  }

  async updatePreviewFromSelector() {
    if (!this.canvasEl || !this.waveformSelectorEl || typeof this.waveformSelectorEl.getValue !== 'function') return;

    const { bank } = this.waveformSelectorEl.getValue();
    await this.getBankWaveforms(bank);
    this.resizeAndDraw();
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
