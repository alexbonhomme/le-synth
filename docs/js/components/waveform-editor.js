import './waveform-selector.js';
import { CUSTOM_WAVEFORM_BANKS, WAVEFORM_JSON_FILES } from '../constants.js';

const WAVEFORM_BG = '#0f0f12';
const WAVEFORM_STROKE = '#7d9cd4';
const WAVEFORM_ZERO_LINE = '#2a2a32';
const WAVEFORM_INT16_RANGE = 65536;
const WAVEFORM_PADDING_PX = 16;
const WAVEFORM_LINE_WIDTH_PX = 2;

const FAVORITES_STORAGE_KEY = 'icarus-favorite-waveforms';
const FAVORITES_SLOT_COUNT = 8;

/**
 * @param {{ bank: number, index: number }|null} entry
 * @returns {string}
 */
function formatFavoriteLabel(entry) {
  if (!entry || typeof entry.bank !== 'number' || typeof entry.index !== 'number') return '—';
  const bankInfo = CUSTOM_WAVEFORM_BANKS[Math.max(0, Math.min(2, entry.bank))];
  const label = bankInfo ? bankInfo.label : '?';
  return `${label} #${entry.index + 1}`;
}

/**
 * @returns {({ bank: number, index: number }|null)[]}
 */
function loadFavoritesFromStorage() {
  try {
    const raw = localStorage.getItem(FAVORITES_STORAGE_KEY);
    if (!raw) return Array(FAVORITES_SLOT_COUNT).fill(null);
    const parsed = JSON.parse(raw);
    if (!Array.isArray(parsed)) return Array(FAVORITES_SLOT_COUNT).fill(null);
    const result = [];
    for (let i = 0; i < FAVORITES_SLOT_COUNT; i++) {
      const item = parsed[i];
      if (item && typeof item.bank === 'number' && typeof item.index === 'number') {
        result.push({ bank: item.bank, index: item.index });
      } else {
        result.push(null);
      }
    }
    return result;
  } catch {
    return Array(FAVORITES_SLOT_COUNT).fill(null);
  }
}

/**
 * @param {({ bank: number, index: number }|null)[]} favorites
 */
function saveFavoritesToStorage(favorites) {
  try {
    localStorage.setItem(FAVORITES_STORAGE_KEY, JSON.stringify(favorites));
  } catch {
    // ignore
  }
}

class WaveformEditor extends HTMLElement {
  constructor() {
    super();
    this.waveformSelectorEl = null;
    this.canvasEl = null;
    this.bankDataCache = /** @type {Record<number, number[][]>} */ ({});
    /** @type {({ bank: number, index: number }|null)[]} */
    this.favorites = loadFavoritesFromStorage();
  }

  connectedCallback() {
    this.render();
    this.cacheElements();
    this.bindEvents();
    this.renderFavoritesLabels();
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
            class="flex flex-wrap items-end gap-2 md:gap-4"
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
          <div class="flex flex-col gap-2 pt-2 border-t border-surface-border">
            <span class="text-[0.7rem] uppercase tracking-wider text-gray-500">Favorites</span>
            <div class="grid md:grid-cols-4 gap-2 md:gap-5" data-role="favorites-container">
              ${Array.from({ length: FAVORITES_SLOT_COUNT }, (_, i) => `
                <div class="flex items-center gap-1 min-w-0" data-slot="${i}">
                  <span class="text-xs text-gray-400 w-20 flex-grow" data-role="favorite-label" data-slot="${i}">—</span>
                  <button type="button" data-role="load-favorite" data-slot="${i}" class="py-1 px-2 text-xs rounded border border-surface-border bg-[#0f0f12] text-[#e8e6e3] hover:border-accent disabled:opacity-50 disabled:cursor-not-allowed focus:outline-none focus:border-accent" title="Load">Load</button>
                  <button type="button" data-role="save-favorite" data-slot="${i}" class="py-1 px-2 text-xs rounded border border-surface-border bg-[#0f0f12] text-[#e8e6e3] hover:border-accent focus:outline-none focus:border-accent" title="Save current here">Save</button>
                </div>
              `).join('')}
            </div>
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
    const favContainer = this.querySelector('[data-role="favorites-container"]');
    if (favContainer) {
      favContainer.addEventListener('click', (e) => {
        const loadBtn = e.target?.closest?.('[data-role="load-favorite"]');
        const saveBtn = e.target?.closest?.('[data-role="save-favorite"]');
        const slot = loadBtn?.dataset?.slot ?? saveBtn?.dataset?.slot;
        if (slot !== undefined) {
          const i = parseInt(slot, 10);
          if (loadBtn) this.loadFavorite(i);
          else if (saveBtn) this.saveFavorite(i);
        }
      });
    }
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

  renderFavoritesLabels() {
    this.favorites.forEach((entry, i) => {
      const labelEl = this.querySelector(`[data-role="favorite-label"][data-slot="${i}"]`);
      const loadBtn = this.querySelector(`[data-role="load-favorite"][data-slot="${i}"]`);
      if (labelEl) labelEl.textContent = `${i + 1}: ${formatFavoriteLabel(entry)}`;
      if (loadBtn) loadBtn.disabled = entry == null;
    });
  }

  /**
   * @param {number} slotIndex 0..4
   */
  loadFavorite(slotIndex) {
    const entry = this.favorites[slotIndex];
    if (!entry) return;
    this.setFromData(entry);
    this.dispatchEvent(
      new CustomEvent('waveform-change', {
        detail: { bank: entry.bank, index: entry.index },
        bubbles: true,
      }),
    );
  }

  /**
   * @param {number} slotIndex 0..4
   */
  saveFavorite(slotIndex) {
    if (!this.waveformSelectorEl || typeof this.waveformSelectorEl.getValue !== 'function') return;
    const { bank, index } = this.waveformSelectorEl.getValue();
    this.favorites[slotIndex] = { bank, index };
    saveFavoritesToStorage(this.favorites);
    this.renderFavoritesLabels();
  }
}

customElements.define('waveform-editor', WaveformEditor);
