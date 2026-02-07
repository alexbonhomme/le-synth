#!/usr/bin/env node
/**
 * Export waveform .h arrays from src/waveforms to JSON for the docs app.
 * Reads AKWF_*_256.h files, extracts the 256-sample int16 array, writes one
 * JSON file per bank: docs/assets/waveforms/{fmsynth,granular,overtone}.json
 * Each JSON file is an array of waveforms; each waveform is an array of 256 numbers.
 */

const fs = require('fs');
const path = require('path');

const SRC = path.join(__dirname, '..', 'src', 'waveforms');
const OUT = path.join(__dirname, '..', 'docs', 'assets', 'waveforms');

const BANKS = [
  { dir: 'AKWF_fmsynth', out: 'fmsynth.json' },
  { dir: 'AKWF_granular', out: 'granular.json' },
  { dir: 'AKWF_overtone', out: 'overtone.json' },
];

function parseWaveformArray(content) {
  const match = content.match(/\{\s*([^}]+)\s*\}/s);
  if (!match) return null;
  const list = match[1]
    .split(',')
    .map((s) => parseInt(s.trim(), 10))
    .filter((n) => !Number.isNaN(n));
  return list.length === 256 ? list : null;
}

function exportBank(bank) {
  const dir = path.join(SRC, bank.dir);
  if (!fs.existsSync(dir)) {
    console.warn('Skip (missing):', dir);
    return;
  }
  const files = fs.readdirSync(dir).filter((f) => f.endsWith('_256.h'));
  files.sort((a, b) => {
    const nA = parseInt(a.replace(/^AKWF_\w+_(\d+)_256\.h$/, '$1'), 10) || 0;
    const nB = parseInt(b.replace(/^AKWF_\w+_(\d+)_256\.h$/, '$1'), 10) || 0;
    return nA - nB;
  });
  const waveforms = [];
  for (const f of files) {
    const content = fs.readFileSync(path.join(dir, f), 'utf8');
    const samples = parseWaveformArray(content);
    if (samples) waveforms.push(samples);
  }
  const outPath = path.join(OUT, bank.out);
  fs.mkdirSync(path.dirname(outPath), { recursive: true });
  fs.writeFileSync(outPath, JSON.stringify(waveforms), 'utf8');
  console.log('%s → %s (%d waveforms)', bank.dir, bank.out, waveforms.length);
}

fs.mkdirSync(OUT, { recursive: true });
for (const bank of BANKS) exportBank(bank);
