Name:        starfaceserver
Version:     @STARFACESERVER_RPM_VERSION
Release:     1
Summary:     STAROS
Vendor:      Nanjing StarOS Network Technology Co., Ltd
Packager:    StarOS.xyz

Group:       Development/Server
License:     Commercial
URL:         http://www.staros.xyz
Source0:     %{name}-@STARFACESERVER_RPM_VERSION.tar.gz

BuildRoot: %{_tmppath}/%{name}-buildroot
Requires: starcore
Requires: stardlls
Requires: dipc
Requires: protocol
Requires: starlang
%define _binaries_in_noarch_packages_terminate_build   0

%description
Starface Server binaries for Red Hat Linux.

%prep
%setup -q

%install
mkdir -p %{buildroot}
cp -R * %{buildroot}

%clean
rm -rf %{buildroot}

%files
%defattr(-,root,root,-)
/opt/staros.xyz/starfaceserver

%post
echo Starface server successfully

%preun
%postun
#TODO this should be less brute-force
rm -rf /opt/staros.xyz/starfaceserver
%changelog
# TODO

