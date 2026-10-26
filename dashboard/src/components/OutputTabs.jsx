import React, { useState } from 'react';

export function OutputTabs({
  activeTab,
  diagnostics = [],
  summaryLines = [],
  rawStdout = '',
  rawStderr = '',
  phases = [],
  programOutput = null,
  onJumpToLine,
  fontSizeOffset = 0
}) {
  const [copiedRaw, setCopiedRaw] = useState(false);

  const handleCopyRaw = () => {
    const combined = `${rawStdout}\n${rawStderr}`.trim();
    navigator.clipboard.writeText(combined);
    setCopiedRaw(true);
    setTimeout(() => setCopiedRaw(false), 1500);
  };

  const outputFontSize = 15 + fontSizeOffset;
  const activePhase = phases.find(p => p.id === activeTab);

  return (
    <div className="w-full h-full flex flex-col bg-[var(--surface-1)] select-none overflow-hidden">
      {/* Content Area */}
      <div
        className="flex-1 overflow-y-auto p-4 font-mono bg-white"
        style={{ fontSize: `${outputFontSize}px` }}
      >
        {/* DIAGNOSTICS VIEW */}
        {activeTab === 'diagnostics' && (
          <div className="space-y-3">
            <div className="flex items-center justify-between border-b border-[var(--border-subtle)] pb-2 mb-3">
              <span className="font-heading font-extrabold text-xs uppercase tracking-wider text-black">
                Diagnostic Findings ({diagnostics.length})
              </span>
              <span className="text-xs font-mono text-[var(--text-muted)]">
                Click any card to jump to line in code
              </span>
            </div>

            {diagnostics.length === 0 ? (
              <div className="p-4 border border-[var(--border-medium)] rounded bg-[var(--surface-2)] text-[var(--text-primary)] flex items-center gap-3">
                <span className="pill-ok">
                  <span>OK</span>
                  <span>✓</span>
                </span>
                <span className="font-sans font-medium text-sm">
                  No compiler errors or logic verifier warnings detected. All checks passed.
                </span>
              </div>
            ) : (
              diagnostics.map((diag, idx) => {
                const isError = diag.severity === 'error';
                return (
                  <div
                    key={idx}
                    onClick={() => onJumpToLine(diag.line, diag.col)}
                    className="p-3.5 border border-[var(--border-medium)] hover:border-black rounded bg-white hover:bg-[var(--surface-2)] cursor-pointer transition-all space-y-2 shadow-sm"
                    title="Click to jump to line in editor"
                  >
                    <div className="flex items-center justify-between">
                      <div className="flex items-center gap-2.5">
                        {isError ? (
                          <span className="pill-error">
                            <span>■ ERROR</span>
                          </span>
                        ) : (
                          <span className="pill-warning">
                            <span>△ WARNING</span>
                          </span>
                        )}
                        {diag.code && (
                          <span className="px-2 py-0.5 border border-black font-bold text-xs bg-[var(--surface-2)] text-black rounded">
                            [{diag.code}]
                          </span>
                        )}
                        <span className="text-black font-bold hover:underline text-xs bg-[var(--surface-3)] px-2 py-0.5 rounded">
                          Line {diag.line}, Col {diag.col}
                        </span>
                      </div>
                      <span className="text-xs font-bold text-black font-sans">
                        Jump to code ↗
                      </span>
                    </div>

                    <div className="text-sm text-black font-mono whitespace-pre-wrap pl-1 font-medium leading-relaxed">
                      {diag.message}
                    </div>
                  </div>
                );
              })
            )}

            {/* Summary Lines Block */}
            {summaryLines.length > 0 && (
              <div className="mt-4 pt-3 border-t border-[var(--border-subtle)] text-xs text-[var(--text-secondary)] space-y-1">
                {summaryLines.map((s, idx) => (
                  <div key={idx} className="font-mono font-medium">
                    {s}
                  </div>
                ))}
              </div>
            )}
          </div>
        )}

        {/* RAW OUTPUT VIEW */}
        {activeTab === 'raw' && (
          <div className="relative space-y-3">
            <div className="flex items-center justify-between border-b border-[var(--border-subtle)] pb-2">
              <span className="font-heading font-extrabold text-xs uppercase tracking-wider text-black">
                Raw Compiler Output (stdout & stderr)
              </span>
              <button onClick={handleCopyRaw} className="btn-secondary !py-1 text-xs">
                {copiedRaw ? 'COPIED!' : 'COPY RAW'}
              </button>
            </div>
            <pre className="m-0 p-3.5 bg-[var(--surface-2)] border border-[var(--border-medium)] rounded text-black whitespace-pre-wrap font-mono leading-relaxed">
              {rawStdout || rawStderr
                ? `${rawStdout}${rawStderr ? `\n--- STDERR ---\n${rawStderr}` : ''}`
                : '[No compiler output captured]'}
            </pre>
          </div>
        )}

        {/* PROGRAM RUN VIEW */}
        {activeTab === 'program' && programOutput && (
          <div className="space-y-3">
            <div className="flex items-center justify-between border-b border-[var(--border-subtle)] pb-2">
              <span className="font-heading font-extrabold text-xs uppercase tracking-wider text-black">
                Program Execution Output
              </span>
              <div className="flex items-center gap-4 text-xs font-mono font-bold">
                <span>EXIT CODE: <strong className="text-black bg-[var(--surface-2)] px-2 py-0.5 rounded">{programOutput.exitCode}</strong></span>
                <span>DURATION: <strong className="text-black bg-[var(--surface-2)] px-2 py-0.5 rounded">{programOutput.durationMs}ms</strong></span>
              </div>
            </div>
            <pre className="m-0 p-3.5 bg-[var(--surface-2)] border border-[var(--border-medium)] rounded text-black whitespace-pre-wrap font-mono text-base font-semibold leading-relaxed">
              {programOutput.stdout || '[Program terminated with no output]'}
            </pre>
            {programOutput.stderr && (
              <div className="p-3 border border-black bg-red-50 text-red-900 rounded">
                <div className="font-bold mb-1 text-xs uppercase font-sans">Runtime Error:</div>
                <pre className="m-0 whitespace-pre-wrap font-mono text-xs">{programOutput.stderr}</pre>
              </div>
            )}
          </div>
        )}

        {/* PHASE IR & OUTPUT VIEW */}
        {activePhase && (
          <div className="space-y-3">
            <div className="flex items-center justify-between border-b border-[var(--border-subtle)] pb-2">
              <span className="font-heading font-extrabold text-xs uppercase tracking-wider text-black">
                {activePhase.label} Stage Intermediate Output
              </span>
              <div className="flex items-center gap-3 text-xs font-mono">
                <span className="font-bold">
                  STATUS:{' '}
                  <span className={`px-2 py-0.5 rounded ${activePhase.status === 'passed' ? 'bg-black text-white' : 'bg-red-600 text-white'}`}>
                    {activePhase.status.toUpperCase()}
                  </span>
                </span>
                {activePhase.exitCode !== null && (
                  <span>EXIT: <strong>{activePhase.exitCode}</strong></span>
                )}
                {activePhase.durationMs > 0 && (
                  <span>DURATION: <strong>{activePhase.durationMs}ms</strong></span>
                )}
              </div>
            </div>
            <pre className="m-0 p-3.5 bg-[var(--surface-2)] border border-[var(--border-medium)] rounded text-black whitespace-pre-wrap font-mono leading-relaxed">
              {activePhase.stdout || activePhase.stderr || `[Phase ${activePhase.status}]`}
            </pre>
          </div>
        )}
      </div>
    </div>
  );
}
