import React from 'react';

export function Header({
  languageName = 'J--',
  health,
  onRunAllCases,
  fontSizeOffset,
  onChangeFontSize,
  onToggleRail,
  railOpen
}) {
  const isReady = health && health.ok;

  return (
    <header className="w-full h-14 border-b border-[var(--border-subtle)] bg-[var(--surface-1)] px-5 flex items-center justify-between select-none shadow-sm">
      <div className="flex items-center gap-4">
        <button
          onClick={onToggleRail}
          title="Toggle Case Library (Ctrl+/)"
          className="btn-secondary !py-1 !px-2.5 text-xs font-mono font-bold"
        >
          {railOpen ? '◀ CASES' : '▶ CASES'}
        </button>

        <div className="flex items-center gap-3">
          <span className="font-heading font-black text-2xl text-black tracking-tight">
            {languageName}
          </span>
          <span className="text-[var(--border-medium)] text-lg">/</span>
          <span className="font-heading font-black text-xl md:text-2xl text-black tracking-tight">
            Logic Verifier Dashboard
          </span>
        </div>
      </div>

      <div className="flex items-center gap-2.5">
        {/* Compiler Health Pill */}
        <div
          className={`flex items-center gap-2 px-2.5 py-1 border text-xs tracking-wider uppercase font-mono rounded ${
            isReady
              ? 'border-[var(--border-medium)] bg-[var(--surface-2)] text-black font-semibold'
              : 'border-[#EF4444] bg-[#FEF2F2] text-[#B91C1C]'
          }`}
          title={health ? `Path: ${health.compilerPath}` : 'Checking compiler...'}
        >
          <span className={`inline-block w-2 h-2 rounded-full ${isReady ? 'bg-black' : 'bg-red-500'}`}></span>
          <span>COMPILER: {isReady ? 'READY' : 'OFFLINE'}</span>
        </div>

        {/* Font Size Scaling */}
        <div className="flex items-center border border-[var(--border-medium)] rounded overflow-hidden">
          <button
            onClick={() => onChangeFontSize(-1)}
            disabled={fontSizeOffset <= -4}
            title="Decrease Font Size"
            className="px-2.5 py-1 bg-white hover:bg-[var(--surface-2)] text-black font-mono text-xs border-r border-[var(--border-medium)] disabled:opacity-30"
          >
            A-
          </button>
          <button
            onClick={() => onChangeFontSize(1)}
            disabled={fontSizeOffset >= 8}
            title="Increase Font Size"
            className="px-2.5 py-1 bg-white hover:bg-[var(--surface-2)] text-black font-mono text-xs disabled:opacity-30"
          >
            A+
          </button>
        </div>

        {/* Run All Cases Matrix Modal */}
        <button
          onClick={onRunAllCases}
          className="btn-secondary text-xs font-mono font-bold"
          title="Run verification matrix on all test cases"
        >
          ALL CASES
        </button>
      </div>
    </header>
  );
}
