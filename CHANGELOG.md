# Changelog

All notable changes to Tack are documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [0.8.0] — 2026-08-18

### Added

- Mouse support: click to move the caret, click menus, scroll wheel
- ASCII pop-up dialogs with fields and buttons (Open, Save As, About, Find UI)
- Ctrl+O opens the Open dialog

## [0.7.0] — 2026-08-18

### Added

- Notepad-style menu bar: File, Edit, Format, View, Help
- Keyboard menu navigation (F10, Alt+letter, arrows, Enter, Esc)
- Menu actions for New, Save, Exit, Undo/Redo, Line Numbers, Word Wrap, themes
- Filename shown on the right of the menu bar (Links-style)

## [0.6.0] — 2026-08-18

### Added

- Line numbers in the left gutter, on by default
- Settings file (`~/.config/tack/config`) for line numbers, word wrap, tab stop,
  and key-binding theme name
- Toggle line numbers (`editor_toggle_line_numbers`)

## [0.5.0] — 2026-08-18

### Added

- Save to the current file (Ctrl+S)
- Save As via `editor_save_as` (dialogs come later)
- New document (Ctrl+N)
- Status-bar messages for save success and missing file name

## [0.4.0] — 2026-08-18

### Added

- Insert text, Enter for a new line, Tab, Backspace, and Delete
- Undo / redo (Ctrl+Z / Ctrl+Y)
- Multi-line buffer span copy and delete (used by undo)
- Dirty-document flag while editing

## [0.3.0] — 2026-08-18

### Added

- Full-screen terminal UI (alternate screen buffer)
- Virtual screen canvas used by the TUI and by tests
- Keyboard navigation: arrows, Home/End, Page Up/Down, Ctrl+arrows, Ctrl+Home/End
- Open a file for viewing (`tack FILE`); missing files start empty
- Quit with Ctrl+Q
- Title bar and status line (line/column)

## [0.2.0] — 2026-08-18

### Added

- In-memory line buffer: insert, delete, split, join, load from memory
- UTF-8 navigation helpers (next/prev code point, display columns, tabs)
- Unit tests for buffer and UTF-8 handling

## [0.1.0] — 2026-08-18

### Added

- Project layout (`src/`, `include/tack/`, `tests/`, `bin/`)
- Makefile build for the editor and the test suite
- Command-line interface: `tack [FILE]`, `--help` / `-h`, `--version` / `-v`
- Version reported as 0.1.0
