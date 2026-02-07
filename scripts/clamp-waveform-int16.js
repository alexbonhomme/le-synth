#!/usr/bin/env node
/**
 * Clamp all waveform sample values in *_256.h files to int16_t range
 * [-32768, 32767] to fix -Wnarrowing compiler warnings.
 */

const fs = require('node:fs');
const path = require('node:path');

const MIN = -32768;
const MAX = 32767;
const WAVEFORMS_DIR = path.join(__dirname, '..', 'src', 'waveforms');

function clamp(n) {
  const v = parseInt(n, 10);
  if (Number.isNaN(v)) return n;
  if (v < MIN) return String(MIN);
  if (v > MAX) return String(MAX);
  return n;
}

function processFile(filePath) {
  const content = fs.readFileSync(filePath, 'utf8');
  const match = content.match(/\{\s*([^}]+)\s*\}/s);
  if (!match) return { changed: false };

  const inner = match[1];
  const clamped = inner.replace(/-?\d+/g, clamp);
  if (clamped === inner) return { changed: false };

  const newContent = content.replace(/\{\s*[^}]+\s*\}/s, '{ ' + clamped + ' }');
  fs.writeFileSync(filePath, newContent, 'utf8');
  return { changed: true };
}

function findWaveformFiles(dir, files = []) {
  const entries = fs.readdirSync(dir, { withFileTypes: true });
  for (const e of entries) {
    const full = path.join(dir, e.name);
    if (e.isDirectory()) {
      findWaveformFiles(full, files);
    } else if (e.name.endsWith('_256.h') && e.name !== 'Waveforms.h') {
      files.push(full);
    }
  }
  return files;
}

const files = findWaveformFiles(WAVEFORMS_DIR);
let changedCount = 0;
for (const f of files) {
  const { changed } = processFile(f);
  if (changed) {
    changedCount++;
    console.log(path.relative(WAVEFORMS_DIR, f));
  }
}
console.log(`Clamped ${changedCount} of ${files.length} waveform files.`);
