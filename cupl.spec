Name: cupl
Version: 1.3
Release: 1
Source: http://www.tuxedo.org/~esr/cupl-1.3.tar.gz
Copyright: GPL
Group: Languages
Summary: Interpreter for the archaic CUPL and CORC programming languages.

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

%files
%doc READ.ME COPYING corc.doc cupl.doc
/usr/bin/cupl
/usr/share/man/man1/cupl.1
