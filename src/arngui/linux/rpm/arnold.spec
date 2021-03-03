#
# spec file for package spec (Version 2.0)
#
# Copyright (c) 2003 SuSE Linux AG, Nuernberg, Germany.
# This file and all modifications and additions to the pristine
# package are under the same license as the package itself.
#
# Please submit bugfixes or comments via http://www.suse.de/feedback/
#

# norootforbuild

Name:         arnold
License:      GPL
Group:        unsorted
Autoreqprov:  on
Version:      20090317
Release:      0
Summary:      Amstrad CPC Emulator
Source:       %name.tar.gz
Requires:     wxGTK SDL
BuildRequires: wxGTK-devel SDL-devel
BuildRoot:     %{_tmppath}/%{name}-%{version}-build
URL: http://80.229.30.31:1971/arnold/index

%description
Amstrad CPC emulator

%prep
%setup -q -n %name

%build
cd ..
rm -rf build_tree
mkdir build_tree
cd build_tree
cmake -DCMAKE_INSTALL_PREFIX=%{rpmprefix} ../%{srcdirname}
make

%install
make install DESTDIR=%{buildroot}


%clean
rm -rf %{srcdirname}
rm -rf build_tree

%files
%defattr(-,root,root)
%doc *.txt *.linux README*
/usr/bin/arnold
