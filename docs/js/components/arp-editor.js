import './arp-modes.js';

class ArpEditor extends HTMLElement {
  constructor() {
    super();
    this.arpModesEl = null;
  }

  connectedCallback() {
    this.render();
    this.cacheElements();
    this.bindEvents();
  }

  render() {
    this.innerHTML = `
      <section class="mt-8 pt-6 border-t border-surface-border">
        <h2 class="text-base font-semibold mb-1">ARP modes</h2>
        <p class="text-xs text-gray-500 mb-4 leading-relaxed">
          Click a cell to set each step's value (1–8); length sets how many steps are used.
        </p>
        <arp-modes
          id="arpModes"
          class="flex flex-col gap-5 lg:flex-row lg:gap-6 lg:items-stretch"
        ></arp-modes>
      </section>
    `;
  }

  cacheElements() {
    this.arpModesEl = this.querySelector('#arpModes');
  }

  bindEvents() {
    if (!this.arpModesEl) return;
    this.arpModesEl.addEventListener('arp-change', (event) => {
      this.dispatchEvent(
        new CustomEvent('arp-change', {
          detail: event.detail,
          bubbles: true,
        }),
      );
    });
  }

  /**
   * Update arp UI from device data. data = [ steps0[], steps1[], steps2[] ].
   * @param {number[][]} data
   */
  setFromData(data) {
    if (!this.arpModesEl || typeof this.arpModesEl.setFromData !== 'function') return;
    this.arpModesEl.setFromData(data);
  }
}

customElements.define('arp-editor', ArpEditor);

