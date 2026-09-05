#!/bin/sh
# MissNotepad installer.
#
# Usage:
#   curl -fsSL https://raw.githubusercontent.com/tkoop/miss-notepad/master/scripts/install.sh | sh
#   ... or download and run:  ./install.sh [version]
#
# Downloads the latest (or given) static binary release from GitHub,
# verifies its SHA-256 checksum, and installs it to $HOME/.local/bin
# (or /usr/local/bin, or $MISS_INSTALL_DIR). No root required unless
# you want it in /usr/local/bin and lack write access.
set -eu

REPO="tkoop/miss-notepad"
TAG="${1:-latest}"

case "$(uname -s)" in
  Linux) ;;
  *) echo "error: this installer only supports Linux" >&2; exit 1 ;;
esac

case "$(uname -m)" in
  x86_64|amd64)   ARCH="x86_64" ;;
  aarch64|arm64)  ARCH="aarch64" ;;
  *) echo "error: unsupported architecture: $(uname -m)" >&2; exit 1 ;;
esac

fetch() {
  # fetch <url> <output>
  if command -v curl >/dev/null 2>&1; then
    curl -fsSL -o "$2" "$1"
  elif command -v wget >/dev/null 2>&1; then
    wget -qO "$2" "$1"
  else
    echo "error: need curl or wget to download" >&2
    exit 1
  fi
}

BASE="https://github.com/${REPO}/releases"
case "$TAG" in
  latest) URL="${BASE}/latest/download" ;;
  v*)     URL="${BASE}/download/${TAG}" ;;
  *)      URL="${BASE}/download/v${TAG}" ;;
esac

ASSET="miss-linux-${ARCH}.tar.gz"
TMP="$(mktemp -d)"
trap 'rm -rf "$TMP"' EXIT

echo "==> Downloading MissNotepad ${TAG} for ${ARCH}..."
fetch "${URL}/${ASSET}"          "${TMP}/${ASSET}"
fetch "${URL}/${ASSET}.sha256"   "${TMP}/${ASSET}.sha256"

echo "==> Verifying checksum..."
if ! (cd "$TMP" && sha256sum -c "${ASSET}.sha256" >/dev/null 2>&1); then
  expected="$(cut -d' ' -f1 "${TMP}/${ASSET}.sha256")"
  actual="$(sha256sum "${TMP}/${ASSET}" | cut -d' ' -f1)"
  if [ "$expected" != "$actual" ]; then
    echo "error: checksum mismatch (expected $expected, got $actual)" >&2
    exit 1
  fi
fi
echo "    OK"

tar -xzf "${TMP}/${ASSET}" -C "$TMP"
if [ ! -f "${TMP}/miss" ]; then
  echo "error: archive did not contain the miss binary" >&2
  exit 1
fi

# Pick an install dir: explicit override > ~/.local/bin (if on PATH)
# > /usr/local/bin (directly writable or root) > sudo /usr/local/bin
# > ~/.local/bin created on the spot.
SUDO=""
if [ -n "${MISS_INSTALL_DIR:-}" ]; then
  INSTALL_DIR="$MISS_INSTALL_DIR"
elif [ -d "$HOME/.local/bin" ] && case ":$PATH:" in
       *":$HOME/.local/bin:"*) true ;; *) false ;; esac; then
  INSTALL_DIR="$HOME/.local/bin"
elif [ "$(id -u)" = "0" ] || [ -w /usr/local/bin 2>/dev/null ]; then
  INSTALL_DIR="/usr/local/bin"
elif command -v sudo >/dev/null 2>&1; then
  INSTALL_DIR="/usr/local/bin"
  SUDO="sudo"
else
  INSTALL_DIR="$HOME/.local/bin"
fi

if [ ! -d "$INSTALL_DIR" ]; then
  $SUDO mkdir -p "$INSTALL_DIR"
fi

echo "==> Installing to ${INSTALL_DIR}/miss..."
$SUDO install -m 0755 "${TMP}/miss" "${INSTALL_DIR}/miss"

echo
echo "Installed! Run it with:"
echo "  ${INSTALL_DIR}/miss FILE"
case ":$PATH:" in
  *":${INSTALL_DIR}:"*) ;;
  *)
    echo
    echo "NOTE: ${INSTALL_DIR} is not on your PATH. Add this to your shell profile:"
    echo "  export PATH=\"${INSTALL_DIR}:\$PATH\""
    ;;
esac