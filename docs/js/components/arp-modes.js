const ARP_MAX_STEPS = 8;
const ARP_MAX_VALUE = 7;

const ARP_VALUE_CELL_BASE =
  'w-full h-7 flex items-center justify-center text-sm font-medium rounded border border-surface-border bg-[#1a1a1f] text-[#e8e6e3] hover:border-accent hover:text-accent hover:bg-accent/10 transition-colors';
const ARP_VALUE_CELL_ACTIVE = 'border-accent bg-accent text-[#0f0f12] font-semibold';

class ArpModes extends HTMLElement {
  connectedCallback() {
    if (!this.hasChildNodes()) {
      this.buildUi();
    }
  }

  buildUi() {
    this.innerHTML = '';
    const modeLabels = ['Mode A', 'Mode B', 'Mode C'];
    for (let mode = 0; mode < 3; mode++) {
      const block = document.createElement('div');
      block.className =
        'arp-mode-block bg-[#0f0f12] border border-surface-border rounded-lg px-4 py-3 w-full lg:flex-1 lg:min-w-0';

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
      lengthSelect.className =
        'arp-length-select w-auto min-w-[3rem] m-0 py-1.5 px-2 text-sm bg-[#0f0f12] border border-surface-border rounded-lg text-[#e8e6e3] focus:outline-none focus:border-accent cursor-pointer';
      lengthSelect.dataset.mode = String(mode);
      for (let i = 1; i <= ARP_MAX_STEPS; i++) {
        const opt = document.createElement('option');
        opt.value = String(i);
        opt.textContent = String(i);
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
            const stepIdx = parseInt(col?.dataset.stepIndex ?? '0', 10);
            tbody
              .querySelectorAll(`td.arp-step-col[data-step-index="${stepIdx}"]`)
              .forEach((cell) => {
                const b = cell.querySelector('button');
                if (b) b.className = ARP_VALUE_CELL_BASE;
              });
            btn.className = ARP_VALUE_CELL_BASE + ' ' + ARP_VALUE_CELL_ACTIVE;
            this.emitArpChange(block, mode);
          });
          td.appendChild(btn);
          tr.appendChild(td);
        }
        tbody.appendChild(tr);
      }

      tableEl.appendChild(tbody);
      table.appendChild(tableEl);
      block.appendChild(table);

      const setVisibleColumns = (len) => {
        block.querySelectorAll('td.arp-step-col').forEach((el) => {
          const stepIdx = parseInt(el.dataset.stepIndex ?? '0', 10);
          el.classList.toggle('hidden', stepIdx >= len);
        });
      };

      setVisibleColumns(ARP_MAX_STEPS);

      lengthSelect.addEventListener('change', () => {
        const len = parseInt(lengthSelect.value ?? '0', 10);
        setVisibleColumns(len);
        this.emitArpChange(block, mode);
      });

      this.appendChild(block);
    }
  }

  emitArpChange(block, mode) {
    const lengthSelect = block.querySelector('.arp-length-select');
    const length = parseInt(lengthSelect?.value ?? '0', 10) || 1;
    const tbody = block.querySelector('.arp-steps-tbody');
    const steps = [];

    for (let stepIndex = 0; stepIndex < length; stepIndex++) {
      const cells = tbody?.querySelectorAll(`td.arp-step-col[data-step-index="${stepIndex}"]`) ?? [];
      let value = 0;
      for (const cell of cells) {
        const btn = cell.querySelector('button');
        if (btn?.classList.contains('bg-accent')) {
          const row = btn.closest('tr');
          value = parseInt(row?.dataset.value ?? '0', 10);
          break;
        }
      }
      steps.push(value);
    }

    this.dispatchEvent(
      new CustomEvent('arp-change', {
        detail: { mode, steps, length },
        bubbles: true,
      }),
    );
  }

  /**
   * Update arp UI from device data. data = [ steps0[], steps1[], steps2[] ].
   * @param {number[][]} data
   */
  setFromData(data) {
    if (!data || data.length !== 3) return;
    this.querySelectorAll('.arp-mode-block').forEach((block, mode) => {
      const steps = data[mode] || [];
      const lengthSelect = block.querySelector('.arp-length-select');
      if (!lengthSelect) return;
      const len = Math.min(Math.max(steps.length, 1), ARP_MAX_STEPS);
      lengthSelect.value = String(len);

      block.querySelectorAll('td.arp-step-col').forEach((el) => {
        const stepIdx = parseInt(el.dataset.stepIndex ?? '0', 10);
        el.classList.toggle('hidden', stepIdx >= len);
      });

      block.querySelectorAll('.arp-value-row').forEach((tr) => {
        const rowValue = parseInt(tr.dataset.value ?? '0', 10);
        tr.querySelectorAll('td.arp-step-col').forEach((td) => {
          const stepIdx = parseInt(td.dataset.stepIndex ?? '0', 10);
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
}

customElements.define('arp-modes', ArpModes);

