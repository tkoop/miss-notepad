# Tack

**Microsoft Notepad for the Linux command line.**

Tack is a full-screen terminal text editor. It is meant to feel familiar to
anyone who has used Notepad, while running entirely in the terminal (a TUI,
not a GUI) — like nano, but with a Notepad-style menu, pop-up dialogs, and
mouse support.

This is **version 0.4.0**: a full-screen editor you can type in, with undo.
Menus, save dialogs, and mouse support arrive in later versions.

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
keys, Home/End, and Page Up/Down. Ctrl+Z undoes, Ctrl+Y redoes, Ctrl+Q quits.

## Tests

```sh
make test
```

## Requirements

- A C11 compiler (`gcc` or `clang`)
- `make`
- A POSIX system (Linux)
