// Parses compiler stdout/stderr into structured diagnostics and summary blocks

const DIAG_REGEX = /^line\s+(\d+),\s+col\s+(\d+):\s*(?:(error|warning):\s*)?(?:\[(LV\d+)\]\s*)?(.*)$/i;
const SUMMARY_REGEX = /(?:error\(s\)|warning\(s\)|Pipeline stopped|Libraries loaded|Link map|Symbol Table|Optimization Stats|=== Original Output ===|=== Optimized Output ===)/i;

export function parseDiagnostics(stdout = '', stderr = '') {
  const combined = `${stdout}\n${stderr}`;
  const lines = combined.split(/\r?\n/);

  const diagnostics = [];
  const summaryLines = [];
  let currentDiag = null;

  for (const rawLine of lines) {
    const line = rawLine.trimEnd();
    if (!line.trim()) continue;

    const diagMatch = line.match(DIAG_REGEX);
    if (diagMatch) {
      if (currentDiag) {
        diagnostics.push(currentDiag);
      }
      const [, lineStr, colStr, severityRaw, codeRaw, message] = diagMatch;
      
      let severity = 'error';
      if (severityRaw) {
        severity = severityRaw.toLowerCase();
      } else if (line.toLowerCase().includes('warning')) {
        severity = 'warning';
      }

      currentDiag = {
        line: parseInt(lineStr, 10),
        col: parseInt(colStr, 10),
        severity: severity, // 'error' or 'warning'
        code: codeRaw || null,
        message: message.trim(),
        fullText: line
      };
      continue;
    }

    // Check for continuation line (starts with spaces or tabs)
    if (currentDiag && (rawLine.startsWith('  ') || rawLine.startsWith('\t'))) {
      currentDiag.message += `\n${rawLine.trim()}`;
      currentDiag.fullText += `\n${rawLine.trim()}`;
      continue;
    }

    // Otherwise, it might be a summary or informational line
    if (SUMMARY_REGEX.test(line)) {
      summaryLines.push(line.trim());
    } else if (currentDiag) {
      diagnostics.push(currentDiag);
      currentDiag = null;
      summaryLines.push(line.trim());
    } else {
      summaryLines.push(line.trim());
    }
  }

  if (currentDiag) {
    diagnostics.push(currentDiag);
  }

  // Fallback: If no structured line/col diagnostics were parsed but stderr exists and has errors
  if (diagnostics.length === 0 && stderr && stderr.trim().length > 0) {
    const errLines = stderr.split(/\r?\n/).map(l => l.trim()).filter(Boolean);
    for (const el of errLines) {
      if (!SUMMARY_REGEX.test(el)) {
        diagnostics.push({
          line: 1,
          col: 1,
          severity: 'error',
          code: null,
          message: el,
          fullText: el
        });
      }
    }
  }

  return {
    diagnostics,
    summaryLines,
    codes: Array.from(new Set(diagnostics.map(d => d.code).filter(Boolean)))
  };
}
