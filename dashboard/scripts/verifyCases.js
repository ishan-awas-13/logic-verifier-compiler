import fs from 'node:fs';
import path from 'node:path';
import { fileURLToPath } from 'node:url';
import { loadConfig } from '../server/config.js';
import { runCompiler } from '../server/runner.js';

const __filename = fileURLToPath(import.meta.url);
const __dirname = path.dirname(__filename);
const rootDir = path.resolve(__dirname, '..');

async function main() {
  console.log('=========================================================================================================');
  console.log('                               VERIFYING ALL C-- DASHBOARD TEST CASES                                    ');
  console.log('=========================================================================================================\n');

  const config = loadConfig();
  const manifestPath = path.join(rootDir, 'cases', 'manifest.json');
  if (!fs.existsSync(manifestPath)) {
    console.error(`Error: Manifest not found at ${manifestPath}`);
    process.exit(1);
  }

  const manifest = JSON.parse(fs.readFileSync(manifestPath, 'utf8'));

  const colId = 25;
  const colExp = 14;
  const colAct = 14;
  const colExitExp = 8;
  const colExitAct = 8;
  const colStatus = 8;
  const colDur = 10;

  const header = [
    'Case ID'.padEnd(colId),
    'Expected LV'.padEnd(colExp),
    'Actual LV'.padEnd(colAct),
    'Exp Exit'.padEnd(colExitExp),
    'Act Exit'.padEnd(colExitAct),
    'Status'.padEnd(colStatus),
    'Duration'.padEnd(colDur)
  ].join(' | ');

  console.log(header);
  console.log('-'.repeat(header.length));

  let failureCount = 0;

  for (const c of manifest) {
    const filePath = path.join(rootDir, c.file);
    if (!fs.existsSync(filePath)) {
      console.log(`${c.id.padEnd(colId)} | MISSING FILE (${c.file})`);
      failureCount++;
      continue;
    }

    const source = fs.readFileSync(filePath, 'utf8');
    const result = await runCompiler({
      compilerPath: config.resolvedCompilerPath,
      fileExtension: config.fileExtension,
      flags: ['--verify'],
      source,
      inputs: c.inputs || null,
      timeoutMs: config.timeoutMs || 10000
    });

    const expectedCodes = (c.expectedCodes || []).slice().sort();
    const actualCodes = (result.codes || []).slice().sort();

    const exitMatch = result.exitCode === c.expectedExit;
    const codesMatch =
      expectedCodes.length === actualCodes.length &&
      expectedCodes.every((code, idx) => code === actualCodes[idx]);

    const passed = exitMatch && codesMatch;
    if (!passed) {
      failureCount++;
    }

    const expCodeStr = expectedCodes.length > 0 ? expectedCodes.join(',') : 'none';
    const actCodeStr = actualCodes.length > 0 ? actualCodes.join(',') : 'none';
    const statusStr = passed ? 'PASS' : 'FAIL';
    const durStr = `${result.durationMs}ms`;

    const row = [
      c.id.padEnd(colId),
      expCodeStr.padEnd(colExp),
      actCodeStr.padEnd(colAct),
      String(c.expectedExit).padEnd(colExitExp),
      String(result.exitCode).padEnd(colExitAct),
      statusStr.padEnd(colStatus),
      durStr.padEnd(colDur)
    ].join(' | ');

    console.log(row);

    if (!passed) {
      if (!exitMatch) {
        console.log(`   -> Exit code mismatch: expected ${c.expectedExit}, got ${result.exitCode}`);
      }
      if (!codesMatch) {
        console.log(`   -> Diagnostic code mismatch: expected [${expCodeStr}], got [${actCodeStr}]`);
      }
    }
  }

  console.log('-'.repeat(header.length));
  console.log(`\nTotal cases: ${manifest.length} | Passed: ${manifest.length - failureCount} | Failed: ${failureCount}\n`);

  if (failureCount > 0) {
    console.error(`Verification FAILED: ${failureCount} case(s) mismatched.`);
    process.exit(1);
  } else {
    console.log('All cases passed successfully and match the compiler ground truth!');
    process.exit(0);
  }
}

main().catch(err => {
  console.error('Fatal error during verification:', err);
  process.exit(1);
});

