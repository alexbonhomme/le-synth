const STATUS_BASE = 'block w-full text-sm rounded-lg border px-3 py-2.5 mb-6';
const STATUS_PENDING = 'border-surface-border text-gray-500';
const STATUS_CONNECTED = 'border-success bg-success/10 text-green-300';
const STATUS_ERROR = 'border-error bg-error/10 text-red-300';

class MidiStatus extends HTMLElement {
  constructor() {
    super();
    this._message = this.getAttribute('message') ?? 'Request MIDI access to continue.';
    this._type = this.getAttribute('type') ?? 'pending';
  }

  static get observedAttributes() {
    return ['message', 'type'];
  }

  connectedCallback() {
    this.render();
  }

  attributeChangedCallback(name, _old, value) {
    if (name === 'message') {
      this._message = value;
    } else if (name === 'type') {
      this._type = value;
    }
    this.render();
  }

  setStatus(message, type = 'pending') {
    this._message = message;
    this._type = type;
    this.render();
  }

  render() {
    const variant =
      this._type === 'connected' ? STATUS_CONNECTED : this._type === 'error' ? STATUS_ERROR : STATUS_PENDING;
    this.className = `${STATUS_BASE} ${variant}`;
    this.textContent = this._message;
  }
}

customElements.define('midi-status', MidiStatus);

