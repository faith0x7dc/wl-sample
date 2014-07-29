%define _unpackaged_files_terminate_build 0
%bcond_with wayland

Name:           wl-sample
Version:        0.1.4
Release:        0
Summary:        Wayland sample application
License:        MIT
Group:          Graphics

Source0:        %name-%version.tar.xz
BuildRequires:  pkgconfig(wayland-client)
BuildRequires:  pkgconfig(wayland-egl)
BuildRequires:  pkgconfig(wayland-cursor)
BuildRequires:  pkgconfig(egl)
BuildRequires:  pkgconfig(libpng)
BuildRequires:  mesa-devel

%if !%{with wayland}
ExclusiveArch:
%endif


%description
Wayland sample application

%prep
%setup -q

%build
%autogen
make V=1 CFLAGS="-g -Wall"

%install
%make_install

mkdir -p %{buildroot}%{_bindir}/wl-sample/

install -m 755 solid-surf-egl %{buildroot}%{_bindir}/wl-sample/
install -m 755 square %{buildroot}%{_bindir}/wl-sample/
install -m 755 double-square %{buildroot}%{_bindir}/wl-sample/
install -m 755 square-shm %{buildroot}%{_bindir}/wl-sample/
install -m 755 event-test %{buildroot}%{_bindir}/wl-sample/
install -m 755 image-viewer-egl %{buildroot}%{_bindir}/wl-sample/

%files
%defattr(-,root,root)
%_bindir/wl-sample/solid-surf-egl
%_bindir/wl-sample/square
%_bindir/wl-sample/double-square
%_bindir/wl-sample/square-shm
%_bindir/wl-sample/event-test
%_bindir/wl-sample/image-viewer-egl

