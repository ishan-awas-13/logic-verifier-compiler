import { test, before, after, describe } from 'node:test';
import assert from 'node:assert/strict';
import http from 'node:http';
import path from 'node:path';
import { fileURLToPath } from 'node:url';
import express from 'express';
import cors from 'cors';
import { loadConfig } from '../server/config.js';
import { runCompiler } from '../server/runner.js';

const __filename = fileURLToPath(import.meta.url);
const __dirname = path.dirname(__filename);
const rootDir = path.resolve(__dirname, '..');

let server;
let baseUrl;

function request(path, options = {}) {
  return new Promise((resolve, reject) => {
    const url = new URL(path, baseUrl);
    const req = http.request(url, options, res => {
      let data = '';
      res.on('data', chunk => { data += chunk; });
      res.on('end', () => {
        let json = null;
        try {
          json = JSON.parse(data);
        } catch (_) {}
        resolve({ status: res.statusCode, headers: res.headers, data, json });
      });
    });
    req.on('error', reject);
    if (options.body) {
      req.write(typeof options.body === 'string' ? options.body : JSON.stringify(options.body));
    }
    req.end();
  });
}

describe('Dashboard Backend Integration Tests', () => {
  before(async () => {
    const { app } = await import('../server/server.js');
    await new Promise(resolve => {
      server = app.listen(0, '127.0.0.1', () => {
        const port = server.address().port;
        baseUrl = `http://127.0.0.1:${port}`;
        resolve();
      });
    });
  });

  after(() => {
    if (server) server.close();
  });

  test('GET /api/health returns healthy compiler status', async () => {
    const res = await request('/api/health');
    assert.equal(res.status, 200);
    assert.equal(res.json.ok, true);
    assert.equal(res.json.languageName, 'J--');
    assert.equal(res.json.compilerExists, true);
    assert.equal(res.json.compilerRunnable, true);
    assert.ok(Array.isArray(res.json.availablePhases));
  });

  test('GET /api/cases returns case library with sources', async () => {
    const res = await request('/api/cases');
    assert.equal(res.status, 200);
    assert.ok(Array.isArray(res.json));
    assert.equal(res.json.length, 26);
    assert.ok(res.json[0].source && res.json[0].source.length > 0);
  });

  test('POST /api/pipeline with valid program passes every phase', async () => {
    const source = `num count = 3;
until (count equals 0) {
    show(count);
    count--;
}
show(0);
`;
    const res = await request('/api/pipeline', {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: { source, inputs: '' }
    });

    assert.equal(res.status, 200);
    assert.equal(res.json.passed, true);
    assert.ok(Array.isArray(res.json.phases));
    for (const phase of res.json.phases) {
      assert.equal(phase.status, 'passed', `Phase ${phase.id} should pass`);
    }
  });

  test('POST /api/pipeline with logic error (LV10) fails at logic verifier and stops later phases', async () => {
    const source = `num a = 10;
num b = 0;
num c = a / b;
show(c);
`;
    const res = await request('/api/pipeline', {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: { source }
    });

    assert.equal(res.status, 200);
    assert.equal(res.json.passed, false);
    const phases = res.json.phases;
    const logicPhase = phases.find(p => p.id === 'logic');
    assert.ok(logicPhase);
    assert.equal(logicPhase.status, 'failed');
    assert.ok(logicPhase.codes.includes('LV10'));

    const laterPhases = phases.slice(phases.findIndex(p => p.id === 'logic') + 1);
    for (const p of laterPhases) {
      assert.equal(p.status, 'not_reached', `Phase ${p.id} should be not_reached`);
    }
  });

  test('POST /api/pipeline with syntax error fails early and halts', async () => {
    const source = `check (true) {
    show(1);
`;
    const res = await request('/api/pipeline', {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: { source }
    });

    assert.equal(res.status, 200);
    assert.equal(res.json.passed, false);
    const phases = res.json.phases;
    const parserPhase = phases.find(p => p.id === 'parser');
    assert.ok(parserPhase);
    assert.equal(parserPhase.status, 'failed');

    const laterPhases = phases.slice(phases.findIndex(p => p.id === 'parser') + 1);
    for (const p of laterPhases) {
      assert.equal(p.status, 'not_reached');
    }
  });

  test('POST /api/run rejects oversized source (> 200 KB)', async () => {
    const bigSource = 'num x = 0;\n' + ' '.repeat(210 * 1024);
    const res = await request('/api/run', {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: { source: bigSource, mode: 'verify' }
    });

    assert.equal(res.status, 413);
    assert.ok(res.json.error.includes('200 KB limit'));
  });

  test('Direct runner kills execution and flags timedOut on timeout', async () => {
    const config = loadConfig();
    const source = `num count = 100000;
until (count equals 0) {
    count--;
}
show(count);
`;
    // Set a tiny timeout (1ms) to verify that process killing and timedOut flag work as required
    const result = await runCompiler({
      compilerPath: config.resolvedCompilerPath,
      fileExtension: config.fileExtension,
      flags: ['--verify'],
      source,
      timeoutMs: 1
    });

    assert.equal(result.timedOut, true);
  });
});

