EXTRA_DIST = neardal.pc.in org.neardal.conf

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = neardal.pc

confdir = $(sysconfdir)/dbus-1/system.d/
conf_DATA = org.neardal.conf

SUBDIRS = lib ncl demo

if HAVE_DOXYGEN
.PHONY: doc clean-doc

doc: clean-doc

doc: doxygen.cfg
	$(DOXYGEN) $<

clean-doc:
	-rm -rf $(builddir)/doc/*

clean-local: clean-doc
endif
