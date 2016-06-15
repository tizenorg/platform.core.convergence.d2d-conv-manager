Name:       d2d-conv-manager
Summary:    D2D Convergence Manager
Version: 	0.0.1
Release:    0
Group:      Network & Connectivity/Service
License:    Apache-2.0
Source0:    %{name}-%{version}.tar.gz
Source1:    %{name}.service
Source2:    %{name}.conf
Source1001: 	%{name}.manifest
Source1002: 	lib%{name}.manifest

%define BUILD_PROFILE %{?profile}%{!?profile:%{?tizen_profile_name}}

ExcludeArch: aarch64 x86_64

BuildRequires: cmake
BuildRequires: pkgconfig(gio-2.0)
BuildRequires: pkgconfig(glib-2.0)
BuildRequires: pkgconfig(dlog)

BuildRequires: pkgconfig(json-glib-1.0)
BuildRequires: pkgconfig(vconf)
BuildRequires: pkgconfig(iotcon)
#BuildRequires: pkgconfig(security-server)
BuildRequires: pkgconfig(capi-appfw-app-manager)
BuildRequires: pkgconfig(libwebsockets)
BuildRequires: pkgconfig(iotcon)
BuildRequires: pkgconfig(vconf)
BuildRequires: pkgconfig(capi-network-bluetooth)
BuildRequires: pkgconfig(capi-network-wifi-direct)
BuildRequires: pkgconfig(capi-appfw-application)
BuildRequires: pkgconfig(capi-network-connection)
BuildRequires: pkgconfig(bundle)
BuildRequires: pkgconfig(capi-system-info)
BuildRequires: pkgconfig(cynara-creds-gdbus)
BuildRequires: pkgconfig(cynara-client)
BuildRequires: pkgconfig(cynara-session)
BuildRequires: pkgconfig(capi-appfw-package-manager)
BuildRequires: pkgconfig(nsd-dns-sd)
BuildRequires: pkgconfig(aul)
BuildRequires: pkgconfig(libtzplatform-config)
BuildRequires: pkgconfig(libcurl)
BuildRequires: openssl
BuildRequires: openssl-devel
BuildRequires: curl
BuildRequires: libcurl-devel

%if "%{?BUILD_PROFILE}" == "mobile"

%endif

BuildRequires: boost-devel
BuildRequires: boost-thread
BuildRequires: boost-system
BuildRequires: boost-filesystem

%description
D2D Convergence Manager Service

%prep
%setup -q
cp %{SOURCE1001} .
cp %{SOURCE1002} .

%package lib
Summary:    Client library
Group:      Network & Connectivity/Service


%description lib
D2D Convergence Manager client library for applications.


%package devel
Summary:    Client library (devel)
Group:      Network & Connectivity/Development
Requires:   d2d-conv-manager-lib = %{version}


%description devel
D2D Convergence Manager development kit.


%build
MAJORVER=`echo %{version} | awk 'BEGIN {FS="."}{print $1}'`
%cmake . -DMAJORVER=${MAJORVER} -DFULLVER=%{version} -DBIN_INSTALL_DIR:PATH=%{_bindir} -DPROFILE=%{?BUILD_PROFILE}


%install
rm -rf %{buildroot}/BUILD/d2d-conv-manager*
%make_install

mkdir -p %{buildroot}%{_unitdir_user}
mkdir -p %{buildroot}%{_datadir}/license
#mkdir -p %{buildroot}%{_libdir}/systemd/system/graphical.target.wants
cp LICENSE %{buildroot}%{_datadir}/license/%{name}
install -m 0644 %{SOURCE1} %{buildroot}%{_unitdir_user}
#ln -s ../%{name}.service %{buildroot}%{_libdir}/systemd/system/graphical.target.wants/%{name}.service

mkdir -p %{buildroot}%{_sysconfdir}/dbus-1/session.d
install -m 0644 %{SOURCE2} %{buildroot}%{_sysconfdir}/dbus-1/session.d/

%post
mkdir -p %{_unitdir_user}/default.target.wants
ln -s ../%{name}.service %{_unitdir_user}/default.target.wants/
/sbin/ldconfig

#systemctl daemon-reload
#if [ $1 == 1 ]; then
#    systemctl restart %{name}.service
#fi

%postun
rm -f %{_unitdir_user}/default.target.wants/%{name}.service
/sbin/ldconfig

#if [ $1 == 0 ]; then
#    systemctl stop %{name}.service
#fi
#systemctl daemon-reload

%post lib -p /sbin/ldconfig

%postun lib -p /sbin/ldconfig

%files
%manifest %{name}.manifest
%config %{_sysconfdir}/dbus-1/session.d/d2d-conv-manager.conf
%{_unitdir_user}/%{name}.service
%{_bindir}/%{name}d
%{_bindir}/%{name}-test
%{_bindir}/msf-api-test*
%{_datadir}/license/%{name}

%files lib
%manifest lib%{name}.manifest
%defattr(-,root,root,-)
%{_libdir}/lib%{name}.so.*
%{_datadir}/license/%{name}


%files devel
%defattr(-,root,root,-)
%{_libdir}/lib%{name}.so
%{_libdir}/pkgconfig/%{name}.pc
%{_includedir}/*.h
