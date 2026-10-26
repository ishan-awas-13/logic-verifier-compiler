import React, { useState } from 'react';

export function CaseRail({ cases = [], activeCaseId, onSelectCase, isOpen }) {
  const [filterText, setFilterText] = useState('');

  if (!isOpen) return null;

  const categories = ['Earlier Phases', 'Logic Flaws', 'Load and Link', 'Valid Programs'];

  const filteredCases = cases.filter(c => {
    const q = filterText.toLowerCase();
    return (
      c.title.toLowerCase().includes(q) ||
      c.code.toLowerCase().includes(q) ||
      c.category.toLowerCase().includes(q) ||
      (c.description && c.description.toLowerCase().includes(q))
    );
  });

  return (
    <aside className="w-80 h-full border-r border-[var(--border-subtle)] bg-[var(--surface-1)] flex flex-col select-none flex-shrink-0">
      {/* Rail Header */}
      <div className="p-3 border-b border-[var(--border-subtle)] bg-[var(--surface-1)]">
        <div className="flex items-center justify-between mb-2">
          <span className="font-heading font-black text-base text-black tracking-tight">
            Case Library
          </span>
          <span className="font-mono text-xs text-[var(--text-muted)] bg-[var(--surface-2)] px-1.5 py-0.5 rounded border border-[var(--border-subtle)]">
            {cases.length}
          </span>
        </div>
        <input
          type="text"
          placeholder="FILTER TEST CASES..."
          value={filterText}
          onChange={e => setFilterText(e.target.value)}
          className="w-full bg-[var(--surface-2)] border border-[var(--border-medium)] focus:border-black text-black font-mono text-xs px-2.5 py-1.5 outline-none rounded transition-colors"
        />
      </div>

      {/* Grouped Cases */}
      <div className="flex-1 overflow-y-auto p-2 space-y-3 bg-[var(--bg-page)]">
        {categories.map(category => {
          const groupCases = filteredCases.filter(c => c.category === category);
          if (groupCases.length === 0) return null;

          return (
            <div key={category} className="space-y-1">
              <div className="font-sans font-bold uppercase text-[10px] tracking-widest text-[var(--text-muted)] px-2 py-1">
                {category} ({groupCases.length})
              </div>
              <div className="space-y-1">
                {groupCases.map(c => {
                  const isActive = c.id === activeCaseId;
                  return (
                    <button
                      key={c.id}
                      onClick={() => onSelectCase(c)}
                      className={`w-full text-left p-2 border rounded flex items-center gap-2.5 transition-all ${
                        isActive
                          ? 'border-black bg-white text-black font-semibold shadow-sm'
                          : 'border-transparent hover:border-[var(--border-medium)] text-[var(--text-secondary)] hover:text-black hover:bg-white bg-transparent'
                      }`}
                    >
                      {/* Code Badge */}
                      <span
                        className={`font-mono text-[10px] font-bold px-1.5 py-0.5 rounded border flex-shrink-0 ${
                          c.code === 'VALID' || c.code === 'SAFE'
                            ? 'border-black bg-transparent text-black'
                            : isActive
                            ? 'border-black bg-black text-white'
                            : 'border-[var(--border-medium)] bg-[var(--surface-2)] text-black'
                        }`}
                      >
                        {c.code}
                      </span>

                      {/* Title */}
                      <span className="text-xs truncate font-sans flex-1">
                        {c.title}
                      </span>
                    </button>
                  );
                })}
              </div>
            </div>
          );
        })}
      </div>
    </aside>
  );
}
