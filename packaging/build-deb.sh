#!/bin/sh
# Build a Debian .deb package for MissNotepad.
# Output: dist/missnotepad_<version>-1_<arch>.deb
#
# Requires: dpkg-deb (present on all Debian-family systems).
set -eu
cd "$(dirname "$0")/.."

VERSION="$(sed -n 's/.*VERSION_STRING "\(.*\)".*/\1/p' include/missnotepad/version.h)"
ARCH="$(dpkg --print-architecture)"
REV="1"

STAGE="$(mktemp -d)"
trap 'rm -rf "$STAGE"' EXIT

echo "==> Building (dynamic) for .deb, version ${VERSION}..."
make all >/dev/null

PKGDIR="${STAGE}/missnotepad-${VERSION}"
DOC="${PKGDIR}/usr/share/doc/missnotepad"
mkdir -p "${PKGDIR}/DEBIAN" "${PKGDIR}/usr/bin" "$DOC"

install -m 0755 bin/miss "${PKGDIR}/usr/bin/miss"
cp README.md "${DOC}/README.md"
gzip -n -9 -c CHANGELOG.md > "${DOC}/changelog.gz"

cat > "${DOC}/copyright" <<EOF
Format: https://www.debian.org/doc/packaging-manuals/copyright-format/1.0/
Upstream-Name: missnotepad
Source: https://github.com/tkoop/miss-notepad

Files: *
Copyright: 2026 Tim Koop
License: MIT
Permission is hereby granted, free of charge, to any person obtaining a
copy of this software and associated documentation files (the "Software"),
to deal in the Software without restriction, including without limitation
the rights to use, copy, modify, merge, publish, distribute, sublicense,
and/or sell copies of the Software, and to permit persons to whom the
Software is furnished to do so, subject to the following conditions:
.
The above copyright notice and this permission notice shall be included
in all copies or substantial portions of the Software.
.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
EOF

cat > "${PKGDIR}/DEBIAN/control" <<EOF
Package: missnotepad
Version: ${VERSION}-${REV}
Section: editors
Priority: optional
Architecture: ${ARCH}
Maintainer: Tim Koop <tkoop@users.noreply.github.com>
Depends: libc6 (>= 2.34)
Homepage: https://github.com/tkoop/miss-notepad
Description: Notepad-style text editor for the terminal
 MissNotepad (command: miss) is a small terminal text editor with a
 Notepad-style menu bar, line numbers, word wrap, undo/redo, search and
 replace, and Notepad/nano/vi/Emacs key-binding themes.
 Written in C11 with no dependencies beyond libc.
EOF

mkdir -p dist
dpkg-deb --build --root-owner-group "${PKGDIR}" \
  "dist/missnotepad_${VERSION}-${REV}_${ARCH}.deb"
echo "==> Built dist/missnotepad_${VERSION}-${REV}_${ARCH}.deb"