Name: cupl
Version: %{myversion}
Release: 1
URL: http://www.tuxedo.org/~esr/cupl/
Source0: %{name}-%{version}.tar.gz
License: GPL
Group: Languages
Summary: Interpreter for the archaic CUPL and CORC programming languages.
%undefine __check_files

%description
Interpreter for the archaic CUPL and CORC programming languages.
Includes full documentation and a chrestomathy of programs.

%prep
%setup

%build
make

%install
rm -f /usr/bin/cupl /usr/share/man/man1/cupl.1
cp cupl /usr/bin
cp cupl.1 /usr/share/man/man1/cupl.1

%files
%doc READ.ME COPYING corc.doc cupl.doc test
/usr/bin/cupl
/usr/share/man/man1/cupl.1

%changelog
* Fri Dec 26 2003 Eric S. Raymond <esr@thyrsus.com> 1.5 
- Updated RPM packaging machinery.

* Tue Jul 30 2000 Eric S. Raymond <esr@thyrsus.com> 1.4
- Move documentation to XML.

* Thu Jul 27 2000 Eric S. Raymond <esr@thyrsus.com> 1.3
- Cleanup release.  Use modern packaging and RPMs.

RPM chokes on the older dates below.

Fri Jun 28 1996 Eric S. Raymond <esr@thyrsus.com> 1.2
- Fixed off-by-one bug in TITLE processing.
- Merged tour.doc into cupl.mm.
- Added "Hello World" program.

Fri Nov 08 1994 Eric S. Raymond <esr@thyrsus.com> 1.1
- Added CORC support.
- Fixed a core-dump bug in the prettyprinter.
