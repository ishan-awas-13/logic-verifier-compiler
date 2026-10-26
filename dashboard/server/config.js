import fs from 'node:fs';
import path from 'node:path';
import { fileURLToPath } from 'node:url';

const __filename = fileURLToPath(import.meta.url);
const __dirname = path.dirname(__filename);
const rootDir = path.resolve(__dirname, '..');

const configPath = path.join(rootDir, 'dashboard.config.json');

export function loadConfig() {
  if (!fs.existsSync(configPath)) {
    throw new Error(`Config file not found at: ${configPath}`);
  }
  const raw = fs.readFileSync(configPath, 'utf8');
  const config = JSON.parse(raw);

  const envCompiler = process.env.COMPILER_PATH;
  let relCompiler = envCompiler || config.compilerPath;
  let resolved = path.isAbsolute(relCompiler)
    ? relCompiler
    : path.resolve(rootDir, relCompiler);

  if (!fs.existsSync(resolved)) {
    const candidates = [
      '../build/c--.exe',
      '../build/c--',
      '../build/Release/c--.exe',
      '../C--/build/c--.exe',
      '../../build/c--.exe',
      '../../C--/build/c--.exe'
    ];
    for (const cand of candidates) {
      const p = path.resolve(rootDir, cand);
      if (fs.existsSync(p)) {
        resolved = p;
        break;
      }
    }
  }

  config.resolvedCompilerPath = resolved;
  return config;
}

