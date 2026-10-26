import React, { useEffect, useRef, useState, useImperativeHandle, forwardRef } from 'react';
import { EditorState, StateEffect, StateField } from '@codemirror/state';
import { EditorView, lineNumbers, gutter, highlightActiveLineGutter, Decoration } from '@codemirror/view';
import { defaultKeymap, history, historyKeymap } from '@codemirror/commands';
import { keymap } from '@codemirror/view';
import { syntaxHighlighting } from '@codemirror/language';
import { cmmLanguage, cmmHighlightStyle, cmmMonochromeTheme } from '../cmmHighlight.js';

// Effects for diagnostics
const setDiagnosticsEffect = StateEffect.define();
const flashLineEffect = StateEffect.define();

export const EditorArea = forwardRef(function EditorArea({
  tabs,
  activeTabId,
  onSelectTab,
  onCloseTab,
  onNewTab,
  activeCase,
  activeSource,
  onChangeSource,
  inputs,
  onChangeInputs,
  onRunPipeline,
  onRunQuickMode,
  quickModes = [],
  selectedQuickMode,
  onSelectQuickMode,
  onResetCase,
  isRunning,
  diagnostics = [],
  pipelineResult,
  fontSizeOffset = 0
}, ref) {
  const editorParentRef = useRef(null);
  const viewRef = useRef(null);
  const lastTabIdRef = useRef(activeTabId);
  const onChangeSourceRef = useRef(onChangeSource);
  const [showInputs, setShowInputs] = useState(false);
  const [copied, setCopied] = useState(false);

  // Keep latest onChangeSource in ref so CodeMirror updateListener never calls a stale closure
  useEffect(() => {
    onChangeSourceRef.current = onChangeSource;
  }, [onChangeSource]);

  // Initialize CodeMirror 6 View
  useEffect(() => {
    if (!editorParentRef.current) return;

    // Line decorations for diagnostic error/warning lines (soft transparent tint)
    const errorLineMarker = Decoration.line({
      attributes: { class: 'cm-error-line' }
    });

    const warningLineMarker = Decoration.line({
      attributes: { class: 'cm-warning-line' }
    });

    const flashLineMarker = Decoration.line({
      attributes: { class: 'flash-highlight' }
    });

    const diagnosticDecorationsField = StateField.define({
      create() {
        return Decoration.none;
      },
      update(decorations, tr) {
        decorations = decorations.map(tr.changes);
        for (const effect of tr.effects) {
          if (effect.is(setDiagnosticsEffect)) {
            const diags = effect.value;
            const decos = [];
            for (const d of diags) {
              if (d.line > 0 && d.line <= tr.state.doc.lines) {
                const linePos = tr.state.doc.line(d.line).from;
                const marker = d.severity === 'warning' ? warningLineMarker : errorLineMarker;
                decos.push(marker.range(linePos));
              }
            }
            decorations = Decoration.set(decos, true);
          } else if (effect.is(flashLineEffect)) {
            const lineNum = effect.value;
            if (lineNum > 0 && lineNum <= tr.state.doc.lines) {
              const linePos = tr.state.doc.line(lineNum).from;
              decorations = Decoration.set([flashLineMarker.range(linePos)], true);
            }
          }
        }
        return decorations;
      },
      provide: f => EditorView.decorations.from(f)
    });

    const startState = EditorState.create({
      doc: activeSource,
      extensions: [
        lineNumbers(),
        highlightActiveLineGutter(),
        history(),
        keymap.of([
          ...defaultKeymap,
          ...historyKeymap,
          {
            key: 'Mod-Enter',
            run: () => {
              onRunPipeline();
              return true;
            }
          }
        ]),
        cmmLanguage,
        syntaxHighlighting(cmmHighlightStyle),
        cmmMonochromeTheme,
        diagnosticDecorationsField,
        EditorView.updateListener.of(update => {
          if (update.docChanged) {
            const text = update.state.doc.toString();
            onChangeSourceRef.current?.(text);
          }
        })
      ]
    });

    const view = new EditorView({
      state: startState,
      parent: editorParentRef.current
    });
    viewRef.current = view;

    return () => {
      view.destroy();
      viewRef.current = null;
    };
  }, []);

  // Sync external activeSource changes to CodeMirror ONLY when switching tabs
  useEffect(() => {
    const view = viewRef.current;
    if (!view) return;

    if (lastTabIdRef.current !== activeTabId) {
      lastTabIdRef.current = activeTabId;
      const currentDoc = view.state.doc.toString();
      if (currentDoc !== activeSource) {
        view.dispatch({
          changes: { from: 0, to: currentDoc.length, insert: activeSource || '' }
        });
      }
    }
  }, [activeTabId, activeSource]);

  // Sync diagnostics onto editor
  useEffect(() => {
    const view = viewRef.current;
    if (!view) return;
    view.dispatch({
      effects: setDiagnosticsEffect.of(diagnostics)
    });
  }, [diagnostics]);

  // Expose imperative methods to parent
  useImperativeHandle(ref, () => ({
    getSource() {
      if (viewRef.current) {
        return viewRef.current.state.doc.toString();
      }
      return activeSource || '';
    },
    setSource(newSource) {
      const view = viewRef.current;
      if (!view) return;
      const currentDoc = view.state.doc.toString();
      view.dispatch({
        changes: { from: 0, to: currentDoc.length, insert: newSource || '' }
      });
    },
    jumpToLineCol(line, col) {
      const view = viewRef.current;
      if (!view) return;
      if (line < 1 || line > view.state.doc.lines) return;

      const lineInfo = view.state.doc.line(line);
      const targetPos = Math.min(lineInfo.from + Math.max(0, col - 1), lineInfo.to);

      view.dispatch({
        selection: { anchor: targetPos, head: targetPos },
        scrollIntoView: true,
        effects: flashLineEffect.of(line)
      });
      view.focus();
    }
  }));

  const handleCopy = () => {
    const textToCopy = viewRef.current ? viewRef.current.state.doc.toString() : activeSource;
    navigator.clipboard.writeText(textToCopy);
    setCopied(true);
    setTimeout(() => setCopied(false), 1500);
  };

  // Evaluation of Expected vs Actual Match
  let matchInfo = null;
  const currentTab = tabs.find(t => t.id === activeTabId);
  const isModified = currentTab?.isModified;

  if (activeCase && pipelineResult) {
    const expCodes = (activeCase.expectedCodes || []).slice().sort();
    const actCodes = (pipelineResult.codes || []).slice().sort();
    const exitMatch = pipelineResult.exitCode === activeCase.expectedExit;
    const codesMatch =
      expCodes.length === actCodes.length &&
      expCodes.every((c, i) => c === actCodes[i]);
    const isMatch = exitMatch && codesMatch;

    matchInfo = {
      isMatch,
      expected: expCodes.length > 0 ? expCodes.join(', ') : 'No flaws',
      actual: actCodes.length > 0 ? actCodes.join(', ') : (pipelineResult.exitCode === 0 ? 'Passed' : 'Syntax/Sema Error')
    };
  }

  const baseFontSize = 16 + fontSizeOffset;

  return (
    <div className="flex-1 h-full flex flex-col bg-white overflow-hidden select-none">
      {/* Editor Tabs Strip */}
      <div className="flex items-center justify-between border-b border-[var(--border-subtle)] bg-[var(--surface-2)] px-2 overflow-x-auto">
        <div className="flex items-center space-x-1 pt-1.5">
          {tabs.map(tab => {
            const isActive = tab.id === activeTabId;
            return (
              <div
                key={tab.id}
                onClick={() => onSelectTab(tab.id)}
                className={`flex items-center gap-2 px-3 py-1.5 border-t border-l border-r rounded-t text-xs font-mono cursor-pointer transition-all ${
                  isActive
                    ? 'border-t-2 border-t-black border-l-[var(--border-medium)] border-r-[var(--border-medium)] bg-white text-black font-bold shadow-sm'
                    : 'border-transparent bg-transparent text-[var(--text-secondary)] hover:text-black hover:bg-white/50'
                }`}
              >
                <span>{tab.title}</span>
                <button
                  onClick={e => {
                    e.stopPropagation();
                    onCloseTab(tab.id);
                  }}
                  className="hover:text-black text-[var(--text-muted)] text-sm ml-1 font-bold"
                >
                  ×
                </button>
              </div>
            );
          })}
          <button
            onClick={onNewTab}
            className="px-2.5 py-1 text-xs font-mono font-bold text-[var(--text-secondary)] hover:text-black hover:bg-white rounded"
            title="Create New Blank Tab"
          >
            + NEW
          </button>
        </div>

        {/* Toggle Inputs Section Button */}
        <button
          onClick={() => setShowInputs(!showInputs)}
          className="text-[11px] font-mono font-bold uppercase tracking-wider text-[var(--text-secondary)] hover:text-black px-2 py-1 rounded border border-[var(--border-medium)] bg-white hover:bg-[var(--surface-2)]"
        >
          {showInputs ? 'HIDE INPUTS ▲' : 'INPUTS (STDIN) ▼'}
        </button>
      </div>

      {/* Active Case Banner */}
      {activeCase && (
        <div className="border-b border-[var(--border-subtle)] bg-[var(--surface-1)] p-3 space-y-1.5">
          <div className="flex items-center justify-between">
            <div className="flex items-center gap-2.5">
              <span className="font-mono text-xs font-bold px-2 py-0.5 rounded border border-black bg-black text-white">
                {activeCase.code}
              </span>
              <span className="font-extrabold text-sm font-sans text-black">
                {activeCase.title}
              </span>
              <span className="text-xs text-[var(--text-muted)] font-mono">
                ({activeCase.category})
              </span>
            </div>

            {/* Expected Summary */}
            <div className="font-mono text-xs text-[var(--text-secondary)] border border-[var(--border-medium)] rounded bg-[var(--surface-2)] px-2 py-0.5">
              EXPECTED:{' '}
              <span className="text-black font-bold">
                {activeCase.expectedCodes?.length > 0 ? activeCase.expectedCodes.join(', ') : 'NO FLAWS'} (exit {activeCase.expectedExit})
              </span>
            </div>
          </div>

          <p className="text-xs text-[var(--text-secondary)] font-sans m-0">
            {activeCase.description}
          </p>

          {/* Actual vs Expected comparison line (only when unedited) */}
          {matchInfo && (
            <div className="mt-2 pt-2 border-t border-[var(--border-subtle)] flex items-center justify-between font-mono text-xs">
              <div className="flex items-center gap-4">
                <span>EXPECTED: <strong className="text-black">{matchInfo.expected}</strong></span>
                <span>|</span>
                <span>ACTUAL: <strong className="text-black">{matchInfo.actual}</strong></span>
              </div>
              <div className="flex items-center gap-2">
                {matchInfo.isMatch ? (
                  <span className="pill-ok">
                    <span>MATCH</span>
                    <span>✓</span>
                  </span>
                ) : (
                  <span className="pill-error">
                    <span>MISMATCH</span>
                    <span>■</span>
                  </span>
                )}
              </div>
            </div>
          )}
        </div>
      )}

      {/* Editor & Collapsible Inputs Row */}
      <div className="flex-1 flex overflow-hidden">
        {/* CodeMirror Editor Container */}
        <div
          ref={editorParentRef}
          className="flex-1 h-full overflow-hidden bg-white"
          style={{ fontSize: `${baseFontSize}px` }}
        />

        {/* Inputs (stdin) Side Drawer */}
        {showInputs && (
          <div className="w-64 border-l border-[var(--border-subtle)] bg-[var(--surface-2)] flex flex-col p-3">
            <div className="font-sans font-bold uppercase text-[10px] tracking-widest text-[var(--text-muted)] mb-1">
              Program Inputs (one per line)
            </div>
            <textarea
              value={inputs}
              onChange={e => onChangeInputs(e.target.value)}
              placeholder="e.g.&#10;42&#10;3.14&#10;true&#10;hello"
              className="flex-1 w-full bg-white border border-[var(--border-medium)] p-2 text-black font-mono text-xs resize-none outline-none focus:border-black rounded"
            />
          </div>
        )}
      </div>

      {/* Action Bar */}
      <div className="h-12 border-t border-[var(--border-subtle)] bg-[var(--surface-1)] px-4 flex items-center justify-between shadow-sm">
        <div className="flex items-center gap-3">
          <button
            onClick={onRunPipeline}
            disabled={isRunning}
            className="btn-primary"
            title="Run full pipeline in order (Ctrl+Enter)"
          >
            {isRunning ? 'RUNNING...' : 'RUN PIPELINE (Ctrl+Enter)'}
          </button>

          {/* Quick Mode Select */}
          <div className="flex items-center border border-[var(--border-medium)] rounded overflow-hidden bg-white">
            <select
              value={selectedQuickMode}
              onChange={e => onSelectQuickMode(e.target.value)}
              disabled={isRunning}
              className="bg-transparent text-black font-mono text-xs px-2.5 py-1.5 outline-none cursor-pointer border-r border-[var(--border-medium)]"
            >
              {quickModes.map(m => (
                <option key={m.id} value={m.id} className="bg-white text-black">
                  {m.label}
                </option>
              ))}
            </select>
            <button
              onClick={() => onRunQuickMode(selectedQuickMode)}
              disabled={isRunning}
              className="px-3 py-1.5 bg-white hover:bg-[var(--surface-2)] text-black font-mono text-xs font-bold uppercase"
              title="Execute selected mode directly"
            >
              RUN QUICK
            </button>
          </div>
        </div>

        <div className="flex items-center gap-2">
          {activeCase && (
            <button
              onClick={onResetCase}
              disabled={isRunning}
              className="btn-secondary"
              title="Reset code to original case contents"
            >
              RESET
            </button>
          )}

          <button
            onClick={handleCopy}
            className="btn-secondary"
            title="Copy source code to clipboard"
          >
            {copied ? 'COPIED!' : 'COPY'}
          </button>
        </div>
      </div>
    </div>
  );
});
