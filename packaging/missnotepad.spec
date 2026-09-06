# RPM spec for MissNotepad. Build with packaging/build-rpm.sh, which
# supplies a source tarball named missnotepad-<version>.tar.gz and
# rewrites the Version field from include/missnotepad/version.h.
Name:           missnotepad
Version:        1.2.0
Release:        1%{?dist}
Summary:        Notepad-style text editor for the terminal

License:        MIT
URL:            https://github.com/tkoop/miss-notepad
Source0:        %{name}-%{version}.tar.gz

BuildRequires:  gcc
BuildRequires:  make
Requires:       glibc

%description
MissNotepad (command: miss) is a small terminal text editor with a
Notepad-style menu bar, line numbers, word wrap, undo/redo, search and
replace, and Notepad/nano/vi/Emacs key-binding themes. Written in C11
with no dependencies beyond libc.

%prep
%setup -q

%build
%set_build_flags
make %{?_smp_mflags}

%install
make install DESTDIR=%{buildroot} PREFIX=/usr

%files
%{_bindir}/miss
%doc README.md CHANGELOG.md

%changelog
* Sun Sep 06 2026 Tim Koop <tkoop@users.noreply.github.com> - 1.2.0-1
- Wheel scrolling moves the view instead of the caret; word wrap scrolls
  one screen row per notch
- Caret blinks when idle, solid for a second after movement
- Saving an untitled document asks for a file name first

* Sat Sep 05 2026 Tim Koop <tkoop@users.noreply.github.com> - 1.1.1-1
- Initial RPM packaging