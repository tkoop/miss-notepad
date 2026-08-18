# Tack

**Microsoft Notepad for the Linux command line.**

Tack is a full-screen terminal text editor. It is meant to feel familiar to
anyone who has used Notepad, while running entirely in the terminal (a TUI,
not a GUI) — like nano, but with a Notepad-style menu, pop-up dialogs, and
mouse support.

This is **version 0.8.0**: menus, mouse, and pop-up dialogs.
Search/replace, clipboard, and word wrap arrive next.

## Build

```sh
make
```

The executable is written to `bin/tack`.

## Run

```sh
bin/tack
bin/tack notes.txt
bin/tack --help
bin/tack --version
```

Tack takes over the whole terminal. Type to insert text. Move with the arrow
keys, Home/End, and Page Up/Down.

- Use the **mouse** to click the menu bar, dialog buttons, and text
- **F10** or **Alt+F / Alt+E / Alt+O / Alt+V / Alt+H** open menus
- **Ctrl+O** Open dialog, **Ctrl+S** save, **File → Save As** for a new name
- **Ctrl+N** new document
- **Ctrl+Z** undo, **Ctrl+Y** redo
- **Ctrl+Q** quit

## Tests

```sh
make test
```

## Requirements

- A C11 compiler (`gcc` or `clang`)
- `make`
- A POSIX system (Linux)
