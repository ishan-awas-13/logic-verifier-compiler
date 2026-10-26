import { StreamLanguage } from '@codemirror/language';
import { HighlightStyle, syntaxHighlighting } from '@codemirror/language';
import { tags as t } from '@lezer/highlight';
import { EditorView } from '@codemirror/view';

// C-- Language Keyword Table (from src/lexer.cpp)
export const CMM_KEYWORDS = [
  'num',
  'decimal',
  'word',
  'letter',
  'boolean',
  'list',
  'import',
  'fixed',
  'check',
  'otherwise',
  'until',
  'foreach',
  'do_until',
  'stop',
  'skip',
  'show',
  'input',
  'give',
  'true',
  'false',
  'equals',
  'notequals',
  'and',
  'or',
  'not'
];

const keywordSet = new Set(CMM_KEYWORDS);

// Stream parser for C--
export const cmmLanguage = StreamLanguage.define({
  name: 'cmm',
  startState() {
    return { inDollarComment: false };
  },
  token(stream, state) {
    if (state.inDollarComment) {
      while (!stream.eol()) {
        if (stream.next() === '$') {
          state.inDollarComment = false;
          break;
        }
      }
      return 'comment';
    }

    if (stream.eatSpace()) return null;

    // Single-line comment: #
    if (stream.peek() === '#') {
      stream.skipToEnd();
      return 'comment';
    }

    // Multi-line comment: $ ... $
    if (stream.peek() === '$') {
      stream.next();
      state.inDollarComment = true;
      while (!stream.eol()) {
        if (stream.next() === '$') {
          state.inDollarComment = false;
          break;
        }
      }
      return 'comment';
    }

    // Double quoted string: "..."
    if (stream.peek() === '"') {
      stream.next();
      let escaped = false;
      while (!stream.eol()) {
        const c = stream.next();
        if (c === '"' && !escaped) break;
        escaped = !escaped && c === '\\';
      }
      return 'string';
    }

    // Single quoted character: '...'
    if (stream.peek() === "'") {
      stream.next();
      let escaped = false;
      while (!stream.eol()) {
        const c = stream.next();
        if (c === "'" && !escaped) break;
        escaped = !escaped && c === '\\';
      }
      return 'character';
    }

    // Numbers: integer or floating point
    if (stream.match(/^[0-9]+(\.[0-9]+)?/)) {
      return 'number';
    }

    // Operators and delimiters
    if (stream.match(/^(\+\+|--|<=|>=|==|!=|[+\-*\/%=<>;,(){}[\]])/)) {
      return 'operator';
    }

    // Identifiers and keywords
    if (stream.match(/^[a-zA-Z_][a-zA-Z0-9_]*/)) {
      const word = stream.current();
      if (keywordSet.has(word)) {
        return 'keyword';
      }
      return 'variableName';
    }

    stream.next();
    return null;
  }
});

// Authentic Minimalist Light Syntax Highlighting Style
export const cmmHighlightStyle = HighlightStyle.define([
  { tag: t.keyword, fontWeight: '700', color: '#000000' },
  { tag: t.comment, fontStyle: 'italic', color: '#6B7280' },
  { tag: t.string, textDecoration: 'underline', color: '#111827' },
  { tag: t.character, textDecoration: 'underline', color: '#111827' },
  { tag: t.number, fontWeight: '600', color: '#111827' },
  { tag: t.operator, fontWeight: '600', color: '#374151' },
  { tag: t.variableName, color: '#111827' },
  { tag: t.punctuation, color: '#4B5563' }
]);

// Clean Light Editor Theme
export const cmmMonochromeTheme = EditorView.theme({
  '&': {
    backgroundColor: '#FFFFFF',
    color: '#111827',
    fontFamily: 'var(--font-mono)',
    fontSize: '16px',
    height: '100%'
  },
  '.cm-content': {
    caretColor: '#000000',
    padding: '12px 0'
  },
  '.cm-cursor, .cm-dropCursor': {
    borderLeftColor: '#000000',
    borderLeftWidth: '2px'
  },
  '&.cm-focused .cm-selectionBackground, .cm-selectionBackground, .cm-content ::selection': {
    backgroundColor: '#000000 !important',
    color: '#FFFFFF !important'
  },
  '.cm-panels': {
    backgroundColor: '#F9FAFB',
    color: '#111827'
  },
  '.cm-activeLine': {
    backgroundColor: '#F3F4F6'
  },
  '.cm-gutters': {
    backgroundColor: '#F9FAFB',
    color: '#9CA3AF',
    borderRight: '1px solid #E5E7EB'
  },
  '.cm-activeLineGutter': {
    backgroundColor: '#E5E7EB',
    color: '#111827',
    fontWeight: 'bold'
  },
  '.cm-lineNumbers .cm-gutterElement': {
    padding: '0 10px',
    minWidth: '34px',
    textAlign: 'right'
  }
}, { dark: false });
