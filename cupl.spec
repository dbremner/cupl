Name: cupl
Version: 1.4
Release: 1
URL: http://www.tuxedo.org/~esr/
Source0: %{name}-%{version}.tar.gz
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
cp cupl.1 /usr/share/man/man1/cupl.1

%files
%doc READ.ME COPYING NEWS corc.doc cupl.doc test
/usr/bin/cupl
/usr/share/man/man1/cupl.1
