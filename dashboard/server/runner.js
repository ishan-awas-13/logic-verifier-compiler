import fs from 'node:fs';
import path from 'node:path';
import os from 'node:os';
import crypto from 'node:crypto';
import { spawn } from 'node:child_process';
import { parseDiagnostics } from './diagnosticParser.js';

const MAX_SOURCE_BYTES = 200 * 1024; // 200 KB
const MAX_OUTPUT_BYTES = 1024 * 1024; // 1 MB

export function formatInputs(inputsText) {
  if (!inputsText || typeof inputsText !== 'string') return null;
  const lines = inputsText
    .split(/\r?\n/)
    .map(line => line.trim())
    .filter(line => line.length > 0);
  if (lines.length === 0) return null;
  return lines.join(',');
}

export async function runCompiler({ compilerPath, fileExtension, flags = [], source, inputs, timeoutMs = 10000 }) {
  if (typeof source !== 'string') {
    throw new Error('Source code must be a string.');
  }

  const byteLength = Buffer.byteLength(source, 'utf8');
  if (byteLength > MAX_SOURCE_BYTES) {
    const err = new Error(`Source code exceeds 200 KB limit (${(byteLength / 1024).toFixed(1)} KB).`);
    err.status = 413;
    throw err;
  }

  const randomId = crypto.randomBytes(8).toString('hex');
  const tempDir = path.join(os.tmpdir(), `cmm-dashboard-${Date.now()}-${randomId}`);
  fs.mkdirSync(tempDir, { recursive: true });

  const tempFilePath = path.join(tempDir, `program${fileExtension}`);
  fs.writeFileSync(tempFilePath, source, 'utf8');

  const args = [tempFilePath, ...flags];
  const formattedInputs = formatInputs(inputs);
  if (formattedInputs !== null) {
    args.push('--inputs', formattedInputs);
  }

  const startTime = Date.now();
  let timedOut = false;
  let stdout = '';
  let stderr = '';
  let stdoutBytes = 0;
  let stderrBytes = 0;

  try {
    const exitCode = await new Promise((resolve, reject) => {
      let proc;
      try {
        proc = spawn(compilerPath, args, {
          windowsHide: true,
          stdio: ['ignore', 'pipe', 'pipe']
        });
      } catch (err) {
        return reject(err);
      }

      const timer = setTimeout(() => {
        timedOut = true;
        try {
          proc.kill('SIGKILL');
        } catch (_) {}
      }, timeoutMs);

      proc.stdout.on('data', chunk => {
        if (stdoutBytes < MAX_OUTPUT_BYTES) {
          const remaining = MAX_OUTPUT_BYTES - stdoutBytes;
          const slice = chunk.slice(0, remaining);
          stdout += slice.toString('utf8');
          stdoutBytes += chunk.length;
          if (stdoutBytes >= MAX_OUTPUT_BYTES && !stdout.endsWith('\n[Output capped at 1 MB]\n')) {
            stdout += '\n[Output capped at 1 MB]\n';
          }
        }
      });

      proc.stderr.on('data', chunk => {
        if (stderrBytes < MAX_OUTPUT_BYTES) {
          const remaining = MAX_OUTPUT_BYTES - stderrBytes;
          const slice = chunk.slice(0, remaining);
          stderr += slice.toString('utf8');
          stderrBytes += chunk.length;
          if (stderrBytes >= MAX_OUTPUT_BYTES && !stderr.endsWith('\n[Error output capped at 1 MB]\n')) {
            stderr += '\n[Error output capped at 1 MB]\n';
          }
        }
      });

      proc.on('error', err => {
        clearTimeout(timer);
        reject(err);
      });

      proc.on('close', code => {
        clearTimeout(timer);
        resolve(code ?? (timedOut ? -1 : 1));
      });
    });

    const durationMs = Date.now() - startTime;
    const parsed = parseDiagnostics(stdout, stderr);

    return {
      stdout,
      stderr,
      exitCode,
      durationMs,
      timedOut,
      diagnostics: parsed.diagnostics,
      summaryLines: parsed.summaryLines,
      codes: parsed.codes
    };
  } finally {
    // Clean up temp directory and files safely
    try {
      fs.rmSync(tempDir, { recursive: true, force: true });
    } catch (_) {}
  }
}

