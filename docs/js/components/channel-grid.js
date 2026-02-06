const BUTTON_BASE_CLASSES =
  'aspect-square text-sm font-medium bg-[#0f0f12] border border-surface-border rounded-lg text-[#e8e6e3] hover:border-accent hover:text-accent hover:bg-accent/10 transition-colors';

const ACTIVE_CLASSES = ['bg-accent', 'border-accent', 'text-[#0f0f12]', 'font-semibold'];
const INACTIVE_CLASSES = ['bg-[#0f0f12]', 'border-surface-border', 'text-[#e8e6e3]', 'font-medium'];

class ChannelGrid extends HTMLElement {
  constructor() {
    super();
    this._activeChannel = null;
  }

  static get observedAttributes() {
    return ['active-channel'];
  }

  connectedCallback() {
    if (!this.hasChildNodes()) {
      this.render();
    }
    this.addEventListener('click', (event) => {
      const btn = event.target.closest('button[data-channel]');
      if (!btn || !this.contains(btn)) return;
      const channel = parseInt(btn.dataset.channel ?? '0', 10);
      if (!channel) return;
      this.setActiveChannel(channel);
      this.dispatchEvent(
        new CustomEvent('channel-select', {
          detail: { channel },
          bubbles: true,
        }),
      );
    });
  }

  attributeChangedCallback(name, _old, value) {
    if (name === 'active-channel') {
      const channel = parseInt(value ?? '0', 10);
      if (channel) {
        this._activeChannel = channel;
        this.applyActiveClasses();
      }
    }
  }

  render() {
    this.innerHTML = `
      <div class="channel-grid-inner grid gap-1 grid-cols-8 lg:grid-cols-[repeat(16,minmax(0,1fr))]"></div>
    `;
    const inner = this.querySelector('.channel-grid-inner');
    if (!inner) return;
    for (let ch = 1; ch <= 16; ch++) {
      const btn = document.createElement('button');
      btn.type = 'button';
      btn.className = BUTTON_BASE_CLASSES;
      btn.dataset.channel = String(ch);
      btn.textContent = String(ch);
      inner.appendChild(btn);
    }
    this.applyActiveClasses();
  }

  setActiveChannel(channel) {
    this._activeChannel = channel;
    this.setAttribute('active-channel', String(channel));
    this.applyActiveClasses();
  }

  applyActiveClasses() {
    this.querySelectorAll('button[data-channel]').forEach((btn) => {
      const ch = parseInt(btn.dataset.channel ?? '0', 10);
      const isActive = ch === this._activeChannel;
      ACTIVE_CLASSES.forEach((c) => { btn.classList.toggle(c, isActive); });
      INACTIVE_CLASSES.forEach((c) => { btn.classList.toggle(c, !isActive); });
    });
  }
}

customElements.define('channel-grid', ChannelGrid);

