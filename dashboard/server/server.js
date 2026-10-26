import fs from 'node:fs';
import path from 'node:path';
import { fileURLToPath } from 'node:url';
import express from 'express';
import cors from 'cors';
import { loadConfig } from './config.js';
import { runCompiler } from './runner.js';

const __filename = fileURLToPath(import.meta.url);
const __dirname = path.dirname(__filename);
const rootDir = path.resolve(__dirname, '..');

const app = express();
app.use(cors());
app.use(express.json({ limit: '500kb' }));

// GET /api/health
app.get('/api/health', async (req, res) => {
  try {
    const currentConfig = loadConfig();
    const exists = fs.existsSync(currentConfig.resolvedCompilerPath);
    let runnable = false;

    if (exists) {
      try {
        const testRun = await runCompiler({
          compilerPath: currentConfig.resolvedCompilerPath,
          fileExtension: currentConfig.fileExtension,
          flags: ['--libs'],
          source: '',
          inputs: null,
          timeoutMs: 3000
        });
        runnable = testRun.exitCode === 0;
      } catch (e) {
        runnable = false;
      }
    }

    res.json({
      ok: exists && runnable,
      compilerExists: exists,
      compilerRunnable: runnable,
      compilerPath: currentConfig.resolvedCompilerPath,
      languageName: currentConfig.languageName,
      fileExtension: currentConfig.fileExtension,
      availablePhases: currentConfig.pipeline.map(p => ({ id: p.id, label: p.label })),
      quickModes: currentConfig.quickModes
    });
  } catch (err) {
    res.status(500).json({ ok: false, error: err.message });
  }
});

// GET /api/cases
app.get('/api/cases', (req, res) => {
  try {
    const manifestPath = path.join(rootDir, 'cases', 'manifest.json');
    if (!fs.existsSync(manifestPath)) {
      return res.status(404).json({ error: 'Manifest not found' });
    }
    const manifest = JSON.parse(fs.readFileSync(manifestPath, 'utf8'));

    // Attach file source to each case
    const enriched = manifest.map(c => {
      let source = '';
      const filePath = path.resolve(rootDir, c.file);
      if (fs.existsSync(filePath)) {
        source = fs.readFileSync(filePath, 'utf8');
      }
      return {
        ...c,
        source
      };
    });

    res.json(enriched);
  } catch (err) {
    res.status(500).json({ error: err.message });
  }
});

// POST /api/run
app.post('/api/run', async (req, res) => {
  try {
    const { source, mode, inputs } = req.body;
    if (typeof source !== 'string') {
      return res.status(400).json({ error: 'source is required and must be a string' });
    }

    const currentConfig = loadConfig();
    const modeConfig = currentConfig.quickModes.find(m => m.id === mode);
    if (!modeConfig) {
      return res.status(400).json({ error: `Unknown mode: ${mode}` });
    }

    const result = await runCompiler({
      compilerPath: currentConfig.resolvedCompilerPath,
      fileExtension: currentConfig.fileExtension,
      flags: modeConfig.flags,
      source,
      inputs,
      timeoutMs: currentConfig.timeoutMs || 10000
    });

    res.json(result);
  } catch (err) {
    const status = err.status || 500;
    res.status(status).json({ error: err.message, timedOut: false });
  }
});

// POST /api/pipeline
app.post('/api/pipeline', async (req, res) => {
  try {
    const { source, inputs } = req.body;
    if (typeof source !== 'string') {
      return res.status(400).json({ error: 'source is required and must be a string' });
    }

    const currentConfig = loadConfig();
    const phases = currentConfig.pipeline;
    const results = [];
    let stopped = false;

    const hasLinkFlag = phases.some(p => p.id === 'link');

    for (const phase of phases) {
      if (stopped) {
        results.push({
          id: phase.id,
          label: phase.label,
          status: 'not_reached',
          stdout: '',
          stderr: '',
          exitCode: null,
          durationMs: 0,
          diagnostics: []
        });
        continue;
      }

      const phaseRun = await runCompiler({
        compilerPath: currentConfig.resolvedCompilerPath,
        fileExtension: currentConfig.fileExtension,
        flags: phase.flags,
        source,
        inputs,
        timeoutMs: currentConfig.timeoutMs || 10000
      });

      const passed = phaseRun.exitCode === 0 && !phaseRun.timedOut;
      let phaseStatus = passed ? 'passed' : 'failed';

      // Check special link / logic mapping rule
      if (hasLinkFlag && phase.id === 'logic') {
        const linkCodes = ['LV12', 'LV13', 'LV14'];
        const hasOnlyLinkFlaws = phaseRun.codes.length > 0 && phaseRun.codes.every(c => linkCodes.includes(c));
        if (hasOnlyLinkFlaws) {
          const linkIdx = results.findIndex(r => r.id === 'link');
          if (linkIdx >= 0) {
            results[linkIdx].status = 'failed';
            results[linkIdx].diagnostics = phaseRun.diagnostics;
          }
        }
      }

      results.push({
        id: phase.id,
        label: phase.label,
        status: phaseStatus,
        stdout: phaseRun.stdout,
        stderr: phaseRun.stderr,
        exitCode: phaseRun.exitCode,
        durationMs: phaseRun.durationMs,
        timedOut: phaseRun.timedOut,
        diagnostics: phaseRun.diagnostics,
        summaryLines: phaseRun.summaryLines,
        codes: phaseRun.codes
      });

      if (!passed) {
        stopped = true;
      }
    }

    const failingPhase = results.find(r => r.status === 'failed');
    const lastExecuted = results.filter(r => r.status !== 'not_reached').pop();
    const primaryPhase = failingPhase || lastExecuted;

    res.json({
      passed: !stopped,
      phases: results,
      primaryPhase: primaryPhase ? primaryPhase.id : null,
      diagnostics: primaryPhase ? primaryPhase.diagnostics : [],
      summaryLines: primaryPhase ? primaryPhase.summaryLines : [],
      codes: primaryPhase ? primaryPhase.codes : []
    });
  } catch (err) {
    const status = err.status || 500;
    res.status(status).json({ error: err.message });
  }
});

// Serve frontend build if available
const distPath = path.join(rootDir, 'dist');
if (fs.existsSync(distPath)) {
  app.use(express.static(distPath));
  app.get('*', (req, res, next) => {
    if (req.path.startsWith('/api')) return next();
    res.sendFile(path.join(distPath, 'index.html'));
  });
}

export function startServer(port = 4000) {
  return app.listen(port, '127.0.0.1', () => {
    console.log(`C-- Logic Verifier Dashboard listening on http://127.0.0.1:${port}`);
  });
}

const isDirectRun = process.argv[1] && (
  process.argv[1].endsWith('server.js') ||
  process.argv[1].endsWith('server')
);

if (isDirectRun && process.env.NODE_ENV !== 'test') {
  const PORT = process.env.PORT || 4000;
  startServer(PORT);
}

export { app };

