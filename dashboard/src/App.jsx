import React, { useState, useEffect, useRef, useCallback } from 'react';
import { Header } from './components/Header.jsx';
import { CaseRail } from './components/CaseRail.jsx';
import { EditorArea } from './components/EditorArea.jsx';
import { PipelineStrip } from './components/PipelineStrip.jsx';
import { OutputTabs } from './components/OutputTabs.jsx';
import { StatusBar } from './components/StatusBar.jsx';
import { AllCasesModal } from './components/AllCasesModal.jsx';

export default function App() {
  const [health, setHealth] = useState(null);
  const [cases, setCases] = useState([]);
  const [tabs, setTabs] = useState([]);
  const [activeTabId, setActiveTabId] = useState('');
  const activeTabIdRef = useRef('');
  activeTabIdRef.current = activeTabId;

  const [railOpen, setRailOpen] = useState(true);
  const [isFullscreen, setIsFullscreen] = useState(false);
  const [fontSizeOffset, setFontSizeOffset] = useState(0);
  const [allCasesOpen, setAllCasesOpen] = useState(false);

  // Fullscreen toggle handler
  const toggleFullScreen = () => {
    if (!document.fullscreenElement) {
      document.documentElement.requestFullscreen().catch(err => {
        console.warn('Fullscreen request failed:', err);
      });
    } else {
      if (document.exitFullscreen) {
        document.exitFullscreen().catch(err => {
          console.warn('Exit fullscreen failed:', err);
        });
      }
    }
  };

  useEffect(() => {
    const handleFsChange = () => {
      setIsFullscreen(!!document.fullscreenElement);
    };
    document.addEventListener('fullscreenchange', handleFsChange);
    return () => document.removeEventListener('fullscreenchange', handleFsChange);
  }, []);

  // Terminal resizable height state
  const savedHeight = parseInt(localStorage.getItem('cmm_terminal_height') || '280', 10);
  const [outputHeight, setOutputHeight] = useState(isNaN(savedHeight) ? 280 : savedHeight);
  const [isDragging, setIsDragging] = useState(false);
  const isDraggingRef = useRef(false);
  const startYRef = useRef(0);
  const startHeightRef = useRef(0);

  // Execution & results state
  const [selectedQuickMode, setSelectedQuickMode] = useState('verify');
  const [isRunning, setIsRunning] = useState(false);
  const [pipelinePhases, setPipelinePhases] = useState([]);
  const [pipelineResult, setPipelineResult] = useState(null);
  const [outputTab, setOutputTab] = useState('diagnostics');
  const [diagnostics, setDiagnostics] = useState([]);
  const [summaryLines, setSummaryLines] = useState([]);
  const [rawStdout, setRawStdout] = useState('');
  const [rawStderr, setRawStderr] = useState('');
  const [programOutput, setProgramOutput] = useState(null);
  const [statusExitCode, setStatusExitCode] = useState(null);
  const [statusDurationMs, setStatusDurationMs] = useState(null);
  const [statusSummary, setStatusSummary] = useState('');

  const editorRef = useRef(null);

  // Load health & cases on mount
  useEffect(() => {
    fetch('/api/health')
      .then(res => res.json())
      .then(data => {
        setHealth(data);
        if (data.availablePhases) {
          setPipelinePhases(data.availablePhases.map(p => ({
            id: p.id,
            label: p.label,
            status: 'not_reached',
            durationMs: 0
          })));
        }
      })
      .catch(err => console.error('Health check failed:', err));

    fetch('/api/cases')
      .then(res => res.json())
      .then(caseList => {
        setCases(caseList);

        const savedTabs = localStorage.getItem('cmm_editor_tabs');
        if (savedTabs) {
          try {
            const parsed = JSON.parse(savedTabs);
            if (Array.isArray(parsed) && parsed.length > 0) {
              setTabs(parsed);
              const savedActive = localStorage.getItem('cmm_active_tab') || parsed[0].id;
              setActiveTabId(savedActive);
              return;
            }
          } catch (_) {}
        }

        if (caseList.length > 0) {
          const first = caseList[0];
          const initialTab = {
            id: first.id,
            title: first.title,
            caseId: first.id,
            source: first.source,
            originalSource: first.source,
            inputs: first.inputs || '',
            isModified: false
          };
          setTabs([initialTab]);
          setActiveTabId(first.id);
        }
      })
      .catch(err => console.error('Failed to load cases:', err));
  }, []);

  // Save tabs to localStorage
  useEffect(() => {
    if (tabs.length > 0) {
      localStorage.setItem('cmm_editor_tabs', JSON.stringify(tabs));
      localStorage.setItem('cmm_active_tab', activeTabId);
    }
  }, [tabs, activeTabId]);

  // Handle draggable vertical splitter
  const handleMouseDown = useCallback(e => {
    e.preventDefault();
    isDraggingRef.current = true;
    setIsDragging(true);
    startYRef.current = e.clientY;
    startHeightRef.current = outputHeight;
    document.body.style.userSelect = 'none';
  }, [outputHeight]);

  useEffect(() => {
    const handleMouseMove = e => {
      if (!isDraggingRef.current) return;
      const deltaY = startYRef.current - e.clientY; // pulling up increases height
      const newHeight = Math.max(120, Math.min(window.innerHeight - 200, startHeightRef.current + deltaY));
      setOutputHeight(newHeight);
      localStorage.setItem('cmm_terminal_height', String(newHeight));
    };

    const handleMouseUp = () => {
      if (isDraggingRef.current) {
        isDraggingRef.current = false;
        setIsDragging(false);
        document.body.style.userSelect = '';
      }
    };

    window.addEventListener('mousemove', handleMouseMove);
    window.addEventListener('mouseup', handleMouseUp);
    return () => {
      window.removeEventListener('mousemove', handleMouseMove);
      window.removeEventListener('mouseup', handleMouseUp);
    };
  }, []);

  // Current active tab & case
  const activeTab = tabs.find(t => t.id === activeTabId) || tabs[0] || null;
  const activeCase = activeTab ? cases.find(c => c.id === activeTab.caseId) : null;

  // Handle case selection from rail
  const handleSelectCase = c => {
    const existing = tabs.find(t => t.caseId === c.id);
    if (existing) {
      setActiveTabId(existing.id);
    } else {
      const newTab = {
        id: c.id,
        title: c.title,
        caseId: c.id,
        source: c.source,
        originalSource: c.source,
        inputs: c.inputs || '',
        isModified: false
      };
      setTabs(prev => [...prev, newTab]);
      setActiveTabId(c.id);
    }
  };

  const handleSelectTab = tabId => {
    setActiveTabId(tabId);
  };

  const handleCloseTab = tabId => {
    setTabs(prev => {
      const remaining = prev.filter(t => t.id !== tabId);
      if (remaining.length === 0) {
        const scratch = {
          id: 'scratch-1',
          title: 'Untitled.jmm',
          caseId: null,
          source: '# Scratchpad program\nnum x = 42;\nshow(x);\n',
          originalSource: '',
          inputs: '',
          isModified: false
        };
        setActiveTabId(scratch.id);
        return [scratch];
      }
      if (activeTabIdRef.current === tabId) {
        setActiveTabId(remaining[remaining.length - 1].id);
      }
      return remaining;
    });
  };

  const handleNewTab = () => {
    const id = `new-${Date.now()}`;
    const newTab = {
      id,
      title: `Program-${tabs.length + 1}.jmm`,
      caseId: null,
      source: '# New J-- program\nnum x = 10;\nshow(x);\n',
      originalSource: '',
      inputs: '',
      isModified: false
    };
    setTabs(prev => [...prev, newTab]);
    setActiveTabId(id);
  };

  const handleChangeSource = newSource => {
    setTabs(prevTabs =>
      prevTabs.map(t => {
        if (t.id === activeTabIdRef.current) {
          return {
            ...t,
            source: newSource,
            isModified: true
          };
        }
        return t;
      })
    );
  };

  const handleChangeInputs = newInputs => {
    setTabs(prevTabs =>
      prevTabs.map(t => {
        if (t.id === activeTabIdRef.current) {
          return { ...t, inputs: newInputs };
        }
        return t;
      })
    );
  };

  const handleResetCase = () => {
    if (!activeTab || !activeCase) return;
    editorRef.current?.setSource(activeCase.source);
    setTabs(prevTabs =>
      prevTabs.map(t => {
        if (t.id === activeTab.id) {
          return {
            ...t,
            source: activeCase.source,
            isModified: false
          };
        }
        return t;
      })
    );
  };

  // Run full compiler pipeline (/api/pipeline)
  const handleRunPipeline = async () => {
    if (isRunning) return;

    // ALWAYS read live source directly from CodeMirror editor!
    const liveSource = editorRef.current?.getSource() ?? (activeTab ? activeTab.source : '');
    const currentInputs = activeTab ? activeTab.inputs : '';

    handleChangeSource(liveSource);

    setIsRunning(true);
    setRawStdout('');
    setRawStderr('');
    setDiagnostics([]);
    setSummaryLines([]);
    setProgramOutput(null);

    try {
      const res = await fetch('/api/pipeline', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({
          source: liveSource,
          inputs: currentInputs
        })
      });
      const data = await res.json();
      setPipelineResult(data);
      setPipelinePhases(data.phases || []);

      // Extract raw outputs and diagnostics
      let allOut = '';
      let allErr = '';
      for (const p of data.phases || []) {
        if (p.stdout) allOut += `[${p.label.toUpperCase()}]\n${p.stdout}\n`;
        if (p.stderr) allErr += `[${p.label.toUpperCase()} STDERR]\n${p.stderr}\n`;
      }
      setRawStdout(allOut);
      setRawStderr(allErr);

      setDiagnostics(data.diagnostics || []);
      setSummaryLines(data.summaryLines || []);

      // Check if run phase executed
      const runPhase = (data.phases || []).find(p => p.id === 'run');
      if (runPhase && runPhase.status === 'passed') {
        setProgramOutput({
          stdout: runPhase.stdout,
          stderr: runPhase.stderr,
          exitCode: runPhase.exitCode,
          durationMs: runPhase.durationMs
        });
      }

      // Status Bar Info
      const failingPhase = (data.phases || []).find(p => p.status === 'failed');
      const lastExecuted = (data.phases || []).filter(p => p.status !== 'not_reached').pop();
      const exitVal = failingPhase ? failingPhase.exitCode : (lastExecuted ? lastExecuted.exitCode : 0);
      const totalDur = (data.phases || []).reduce((acc, p) => acc + (p.durationMs || 0), 0);

      setStatusExitCode(exitVal);
      setStatusDurationMs(totalDur);

      let summary = '';
      if (data.passed) {
        summary = 'All pipeline stages passed successfully. Zero errors.';
      } else if (failingPhase) {
        summary = `Pipeline failed at ${failingPhase.label} stage.`;
      }
      if (data.summaryLines && data.summaryLines.length > 0) {
        summary = data.summaryLines[0];
      }
      setStatusSummary(summary);

      // Automatic tab selection:
      // If error occurred -> Show DIAGNOSTICS
      // If all passed and program ran -> Show PROGRAM RUN
      if (!data.passed || (data.diagnostics && data.diagnostics.length > 0)) {
        setOutputTab('diagnostics');
      } else if (runPhase && runPhase.status === 'passed') {
        setOutputTab('program');
      } else {
        setOutputTab('diagnostics');
      }
    } catch (err) {
      console.error('Pipeline execution error:', err);
      setStatusSummary(`Network error: ${err.message}`);
      setOutputTab('raw');
    } finally {
      setIsRunning(false);
    }
  };

  // Run single mode (/api/run)
  const handleRunQuickMode = async modeId => {
    if (isRunning) return;

    const liveSource = editorRef.current?.getSource() ?? (activeTab ? activeTab.source : '');
    const currentInputs = activeTab ? activeTab.inputs : '';

    handleChangeSource(liveSource);

    setIsRunning(true);
    setRawStdout('');
    setRawStderr('');
    setDiagnostics([]);
    setSummaryLines([]);

    try {
      const res = await fetch('/api/run', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({
          source: liveSource,
          mode: modeId,
          inputs: currentInputs
        })
      });
      const data = await res.json();

      setRawStdout(data.stdout || '');
      setRawStderr(data.stderr || '');
      setDiagnostics(data.diagnostics || []);
      setSummaryLines(data.summaryLines || []);
      setStatusExitCode(data.exitCode);
      setStatusDurationMs(data.durationMs);

      if (data.summaryLines && data.summaryLines.length > 0) {
        setStatusSummary(data.summaryLines[0]);
      } else {
        setStatusSummary(`Quick mode '${modeId}' finished with exit code ${data.exitCode}`);
      }

      if (modeId === 'run') {
        setProgramOutput({
          stdout: data.stdout,
          stderr: data.stderr,
          exitCode: data.exitCode,
          durationMs: data.durationMs
        });
        setOutputTab('program');
      } else if (data.diagnostics && data.diagnostics.length > 0) {
        setOutputTab('diagnostics');
      } else {
        setOutputTab('raw');
      }
    } catch (err) {
      console.error('Quick run error:', err);
      setStatusSummary(`Run error: ${err.message}`);
      setOutputTab('raw');
    } finally {
      setIsRunning(false);
    }
  };

  // Keyboard shortcuts handler
  useEffect(() => {
    const handleKeyDown = e => {
      // Ctrl+Enter: Run pipeline
      if ((e.ctrlKey || e.metaKey) && e.key === 'Enter') {
        e.preventDefault();
        handleRunPipeline();
      }
      // Ctrl+S: Save edit to localStorage
      if ((e.ctrlKey || e.metaKey) && (e.key === 's' || e.key === 'S')) {
        e.preventDefault();
        localStorage.setItem('cmm_editor_tabs', JSON.stringify(tabs));
        setStatusSummary('Changes saved to local storage.');
      }
      // Ctrl+/: Toggle case rail
      if ((e.ctrlKey || e.metaKey) && e.key === '/') {
        e.preventDefault();
        setRailOpen(prev => !prev);
      }
      // Esc: Close modal
      if (e.key === 'Escape') {
        if (allCasesOpen) {
          setAllCasesOpen(false);
        }
      }
      // F11: Toggle true browser fullscreen
      if (e.key === 'F11') {
        e.preventDefault();
        toggleFullScreen();
      }
    };

    window.addEventListener('keydown', handleKeyDown);
    return () => window.removeEventListener('keydown', handleKeyDown);
  }, [tabs, activeTab, isRunning, allCasesOpen, isFullscreen]);

  const jumpToLine = (line, col) => {
    if (editorRef.current) {
      editorRef.current.jumpToLineCol(line, col);
    }
  };

  return (
    <div className="w-screen h-screen flex flex-col bg-[var(--bg-page)] text-black overflow-hidden select-none">
      {/* Top Header */}
      <Header
        languageName={health ? health.languageName : 'J--'}
        health={health}
        onRunAllCases={() => setAllCasesOpen(true)}
        fontSizeOffset={fontSizeOffset}
        onChangeFontSize={delta => setFontSizeOffset(prev => prev + delta)}
        onToggleRail={() => setRailOpen(!railOpen)}
        railOpen={railOpen}
      />

      {/* Main Workspace Body */}
      <div className="flex-1 flex overflow-hidden">
        {/* Left Case Rail */}
        <CaseRail
          cases={cases}
          activeCaseId={activeCase ? activeCase.id : ''}
          onSelectCase={handleSelectCase}
          isOpen={railOpen}
        />

        {/* Central Workspace: Editor + Draggable Resizer + Terminal */}
        <div className="flex-1 flex flex-col overflow-hidden bg-[var(--bg-page)]">
          {/* Top half: Code Editor Area */}
          <div className="flex-1 min-h-[180px] flex flex-col overflow-hidden">
            <EditorArea
              ref={editorRef}
              tabs={tabs}
              activeTabId={activeTabId}
              onSelectTab={handleSelectTab}
              onCloseTab={handleCloseTab}
              onNewTab={handleNewTab}
              activeCase={activeCase}
              activeSource={activeTab ? activeTab.source : ''}
              onChangeSource={handleChangeSource}
              inputs={activeTab ? activeTab.inputs : ''}
              onChangeInputs={handleChangeInputs}
              onRunPipeline={handleRunPipeline}
              onRunQuickMode={handleRunQuickMode}
              quickModes={health ? health.quickModes : []}
              selectedQuickMode={selectedQuickMode}
              onSelectQuickMode={setSelectedQuickMode}
              onResetCase={handleResetCase}
              isRunning={isRunning}
              diagnostics={diagnostics}
              pipelineResult={pipelineResult}
              fontSizeOffset={fontSizeOffset}
            />
          </div>

          {/* Draggable Resizer Handle */}
          <div
            onMouseDown={handleMouseDown}
            className={`resizer-handle ${isDragging ? 'dragging' : ''}`}
            title="Click and drag up/down to resize terminal height"
          />

          {/* Unified Pipeline Strip (Upper Series with Stages & Right Corner Diagnostics/Raw Output) */}
          <PipelineStrip
            phases={pipelinePhases}
            activeTab={outputTab}
            onSelectTab={setOutputTab}
            diagnosticsCount={diagnostics.length}
            hasProgramOutput={Boolean(programOutput)}
          />

          {/* Bottom half: Resizable Output Terminal (No redundant second tab row!) */}
          <div
            style={{ height: `${outputHeight}px` }}
            className="flex flex-col overflow-hidden border-t border-[var(--border-subtle)] bg-white"
          >
            <OutputTabs
              activeTab={outputTab}
              diagnostics={diagnostics}
              summaryLines={summaryLines}
              rawStdout={rawStdout}
              rawStderr={rawStderr}
              phases={pipelinePhases}
              programOutput={programOutput}
              onJumpToLine={jumpToLine}
              fontSizeOffset={fontSizeOffset}
            />
          </div>
        </div>
      </div>

      {/* Status Bar */}
      <StatusBar
        exitCode={statusExitCode}
        durationMs={statusDurationMs}
        summary={statusSummary}
        languageName={health ? health.languageName : 'J--'}
      />

      {/* Full-Screen All Cases Matrix Modal */}
      <AllCasesModal
        isOpen={allCasesOpen}
        onClose={() => setAllCasesOpen(false)}
        cases={cases}
      />
    </div>
  );
}
