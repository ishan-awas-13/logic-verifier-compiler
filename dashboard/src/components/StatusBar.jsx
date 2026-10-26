import React from 'react';

export function StatusBar({ exitCode, durationMs, summary, languageName = 'J--' }) {
  return (
    <footer className="w-full h-7 border-t border-[var(--border-subtle)] bg-[var(--surface-1)] px-4 flex items-center justify-between text-xs font-mono text-[var(--text-secondary)] select-none">
      <div className="flex items-center gap-6">
        <div>
          EXIT:{' '}
          <span className="text-black font-bold">
            {exitCode !== null && exitCode !== undefined ? exitCode : '-'}
          </span>
        </div>
        <div>
          DURATION:{' '}
          <span className="text-black font-bold">
            {durationMs !== null && durationMs !== undefined ? `${durationMs}ms` : '-'}
          </span>
        </div>
        {summary && (
          <div className="text-black font-medium truncate max-w-xl">
            {summary}
          </div>
        )}
      </div>

      <div className="flex items-center gap-4 text-[var(--text-muted)] text-[11px]">
        <span>J-- 7-Stage Compiler + Logic Verifier</span>
        <span>127.0.0.1:4000</span>
      </div>
    </footer>
  );
}
