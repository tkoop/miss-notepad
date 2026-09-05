#!/bin/sh
# Build an RPM for MissNotepad.
# Output: dist/<arch>/missnotepad-<version>-1.<arch>.rpm
#
# Uses rpmbuild directly if installed; otherwise falls back to a Fedora
# Docker container (requires docker).
set -eu
cd "$(dirname "$0")/.."

VERSION="$(sed -n 's/.*VERSION_STRING "\(.*\)".*/\1/p' include/missnotepad/version.h)"
TAG="v${VERSION}"
OUT="dist"

mkdir -p "$OUT"

echo "==> Creating source tarball missnotepad-${VERSION}.tar.gz..."
if git rev-parse -q --verify "refs/tags/${TAG}" >/dev/null; then
  REF="$TAG"
else
  REF="HEAD"
fi
git archive --format=tar.gz --prefix="missnotepad-${VERSION}/" \
  -o "${OUT}/missnotepad-${VERSION}.tar.gz" "$REF"

RPM_TOP="$(pwd)/.rpmbuild"

# NOTE: each --define must be ONE shell argument containing the space
# between macro name and value ("--define=_topdir /path"), so they are
# written out literally rather than via a variable.
if command -v rpmbuild >/dev/null 2>&1; then
  echo "==> Building RPM with local rpmbuild..."
  rpmbuild -bb \
    --define="_topdir ${RPM_TOP}" \
    --define="_sourcedir $(pwd)/${OUT}" \
    --define="_rpmdir $(pwd)/${OUT}" \
    --define="_builddir ${RPM_TOP}/BUILD" \
    --define="_specdir $(pwd)/packaging" \
    --define="_srcrpmdir $(pwd)/${OUT}" \
    packaging/missnotepad.spec
else
  echo "==> Building RPM inside a Fedora Docker container..."
  docker run --rm -v "$(pwd):/src" -w /src fedora:latest sh -c "
    dnf install -y rpm-build gcc make >/dev/null &&
    rpmbuild -bb \
      --define='_topdir /src/.rpmbuild' \
      --define='_sourcedir /src/dist' \
      --define='_rpmdir /src/dist' \
      --define='_builddir /src/.rpmbuild/BUILD' \
      --define='_specdir /src/packaging' \
      --define='_srcrpmdir /src/dist' \
      packaging/missnotepad.spec"
fi

# rpmbuild puts results in dist/<arch>/; flatten them.
find "${OUT}" -mindepth 2 -name '*.rpm' -exec mv -t "${OUT}" {} +
rm -rf "${RPM_TOP}"
echo "==> Built $(ls "${OUT}"/*.rpm)"
