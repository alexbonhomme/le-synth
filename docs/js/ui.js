const STATUS_BASE = 'text-sm rounded-lg border px-3 py-2.5 mb-6';
const STATUS_PENDING = 'border-surface-border text-gray-500';
const STATUS_CONNECTED = 'border-success bg-success/10 text-green-300';
const STATUS_ERROR = 'border-error bg-error/10 text-red-300';

export function setStatus(statusEl, message, type) {
  statusEl.textContent = message;
  const variant = type === 'connected' ? STATUS_CONNECTED : type === 'error' ? STATUS_ERROR : STATUS_PENDING;
  statusEl.className = `${STATUS_BASE} ${variant}`;
}

export function setActiveChannel(channelGrid, channel) {
  const activeClasses = ['bg-accent', 'border-accent', 'text-[#0f0f12]', 'font-semibold'];
  const inactiveClasses = ['bg-[#0f0f12]', 'border-surface-border', 'text-[#e8e6e3]', 'font-medium'];
  channelGrid.querySelectorAll('button[data-channel]').forEach((btn) => {
    const isActive = parseInt(btn.dataset.channel, 10) === channel;
    activeClasses.forEach((c) => { btn.classList.toggle(c, isActive); });
    inactiveClasses.forEach((c) => { btn.classList.toggle(c, !isActive); });
  });
}

export function setCurrentChannelDisplay(currentChannelEl, channel) {
  currentChannelEl.textContent = channel != null ? String(channel) : '—';
  currentChannelEl.className = channel != null
    ? 'text-lg font-semibold text-accent'
    : 'text-lg font-normal text-gray-500';
}

export function buildChannelButtons(channelGrid, onChannelClick) {
  const base = 'aspect-square text-sm font-medium bg-[#0f0f12] border border-surface-border rounded-lg text-[#e8e6e3] hover:border-accent hover:text-accent hover:bg-accent/10 transition-colors';
  for (let ch = 1; ch <= 16; ch++) {
    const btn = document.createElement('button');
    btn.type = 'button';
    btn.className = base;
    btn.dataset.channel = ch;
    btn.textContent = ch;
    btn.addEventListener('click', () => onChannelClick(ch));
    channelGrid.appendChild(btn);
  }
}

const ARP_MAX_STEPS = 8;
const ARP_MAX_VALUE = 7;

const ARP_VALUE_CELL_BASE =
  'w-full h-7 flex items-center justify-center text-sm font-medium rounded border border-surface-border bg-[#1a1a1f] text-[#e8e6e3] hover:border-accent hover:text-accent hover:bg-accent/10 transition-colors';
const ARP_VALUE_CELL_ACTIVE =
  'border-accent bg-accent text-[#0f0f12] font-semibold';

/**
 * Build the arp steps UI: 3 modes, each with length (1–8) and a table of clickable value cells (0–7) per step.
 * @param {HTMLElement} container - #arpModes
 * @param {(mode: number, steps: number[], length: number) => void} onArpStepsChange - called when user edits; steps.length === length
 */
export function buildArpStepsUI(container, onArpStepsChange) {
  container.innerHTML = '';
  const modeLabels = ['Mode A', 'Mode B', 'Mode C'];
  for (let mode = 0; mode < 3; mode++) {
    const block = document.createElement('div');
    block.className = 'arp-mode-block bg-[#0f0f12] border border-surface-border rounded-lg px-4 py-3';
    const headerRow = document.createElement('div');
    headerRow.className = 'flex items-center justify-between gap-4 mb-3 px-0.5';
    const label = document.createElement('span');
    label.className = 'text-base font-semibold text-accent';
    label.textContent = modeLabels[mode];
    headerRow.appendChild(label);
    const lengthWrap = document.createElement('div');
    lengthWrap.className = 'flex items-center gap-2';
    const lengthLabel = document.createElement('label');
    lengthLabel.textContent = 'Length';
    lengthLabel.htmlFor = `arp-len-${mode}`;
    lengthLabel.className = 'text-[0.7rem] uppercase tracking-wider text-gray-500 m-0';
    const lengthSelect = document.createElement('select');
    lengthSelect.id = `arp-len-${mode}`;
    lengthSelect.className = 'arp-length-select w-auto min-w-[3rem] m-0 py-1.5 px-2 text-sm bg-[#0f0f12] border border-surface-border rounded-lg text-[#e8e6e3] focus:outline-none focus:border-accent cursor-pointer';
    lengthSelect.dataset.mode = String(mode);
    for (let i = 1; i <= ARP_MAX_STEPS; i++) {
      const opt = document.createElement('option');
      opt.value = i;
      opt.textContent = i;
      lengthSelect.appendChild(opt);
    }
    lengthSelect.value = String(ARP_MAX_STEPS);
    lengthWrap.appendChild(lengthLabel);
    lengthWrap.appendChild(lengthSelect);
    headerRow.appendChild(lengthWrap);
    block.appendChild(headerRow);

    const table = document.createElement('div');
    table.className = 'arp-steps-table overflow-x-auto';
    const tableEl = document.createElement('table');
    tableEl.className = 'w-full border-collapse table-fixed';
    const tbody = document.createElement('tbody');
    tbody.className = 'arp-steps-tbody';
    for (let value = ARP_MAX_VALUE; value >= 0; value--) {
      const tr = document.createElement('tr');
      tr.className = 'arp-value-row';
      tr.dataset.value = String(value);
      for (let stepIndex = 0; stepIndex < ARP_MAX_STEPS; stepIndex++) {
        const td = document.createElement('td');
        td.className = 'arp-step-col p-0.5 text-left';
        td.dataset.stepIndex = String(stepIndex);
        const btn = document.createElement('button');
        btn.type = 'button';
        btn.className = ARP_VALUE_CELL_BASE + (value === 0 ? ' ' + ARP_VALUE_CELL_ACTIVE : '');
        btn.dataset.value = String(value);
        btn.dataset.stepIndex = String(stepIndex);
        btn.textContent = '';
        btn.addEventListener('click', () => {
          const col = btn.closest('td');
          const stepIdx = parseInt(col.dataset.stepIndex, 10);
          tbody.querySelectorAll(`td.arp-step-col[data-step-index="${stepIdx}"]`).forEach((cell) => {
            const b = cell.querySelector('button');
            if (b) b.className = ARP_VALUE_CELL_BASE;
          });
          btn.className = ARP_VALUE_CELL_BASE + ' ' + ARP_VALUE_CELL_ACTIVE;
          emitArpChange(block, mode, onArpStepsChange);
        });
        td.appendChild(btn);
        tr.appendChild(td);
      }
      tbody.appendChild(tr);
    }
    tableEl.appendChild(tbody);
    table.appendChild(tableEl);
    block.appendChild(table);

    function setVisibleColumns(len) {
      block.querySelectorAll('td.arp-step-col').forEach((el) => {
        const stepIdx = parseInt(el.dataset.stepIndex, 10);
        el.classList.toggle('hidden', stepIdx >= len);
      });
    }
    setVisibleColumns(ARP_MAX_STEPS);
    lengthSelect.addEventListener('change', () => {
      const len = parseInt(lengthSelect.value, 10);
      setVisibleColumns(len);
      emitArpChange(block, mode, onArpStepsChange);
    });
    container.appendChild(block);
  }

  function emitArpChange(block, mode, callback) {
    const lengthSelect = block.querySelector('.arp-length-select');
    const length = parseInt(lengthSelect.value, 10);
    const tbody = block.querySelector('.arp-steps-tbody');
    const steps = [];
    for (let stepIndex = 0; stepIndex < length; stepIndex++) {
      const cells = tbody.querySelectorAll(`td.arp-step-col[data-step-index="${stepIndex}"]`);
      let value = 0;
      for (const cell of cells) {
        const btn = cell.querySelector('button');
        if (btn?.classList.contains('bg-accent')) {
          value = parseInt(btn.closest('tr')?.dataset.value ?? '0', 10);
          break;
        }
      }
      steps.push(value);
    }
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
    const len = Math.min(Math.max(steps.length, 1), ARP_MAX_STEPS);
    lengthSelect.value = len;
    block.querySelectorAll('td.arp-step-col').forEach((el) => {
      const stepIdx = parseInt(el.dataset.stepIndex, 10);
      el.classList.toggle('hidden', stepIdx >= len);
    });
    block.querySelectorAll('.arp-value-row').forEach((tr) => {
      const rowValue = parseInt(tr.dataset.value, 10);
      tr.querySelectorAll('td.arp-step-col').forEach((td) => {
        const stepIdx = parseInt(td.dataset.stepIndex, 10);
        const value = stepIdx < steps.length ? steps[stepIdx] : 0;
        const btn = td.querySelector('button');
        if (btn) {
          const isActive = rowValue === value;
          btn.className = ARP_VALUE_CELL_BASE + (isActive ? ' ' + ARP_VALUE_CELL_ACTIVE : '');
        }
      });
    });
  });
}
