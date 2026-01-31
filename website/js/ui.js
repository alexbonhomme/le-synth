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
