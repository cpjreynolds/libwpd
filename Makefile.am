ACLOCAL_AMFLAGS = -I m4

pkgconfdir = $(libdir)/pkgconfig

pkgconf_DATA = libwpd-@WPD_MAJOR_VERSION@.@WPD_MINOR_VERSION@.pc

SUBDIRS = build inc src docs

EXTRA_DIST = \
	autogen.sh \
	libwpd.pc.in \
	COPYING.LGPL \
	COPYING.MPL \
	CREDITS \
	HACKING \
	INSTALL \
	NEWS \
	README \
	TODO

distclean-local:
	rm -rf *.cache *~ *.pc

dist-hook:
	git log --date=short --pretty="format:@%cd  %an  <%ae>  [%H]%n%n%s%n%n%e%b" | sed -e "s|^\([^@]\)|\t\1|" -e "s|^@||" >$(distdir)/ChangeLog

astyle:
	astyle --options=astyle.options \*.h \*.cpp
