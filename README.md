# Tack

**Microsoft Notepad for the Linux command line.**

Tack is a full-screen terminal text editor. It is meant to feel familiar to
anyone who has used Notepad, while running entirely in the terminal (a TUI,
not a GUI) — like nano, but with a Notepad-style menu, pop-up dialogs, and
mouse support.

This is **version 0.2.0**: foundation plus an in-memory text buffer.
Later versions add the full-screen editor.

## Build

```sh
make
```

The executable is written to `bin/tack`.

## Run

```sh
bin/tack --help
bin/tack --version
```

## Tests

```sh
make test
```

## Requirements

- A C11 compiler (`gcc` or `clang`)
- `make`
- A POSIX system (Linux)
