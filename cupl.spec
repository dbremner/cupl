Description: interpreter for the archaic CUPL and CORC programming languages
Name: cupl
Version: 1.1
Release: 1
Source: locke.ccil.org:/pub/esr/cupl-1.1.tar.gz
Copyright: BSD-like
Group: Languages

%prep
%setup

%build
make

%install
rm -f /usr/bin/cupl
cp cupl /usr/bin
cp cupl.1 /usr/man/man1/cupl.1

%files
%doc /usr/man/man1/cupl.1
/usr/bin/cupl
