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

%files
%doc cupl.mm
/usr/bin/cupl
