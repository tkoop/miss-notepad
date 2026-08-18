# Changelog

All notable changes to Tack are documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

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
