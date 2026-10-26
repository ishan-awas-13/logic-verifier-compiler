import React, { useState } from 'react';

export function AllCasesModal({ isOpen, onClose, cases = [] }) {
  const [running, setRunning] = useState(false);
  const [results, setResults] = useState([]);
  const [completedCount, setCompletedCount] = useState(0);

  if (!isOpen) return null;

  const runAllCases = async () => {
    setRunning(true);
    setResults([]);
    setCompletedCount(0);

    const testResults = [];
    for (let i = 0; i < cases.length; i++) {
      const c = cases[i];
      let resData = null;
      try {
        const res = await fetch('/api/pipeline', {
          method: 'POST',
          headers: { 'Content-Type': 'application/json' },
          body: JSON.stringify({ source: c.source, inputs: c.inputs || '' })
        });
        resData = await res.json();
      } catch (err) {
        resData = { passed: false, exitCode: -1, codes: [], error: err.message };
      }

      const expectedCodes = (c.expectedCodes || []).slice().sort();
      const actualCodes = (resData.codes || []).slice().sort();
      const exitMatch = resData.exitCode === c.expectedExit || (c.expectedExit === 0 && resData.passed);
      const codesMatch =
        expectedCodes.length === actualCodes.length &&
        expectedCodes.every((code, idx) => code === actualCodes[idx]);

      const isMatch = exitMatch && codesMatch;
      const duration = (resData.phases || []).reduce((acc, p) => acc + (p.durationMs || 0), 0);

      const entry = {
        id: c.id,
        title: c.title,
        code: c.code,
        category: c.category,
        expectedCodes,
        actualCodes,
        expectedExit: c.expectedExit,
        actualExit: resData.exitCode,
        isMatch,
        duration
      };

      testResults.push(entry);
      setResults([...testResults]);
      setCompletedCount(i + 1);
    }

    setRunning(false);
  };

  const passCount = results.filter(r => r.isMatch).length;
  const failCount = results.filter(r => !r.isMatch).length;

  return (
    <div className="fixed inset-0 z-50 bg-black/50 flex flex-col p-6 select-none overflow-hidden animate-fadeIn backdrop-blur-sm">
      <div className="bg-white rounded-lg shadow-2xl flex-1 flex flex-col p-6 overflow-hidden border border-[var(--border-medium)]">
        {/* Header */}
        <div className="flex items-center justify-between border-b border-[var(--border-subtle)] pb-4 mb-4">
          <div>
            <h2 className="font-heading font-black text-2xl text-black m-0 tracking-tight">
              Pre-Demo Verification Matrix
            </h2>
            <p className="text-xs font-mono text-[var(--text-secondary)] m-0 mt-1">
              Executing all 26 test cases through the real C-- compiler pipeline
            </p>
          </div>

          <div className="flex items-center gap-3">
            <button
              onClick={runAllCases}
              disabled={running}
              className="btn-primary"
            >
              {running ? `RUNNING (${completedCount}/${cases.length})...` : 'EXECUTE ALL CASES'}
            </button>
            <button
              onClick={onClose}
              className="btn-secondary"
            >
              CLOSE (ESC)
            </button>
          </div>
        </div>

        {/* Progress Bar & Stats */}
        {results.length > 0 && (
          <div className="flex items-center justify-between bg-[var(--surface-2)] border border-[var(--border-medium)] rounded p-3 mb-4 font-mono text-xs">
            <div className="flex items-center gap-6">
              <span>TOTAL: <strong className="text-black">{results.length} / {cases.length}</strong></span>
              <span>MATCHES: <strong className="text-black">{passCount}</strong></span>
              <span>MISMATCHES: <strong className="text-black">{failCount}</strong></span>
            </div>

            <div className="flex items-center gap-2">
              {failCount === 0 && results.length === cases.length && (
                <span className="pill-ok">
                  <span>ALL VERIFIED 100% GROUND TRUTH</span>
                  <span>✓</span>
                </span>
              )}
            </div>
          </div>
        )}

        {/* Results Table */}
        <div className="flex-1 overflow-y-auto border border-[var(--border-medium)] rounded bg-white">
          <table className="w-full border-collapse font-mono text-xs text-left">
            <thead className="sticky top-0 bg-[var(--surface-2)] border-b border-[var(--border-medium)] text-[var(--text-secondary)] font-bold">
              <tr>
                <th className="p-3">#</th>
                <th className="p-3">CASE</th>
                <th className="p-3">CATEGORY</th>
                <th className="p-3">EXPECTED</th>
                <th className="p-3">ACTUAL</th>
                <th className="p-3">VERIFICATION</th>
                <th className="p-3">DURATION</th>
              </tr>
            </thead>
            <tbody className="divide-y divide-[var(--border-subtle)]">
              {cases.map((c, idx) => {
                const res = results[idx];
                return (
                  <tr key={c.id} className="hover:bg-[var(--surface-2)]">
                    <td className="p-3 text-[var(--text-muted)]">{idx + 1}</td>
                    <td className="p-3">
                      <div className="flex items-center gap-2">
                        <span className="font-bold border border-black rounded px-1.5 py-0.5 text-[10px] bg-black text-white">
                          {c.code}
                        </span>
                        <span className="text-black font-semibold">{c.title}</span>
                      </div>
                    </td>
                    <td className="p-3 text-[var(--text-secondary)]">{c.category}</td>
                    <td className="p-3 text-[var(--text-secondary)]">
                      {c.expectedCodes?.length > 0 ? c.expectedCodes.join(', ') : 'none'} (exit {c.expectedExit})
                    </td>
                    <td className="p-3 text-black font-semibold">
                      {res ? (
                        res.actualCodes?.length > 0 ? res.actualCodes.join(', ') : 'none'
                      ) : (
                        <span className="text-[var(--text-muted)]">-</span>
                      )}
                    </td>
                    <td className="p-3">
                      {res ? (
                        res.isMatch ? (
                          <span className="pill-ok">
                            <span>MATCH</span>
                            <span>✓</span>
                          </span>
                        ) : (
                          <span className="pill-error">
                            <span>MISMATCH</span>
                            <span>■</span>
                          </span>
                        )
                      ) : (
                        <span className="text-[var(--text-muted)]">Pending...</span>
                      )}
                    </td>
                    <td className="p-3 text-[var(--text-secondary)] font-semibold">
                      {res ? `${res.duration}ms` : '-'}
                    </td>
                  </tr>
                );
              })}
            </tbody>
          </table>
        </div>
      </div>
    </div>
  );
}
