export function setStatus(statusEl, message, type) {
  statusEl.textContent = message;
  statusEl.className = 'status ' + (type || 'pending');
}

export function setActiveChannel(channelGrid, channel) {
  channelGrid.querySelectorAll('.channel-btn').forEach((btn) => {
    btn.classList.toggle('active', parseInt(btn.dataset.channel, 10) === channel);
  });
}

export function setCurrentChannelDisplay(currentChannelEl, channel) {
  currentChannelEl.textContent = channel != null ? String(channel) : '—';
  currentChannelEl.classList.toggle('unknown', channel == null);
}

export function buildChannelButtons(channelGrid, onChannelClick) {
  for (let ch = 1; ch <= 16; ch++) {
    const btn = document.createElement('button');
    btn.type = 'button';
    btn.className = 'channel-btn';
    btn.dataset.channel = ch;
    btn.textContent = ch;
    btn.addEventListener('click', () => onChannelClick(ch));
    channelGrid.appendChild(btn);
  }
}

const ARP_MAX_STEPS = 8;
const ARP_MAX_VALUE = 7;

/**
 * Build the arp steps UI: 3 modes, each with length (1–8) and 8 clickable cells (0–7).
 * @param {HTMLElement} container - #arpModes
 * @param {(mode: number, steps: number[], length: number) => void} onArpStepsChange - called when user edits; steps.length === length
 */
export function buildArpStepsUI(container, onArpStepsChange) {
  container.innerHTML = '';
  const modeLabels = ['Mode 0', 'Mode 1', 'Mode 2'];
  for (let mode = 0; mode < 3; mode++) {
    const block = document.createElement('div');
    block.className = 'arp-mode-block';
    const label = document.createElement('span');
    label.className = 'arp-mode-label';
    label.textContent = modeLabels[mode];
    block.appendChild(label);
    const lengthWrap = document.createElement('div');
    lengthWrap.className = 'arp-length-wrap';
    const lengthLabel = document.createElement('label');
    lengthLabel.textContent = 'Length';
    lengthLabel.htmlFor = `arp-len-${mode}`;
    const lengthSelect = document.createElement('select');
    lengthSelect.id = `arp-len-${mode}`;
    lengthSelect.className = 'arp-length-select';
    lengthSelect.dataset.mode = String(mode);
    for (let i = 1; i <= ARP_MAX_STEPS; i++) {
      const opt = document.createElement('option');
      opt.value = i;
      opt.textContent = i;
      lengthSelect.appendChild(opt);
    }
    lengthWrap.appendChild(lengthLabel);
    lengthWrap.appendChild(lengthSelect);
    block.appendChild(lengthWrap);
    const row = document.createElement('div');
    row.className = 'arp-steps-row';
    row.dataset.mode = String(mode);
    for (let i = 0; i < ARP_MAX_STEPS; i++) {
      const cell = document.createElement('button');
      cell.type = 'button';
      cell.className = 'arp-cell';
      cell.dataset.mode = String(mode);
      cell.dataset.index = String(i);
      cell.textContent = '0';
      cell.addEventListener('click', () => {
        const v = (parseInt(cell.textContent, 10) + 1) % (ARP_MAX_VALUE + 1);
        cell.textContent = String(v);
        emitArpChange(block, mode, onArpStepsChange);
      });
      row.appendChild(cell);
    }
    function setVisibleCells(len) {
      row.classList.remove('arp-length-1', 'arp-length-2', 'arp-length-3', 'arp-length-4', 'arp-length-5', 'arp-length-6', 'arp-length-7', 'arp-length-8');
      row.classList.add('arp-length-' + len);
    }
    setVisibleCells(ARP_MAX_STEPS);
    lengthSelect.addEventListener('change', () => {
      const len = parseInt(lengthSelect.value, 10);
      setVisibleCells(len);
      emitArpChange(block, mode, onArpStepsChange);
    });
    block.appendChild(row);
    container.appendChild(block);
  }

  function emitArpChange(block, mode, callback) {
    const lengthSelect = block.querySelector('.arp-length-select');
    const row = block.querySelector('.arp-steps-row');
    const length = parseInt(lengthSelect.value, 10);
    const steps = [];
    row.querySelectorAll('.arp-cell').forEach((cell, i) => {
      if (i < length) steps.push(parseInt(cell.textContent, 10));
    });
    callback(mode, steps, length);
  }
}

/**
 * Update arp UI from device data. data = [ steps0[], steps1[], steps2[] ].
 */
export function updateArpStepsFromData(container, data) {
  if (!data || data.length !== 3) return;
  container.querySelectorAll('.arp-mode-block').forEach((block, mode) => {
    const steps = data[mode] || [];
    const lengthSelect = block.querySelector('.arp-length-select');
    const row = block.querySelector('.arp-steps-row');
    const len = Math.min(Math.max(steps.length, 1), ARP_MAX_STEPS);
    lengthSelect.value = len;
    row.classList.remove('arp-length-1', 'arp-length-2', 'arp-length-3', 'arp-length-4', 'arp-length-5', 'arp-length-6', 'arp-length-7', 'arp-length-8');
    row.classList.add('arp-length-' + len);
    row.querySelectorAll('.arp-cell').forEach((cell, i) => {
      cell.textContent = String(i < steps.length ? steps[i] : 0);
    });
  });
}
