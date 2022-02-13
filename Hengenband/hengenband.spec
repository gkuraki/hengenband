%define version 1.1.0b
%define release 3

Summary: hengenband %{version}
Name: hengenband
Version: %{version}
Release: %{release}
Copyright: unknown
Group: Amusements/Games
Packager: Takahiro MIZUNO <tow@plum.freemail.ne.jp>
Url: http://echizen.s5.xrea.com/heng/index.html
Source: hengenband-%{version}.tar.bz2
Buildroot: %{_tmppath}/%{name}-%{version}-root

%description
hengenband is a variant of ZAngband.

Official page is this,
http://echizen.s5.xrea.com/heng/eng-hengenband/index.html

More infomation is /usr/doc/hengenband-hoge/readme_eng.txt

Summary(ja):  —∏∏»⁄≈‹ %{version}

%description -l ja
 —∏∏»⁄≈‹§œ Angband §Œ•–¶ÅE¢•Û•»§«§π°§ÅEÀ‹•Ω•’•»•¶•ß•¢§Œ∫«ø∑»«§œ∞ ≤º§ŒΩ‡EÍ§´§È∆˛ºÍ§«§≠§ﬁ§π°§ähttp://echizen.s5.xrea.com/heng/index.html

æ‹§∑§Ø§œ /usr/doc/hengenband-hoge/readme.txt §Úª≤æ»°£

%prep
rm -rf $RPM_BUILD_ROOT

%setup -n %{name}-%{version}

%build
./configure --prefix=%{_prefix} --bindir=%{_bindir} --with-libpath=%{_datadir}/games/hengenband/lib
make

%install
mkdir -p $RPM_BUILD_ROOT/%{_bindir}
mkdir -p $RPM_BUILD_ROOT/%{_datadir}/games/hengenband
cp src/hengenband $RPM_BUILD_ROOT/%{_bindir}
cp -R lib/ -p $RPM_BUILD_ROOT/%{_datadir}/games/hengenband/
touch $RPM_BUILD_ROOT/%{_datadir}/games/hengenband/lib/apex/scores.raw

%clean
rm -rf $RPM_BUILD_ROOT

%preun
if [ -e %{_datadir}/games/hengenband/lib/data/f_info_j.raw ]
then
rm -rf %{_datadir}/games/hengenband/lib/data/*.raw
fi
exit 0

%files
%defattr(-,root,root)
%attr(2755,root,games) %{_bindir}/hengenband
%dir %{_datadir}/games/hengenband/lib
%attr(775,root,games) %dir %{_datadir}/games/hengenband/lib/apex
%attr(775,root,games) %dir %{_datadir}/games/hengenband/lib/bone
%attr(775,root,games) %dir %{_datadir}/games/hengenband/lib/data
%dir %{_datadir}/games/hengenband/lib/edit
%dir %{_datadir}/games/hengenband/lib/file
%dir %{_datadir}/games/hengenband/lib/help
%dir %{_datadir}/games/hengenband/lib/info
%dir %{_datadir}/games/hengenband/lib/pref
%attr(775,root,games) %dir %{_datadir}/games/hengenband/lib/save
%dir %{_datadir}/games/hengenband/lib/script
%dir %{_datadir}/games/hengenband/lib/user
%dir %{_datadir}/games/hengenband/lib/xtra
%dir %{_datadir}/games/hengenband/lib/xtra/graf
%{_datadir}/games/hengenband/lib/apex/h_scores.raw
%{_datadir}/games/hengenband/lib/apex/readme.txt
%attr(664 root,games) %config(noreplace) %{_datadir}/games/hengenband/lib/apex/scores.raw
%{_datadir}/games/hengenband/lib/bone/delete.me
%{_datadir}/games/hengenband/lib/data/delete.me
%{_datadir}/games/hengenband/lib/edit/*.txt
%{_datadir}/games/hengenband/lib/file/*.txt
%{_datadir}/games/hengenband/lib/help/*.hlp
%{_datadir}/games/hengenband/lib/help/*.txt
%{_datadir}/games/hengenband/lib/info/delete.me
%{_datadir}/games/hengenband/lib/pref/*.prf
%{_datadir}/games/hengenband/lib/save/delete.me
%{_datadir}/games/hengenband/lib/script/delete.me
%{_datadir}/games/hengenband/lib/user/delete.me
%{_datadir}/games/hengenband/lib/xtra/graf/8x8.bmp
%doc readme.txt readme_angband readme_eng.txt


%changelog

* Fri Jul 05 2002 Takahiro MIZUNO <tow@plum.freemail.ne.jp>
- hengenband RPM 1.0.0b release 3
- Add %preun script.
- Change source extension. (tar.gz -> bz2)
- Fix Copyright.
- Fix simply %files.
- Fix %description.

* Mon Jun 17 2002 Takahiro MIZUNO <tow@plum.freemail.ne.jp>
- hengenband RPM 1.0.0b release 2
- Fix setgid permission. (Mogami§µ§Û¬øº’)

* Sun Jun 16 2002 Takahiro MIZUNO <tow@plum.freemail.ne.jp>
- hengenband RPM 1.0.0b release 1

* Sun Jun 16 2002 Takahiro MIZUNO <tow@plum.freemail.ne.jp> 
- hengenband RPM 1.0.0 release 1

