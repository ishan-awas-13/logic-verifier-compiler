import React from 'react';

export function PipelineStrip({
  phases = [],
  activeTab,
  onSelectTab,
  diagnosticsCount = 0,
  hasProgramOutput = false
}) {
  return (
    <div className="w-full border-b border-[var(--border-subtle)] bg-[var(--surface-1)] px-4 py-2.5 select-none flex items-center justify-between gap-4 shadow-sm">
      {/* Left: Section Label & Interactive Pipeline Stages */}
      <div className="flex items-center gap-3 overflow-x-auto py-0.5">
        <span className="font-heading font-extrabold text-xs uppercase tracking-wider text-black whitespace-nowrap">
          Pipeline Flow:
        </span>

        <div className="flex items-center">
          {phases.map((phase, idx) => {
            const isPassed = phase.status === 'passed';
            const isFailed = phase.status === 'failed';
            const isSelected = activeTab === phase.id;
            const isLast = idx === phases.length - 1;

            let boxClass = 'border border-dashed border-[var(--border-medium)] text-[var(--text-muted)] bg-transparent hover:border-black hover:text-black';
            if (isPassed) {
              boxClass = isSelected
                ? 'border-2 border-black bg-white text-black font-extrabold ring-2 ring-black/20 shadow-sm'
                : 'border border-[var(--border-medium)] bg-[var(--surface-2)] text-black font-bold hover:border-black';
            } else if (isFailed) {
              boxClass = isSelected
                ? 'border-2 border-black bg-black text-white font-extrabold ring-2 ring-black/30 shadow-sm'
                : 'border-2 border-black bg-black text-white font-bold';
            }

            return (
              <React.Fragment key={phase.id}>
                {/* Phase Stage Box */}
                <button
                  onClick={() => onSelectTab(phase.id)}
                  className={`flex-shrink-0 px-2.5 py-1 text-xs font-mono uppercase tracking-wider rounded transition-all cursor-pointer ${boxClass}`}
                  title={`View ${phase.label} IR & Output (${phase.status || 'not reached'})`}
                >
                  <div className="flex items-center gap-1.5">
                    <span>{phase.label}</span>
                    {isPassed && <span className="font-bold">✓</span>}
                    {isFailed && <span className="font-bold">⊘</span>}
                  </div>
                </button>

                {/* Connector Arrow */}
                {!isLast && (
                  <div className="flex items-center px-1 flex-shrink-0 font-mono text-xs">
                    {isFailed ? (
                      <span className="text-black font-black">⊘-</span>
                    ) : isPassed ? (
                      <span className="text-black font-bold">→</span>
                    ) : (
                      <span className="text-[var(--text-muted)]">┄</span>
                    )}
                  </div>
                )}
              </React.Fragment>
            );
          })}
        </div>
      </div>

      {/* Right Corner: Separate Output Selector Controls (Diagnostics & Raw Output) */}
      <div className="flex items-center gap-2 flex-shrink-0 pl-3 border-l border-[var(--border-subtle)]">
        {/* Diagnostics Button */}
        <button
          onClick={() => onSelectTab('diagnostics')}
          className={`flex items-center gap-1.5 px-3 py-1.5 text-xs font-mono font-bold rounded border transition-all ${
            activeTab === 'diagnostics'
              ? 'bg-black text-white border-black shadow-sm'
              : 'bg-white text-black border-[var(--border-medium)] hover:border-black hover:bg-[var(--surface-2)]'
          }`}
          title="Show compiler diagnostic findings and logic errors"
        >
          <span>DIAGNOSTICS</span>
          {diagnosticsCount > 0 && (
            <span
              className={`px-1.5 py-0.2 rounded-full text-[10px] font-black ${
                activeTab === 'diagnostics' ? 'bg-white text-black' : 'bg-black text-white'
              }`}
            >
              {diagnosticsCount}
            </span>
          )}
        </button>

        {/* Raw Output Button */}
        <button
          onClick={() => onSelectTab('raw')}
          className={`px-3 py-1.5 text-xs font-mono font-bold rounded border transition-all ${
            activeTab === 'raw'
              ? 'bg-black text-white border-black shadow-sm'
              : 'bg-white text-black border-[var(--border-medium)] hover:border-black hover:bg-[var(--surface-2)]'
          }`}
          title="Show raw compiler stdout and stderr"
        >
          RAW OUTPUT
        </button>

        {/* Program Run Button (if executed) */}
        {hasProgramOutput && (
          <button
            onClick={() => onSelectTab('program')}
            className={`flex items-center gap-1.5 px-3 py-1.5 text-xs font-mono font-bold rounded border transition-all ${
              activeTab === 'program'
                ? 'bg-black text-white border-black shadow-sm'
                : 'bg-white text-black border-[var(--border-medium)] hover:border-black hover:bg-[var(--surface-2)]'
            }`}
            title="Show executed program output and exit status"
          >
            <span>PROGRAM RUN</span>
            <span className="text-[10px]">▶</span>
          </button>
        )}
      </div>
    </div>
  );
}
