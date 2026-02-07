import { CUSTOM_WAVEFORM_BANKS } from '../constants.js';

const SELECT_STYLE =
  'w-full min-w-0 py-1.5 px-2 text-sm bg-[#0f0f12] border border-surface-border rounded-lg text-[#e8e6e3] focus:outline-none focus:border-accent cursor-pointer';

const NAV_BTN_STYLE =
  'py-1 px-3 text-sm rounded-lg border border-surface-border bg-[#0f0f12] text-[#e8e6e3] hover:border-accent hover:bg-accent/10 focus:outline-none focus:border-accent transition-colors';

class WaveformSelector extends HTMLElement {
  connectedCallback() {
    if (!this.hasChildNodes()) {
      this.buildUi();
    }
  }

  buildUi() {
    this.innerHTML = '';

    const bankWrap = document.createElement('div');
    bankWrap.className = 'flex flex-col gap-1';

    const bankLabel = document.createElement('label');
    bankLabel.textContent = 'Bank';
    bankLabel.htmlFor = 'waveform-bank';
    bankLabel.className = 'text-[0.7rem] uppercase tracking-wider text-gray-500';

    const bankSelect = document.createElement('select');
    bankSelect.id = 'waveform-bank';
    bankSelect.className = SELECT_STYLE;
    bankSelect.dataset.role = 'bank';
    for (const bank of CUSTOM_WAVEFORM_BANKS) {
      const opt = document.createElement('option');
      opt.value = String(bank.id);
      opt.textContent = bank.label;
      bankSelect.appendChild(opt);
    }
    bankSelect.addEventListener('change', () => this.emitChange());

    bankWrap.appendChild(bankLabel);
    bankWrap.appendChild(bankSelect);
    this.appendChild(bankWrap);

    const indexWrap = document.createElement('div');
    indexWrap.className = 'flex flex-col gap-1';

    const indexLabel = document.createElement('label');
    indexLabel.textContent = 'Waveform';
    indexLabel.htmlFor = 'waveform-index';
    indexLabel.className = 'text-[0.7rem] uppercase tracking-wider text-gray-500';

    const indexSelect = document.createElement('select');
    indexSelect.id = 'waveform-index';
    indexSelect.className = SELECT_STYLE;
    indexSelect.dataset.role = 'index';
    indexSelect.addEventListener('change', () => this.emitChange());

    indexWrap.appendChild(indexLabel);
    indexWrap.appendChild(indexSelect);
    this.appendChild(indexWrap);

    const navWrap = document.createElement('div');
    navWrap.className = 'flex flex-row gap-2';
    const prevBtn = document.createElement('button');
    prevBtn.type = 'button';
    prevBtn.className = NAV_BTN_STYLE;
    prevBtn.textContent = '← Prev';
    prevBtn.title = 'Previous waveform';
    prevBtn.dataset.role = 'prev-waveform';
    const nextBtn = document.createElement('button');
    nextBtn.type = 'button';
    nextBtn.className = NAV_BTN_STYLE;
    nextBtn.textContent = 'Next →';
    nextBtn.title = 'Next waveform';
    nextBtn.dataset.role = 'next-waveform';
    prevBtn.addEventListener('click', () => this.selectPrevious());
    nextBtn.addEventListener('click', () => this.selectNext());
    navWrap.appendChild(prevBtn);
    navWrap.appendChild(nextBtn);
    this.appendChild(navWrap);

    this.fillIndexOptions(0);
  }

  get bankSelect() {
    return this.querySelector('[data-role="bank"]');
  }

  get indexSelect() {
    return this.querySelector('[data-role="index"]');
  }

  fillIndexOptions(bankId) {
    const bank = CUSTOM_WAVEFORM_BANKS.find((b) => b.id === bankId);
    const count = bank ? bank.count : 0;
    const indexSelect = this.indexSelect;
    if (!indexSelect) return;

    const currentValue = indexSelect.value;
    indexSelect.innerHTML = '';
    for (let i = 0; i < count; i++) {
      const opt = document.createElement('option');
      opt.value = String(i);
      opt.textContent = String(i + 1);
      indexSelect.appendChild(opt);
    }
    if (currentValue !== '' && parseInt(currentValue, 10) < count) {
      indexSelect.value = currentValue;
    } else {
      indexSelect.value = '0';
    }
  }

  emitChange() {
    const bankSelect = this.bankSelect;
    const indexSelect = this.indexSelect;
    if (!bankSelect || !indexSelect) return;

    const bank = parseInt(bankSelect.value, 10);
    this.fillIndexOptions(bank);
    const index = parseInt(indexSelect.value, 10);

    this.dispatchEvent(
      new CustomEvent('waveform-change', {
        detail: { bank, index },
        bubbles: true,
      }),
    );
  }

  /**
   * Update UI from device data. { bank, index } (index 0-based).
   * @param {{ bank: number, index: number }|null} data
   */
  setFromData(data) {
    if (!data || typeof data.bank !== 'number' || typeof data.index !== 'number') return;

    const bankSelect = this.bankSelect;
    const indexSelect = this.indexSelect;
    if (!bankSelect || !indexSelect) return;

    const bank = Math.max(0, Math.min(2, data.bank));
    bankSelect.value = String(bank);
    this.fillIndexOptions(bank);

    const bankInfo = CUSTOM_WAVEFORM_BANKS[bank];
    const maxIndex = bankInfo ? bankInfo.count - 1 : 0;
    const index = Math.max(0, Math.min(maxIndex, data.index));
    indexSelect.value = String(index);
  }

  getValue() {
    const bankSelect = this.bankSelect;
    const indexSelect = this.indexSelect;
    if (!bankSelect || !indexSelect) return { bank: 0, index: 0 };
    return {
      bank: parseInt(bankSelect.value, 10),
      index: parseInt(indexSelect.value, 10),
    };
  }

  /** Switch to the next waveform (wrap to next bank, then to bank 0). */
  selectNext() {
    const { bank, index } = this.getValue();
    const bankInfo = CUSTOM_WAVEFORM_BANKS[bank];
    const count = bankInfo ? bankInfo.count : 0;
    let newBank = bank;
    let newIndex = index + 1;
    if (newIndex >= count) {
      newIndex = 0;
      newBank = bank + 1;
      if (newBank > 2) newBank = 0;
    }
    this.setFromData({ bank: newBank, index: newIndex });
    this.emitChange();
  }

  /** Switch to the previous waveform (wrap to previous bank, then to bank 2). */
  selectPrevious() {
    const { bank, index } = this.getValue();
    let newBank = bank;
    let newIndex = index - 1;
    if (newIndex < 0) {
      newBank = bank - 1;
      if (newBank < 0) newBank = 2;
      const bankInfo = CUSTOM_WAVEFORM_BANKS[newBank];
      newIndex = (bankInfo ? bankInfo.count : 1) - 1;
    }
    this.setFromData({ bank: newBank, index: newIndex });
    this.emitChange();
  }
}

customElements.define('waveform-selector', WaveformSelector);
