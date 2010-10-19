Name: cupl
Summary: Interpreter for the archaic CUPL and CORC programming languages.
Version: 1.7
Release: 1
URL: http://www.catb.org/~esr/cupl/
Source0: %{name}-%{version}.tar.gz
License: GPL
Group: Languages
BuildRoot: %{_tmppath}/%{name}-root
#Project-Tag-List: CUPL, CORC, language, interpreter, retrocomputing

%description
Interpreter for the archaic CUPL and CORC programming languages.
Includes full documentation and a chrestomathy of programs.

%prep
%setup -q

%build
make %{?_smp_mflags} cupl cupl.1

%install
[ "$RPM_BUILD_ROOT" -a "$RPM_BUILD_ROOT" != / ] && rm -rf "$RPM_BUILD_ROOT"
mkdir -p "$RPM_BUILD_ROOT"%{_bindir}
mkdir -p "$RPM_BUILD_ROOT"%{_mandir}/man1/
cp cupl "$RPM_BUILD_ROOT"%{_bindir}
cp cupl.1 "$RPM_BUILD_ROOT"%{_mandir}/man1/

%clean
[ "$RPM_BUILD_ROOT" -a "$RPM_BUILD_ROOT" != / ] && rm -rf "$RPM_BUILD_ROOT"

%files
%doc README COPYING corc.doc cupl.doc test
%{_bindir}/cupl
%{_mandir}/man1/cupl.1*

%changelog
* Wed Oct 12 2003 Eric S. Raymond <esr@snark.thyrsus.com> 1.7-1
- Port to modern C, ANSIfying where needed.

* Mon Dec 29 2003 Eric S. Raymond <esr@snark.thyrsus.com> 1.6-1
- Source RPMSs no longer depend on --define myversion.

* Fri Dec 26 2003 Eric S. Raymond <esr@thyrsus.com> 1.5-1
- Updated RPM packaging machinery.

* Tue Jul 30 2000 Eric S. Raymond <esr@thyrsus.com> 1.4-1
- Move documentation to XML.

* Thu Jul 27 2000 Eric S. Raymond <esr@thyrsus.com> 1.3-1
- Cleanup release.  Use modern packaging and RPMs.

RPM chokes on the older dates below.

Fri Jun 28 1996 Eric S. Raymond <esr@thyrsus.com> 1.2
- Fixed off-by-one bug in TITLE processing.
- Merged tour.doc into cupl.mm.
- Added "Hello World" program.

Fri Nov 08 1994 Eric S. Raymond <esr@thyrsus.com> 1.1
- Added CORC support.
- Fixed a core-dump bug in the prettyprinter.
