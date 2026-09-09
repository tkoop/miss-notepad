# Changelog

All notable changes to MissNotepad are documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

### Changed

- Menu accelerator hints now follow the active key theme: switching to
  nano shows ^O/^X/^W hints, vi shows :w/:q/x// hints, and Emacs shows
  C-x C-s hints; actions with no binding in the theme show no hint

## [1.3.0] — 2026-09-08

### Added

- Command-line options: `--keys=THEME` (`notepad`, `nano`, `vi` or `emacs`)
  sets the key binding theme, and `--wrap` / `--no-wrap` turn word wrap on
  or off. Both are saved to the config file so they persist across sessions
- The config file now follows the XDG Base Directory Specification:
  `$XDG_CONFIG_HOME/missnotepad/config` is used when `XDG_CONFIG_HOME` is
  set, `~/.config/missnotepad/config` otherwise (unchanged default)

### Fixed

- The config directory (`~/.config/missnotepad`) is now created recursively,
  so settings can be saved on systems where `~/.config` does not exist yet

### Documented

- Keyboard selection: Shift+arrows select, Shift+Ctrl+arrows select whole
  words, Shift+Home/End and Shift+Page Up/Down extend the selection; typing
  replaces the selection (behavior verified and covered by new regression
  tests)

## [1.2.0] — 2026-09-06

### Changed

- Save (Ctrl+S, File → Save, nano ^S, vi :w/:wq) on an untitled document
  now opens the Save As dialog to pick a file name first; the chosen name
  is kept for all future saves. Save As always asks and switches the
  document to the new name
- The caret blinks when idle (terminal-native blinking bar) and stays
  solid for one second after it moves for any reason (typing, arrow
  keys, clicking, scrolling, dialog fields)
- Mouse wheel now scrolls the file under the caret instead of moving the
  caret line by line; the caret keeps its file-line position and scrolls
  with the text, so it can go off screen while the file is scrolled away
- With word wrap on, wheel scrolling moves one screen row at a time: the
  view can start at any wrapped segment of the top line (the gutter shows
  a continuation marker there), and mouse clicks resolve against the
  scrolled view

### Fixed

- The caret is drawn as a vertical insert-style bar (DECSCUSR) instead of
  the terminal's default block, and the default shape is restored on exit

## [1.1.1] — 2026-09-05

### Added

- README install instructions for the major Linux distributions
  (Debian/Ubuntu, Fedora, Arch, openSUSE, Alpine, Void, NixOS) covering
  build tools, the optional clipboard helpers, and clone/build/install steps

## [1.1.0] — 2026-09-05

### Fixed

- Word wrap now breaks visual rows at the same points used for cursor
  navigation, so wrapped lines match the caret logic
- Down/Up move between screen lines of a wrapped file line, keeping the
  same screen column (clamped to the row width), and the caret is drawn
  at the right column on wrapped rows
- Mouse clicks (and drag-selection) now resolve to the correct file line
  and column when word wrap splits a file line across screen lines

## [1.0.0] — 2026-08-18

### Added

- Key-binding themes: Notepad (default), nano, vi, and Emacs
- vi colon commands in a pop-up (`:w`, `:q`, `:wq`, `:q!`)
- Theme-specific help text
- Complete README (use, install, compile, tests, key maps)

## [0.9.0] — 2026-08-18

### Added

- Selection (Shift+arrows, mouse drag)
- Cut, copy, paste, and Select All (Ctrl+X / C / V / A)
- Find and Find Next (Ctrl+F, F3) with a pop-up
- Replace All (Ctrl+H) with a pop-up
- Word wrap (Format → Word Wrap)

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
- Settings file (`~/.config/missnotepad/config`) for line numbers, word wrap, tab stop,
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
- Open a file for viewing (`miss FILE`); missing files start empty
- Quit with Ctrl+Q
- Title bar and status line (line/column)

## [0.2.0] — 2026-08-18

### Added

- In-memory line buffer: insert, delete, split, join, load from memory
- UTF-8 navigation helpers (next/prev code point, display columns, tabs)
- Unit tests for buffer and UTF-8 handling

## [0.1.0] — 2026-08-18

### Added

- Project layout (`src/`, `include/missnotepad/`, `tests/`, `bin/`)
- Makefile build for the editor and the test suite
- Command-line interface: `miss [FILE]`, `--help` / `-h`, `--version` / `-v`
- Version reported as 0.1.0
