Name: cupl
Version: 1.3
Release: 1
Source: locke.ccil.org:/pub/esr/cupl-1.3.tar.gz
Copyright: BSD-like
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
rm -f /usr/bin/cupl
cp cupl /usr/bin

%files
%doc cupl.mm
/usr/bin/cupl
