# >> macros
%define libneardal_dir %{_libdir}
%define libneardal_pkg %{_libdir}/pkgconfig
%define libneardal_inc %{_includedir}/neardal

%define glib2_version   		2.30.0
# << macros

Name: neardal
Summary: Neard Abstraction Library
Version: 0.1.0
Release: 1.0
Group: System/Libraries
License: LGPLv2
URL: https://github.com/connectivity/neardal.git
Source0: lib%{name}-%{version}.tar.bz2

Requires(post): /sbin/ldconfig
Requires(postun): /sbin/ldconfig
BuildRequires: python 
BuildRequires: intltool >= %{intltool_version}
BuildRequires: libtool
BuildRequires: automake
BuildRequires: autoconf
BuildRequires: gettext
BuildRequires: pkgconfig(glib-2.0) >= %{glib2_version}
BuildRequires: pkgconfig(dbus-glib-1)

%description
This package provides simple C APIs to exchange datas with NFC daemon (Neard) present on the system.

%prep
%setup -q -n lib%{name}-%{version}

%build
autoreconf --force --install

%configure --prefix=/usr
make
  
%install      
rm -rf %{buildroot}
%make_install

# executed after install
%post
/sbin/ldconfig

# executed before uninstall
%postun
/sbin/ldconfig
  
# No locale
# %%find_lang %%{name}
# %%files -f %%{name}.lang

%files
%defattr(-,root,root,-)
%doc README AUTHORS NEWS COPYING

# libraries files
%{libneardal_dir}/libneardal.so
%{libneardal_dir}/libneardal.so.0
%{libneardal_dir}/libneardal.so.0.0.1

# headers files
%{libneardal_inc}/neardal.h
%{libneardal_inc}/neardal_errors.h
# pkg-config files
%{libneardal_pkg}/*.pc
%changelog

