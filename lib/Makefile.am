AM_CPPFLAGS = @gio_CFLAGS@ -I$(builddir) -I$(srcdir) @NEARDAL_EXTRA_FLAGS@

lib_LTLIBRARIES = libneardal.la
noinst_LTLIBRARIES = libgenerated.la

libneardal_la_SOURCES = \
	$(srcdir)/neardal.c \
	$(srcdir)/neardal_adapter.c $(srcdir)/neardal_adapter.h \
	$(srcdir)/neardal_agent_mgr.c $(srcdir)/neardal_agent_mgr.h \
	$(srcdir)/neardal_device.c $(srcdir)/neardal_device.h \
	$(srcdir)/neardal_manager.c $(srcdir)/neardal_manager.h \
	$(srcdir)/neardal_prv.h \
	$(srcdir)/neardal_record.c $(srcdir)/neardal_record.h \
	$(srcdir)/neardal_tag.c $(srcdir)/neardal_tag.h \
	$(srcdir)/neardal_tools.c $(srcdir)/neardal_tools.h \
	$(srcdir)/neardal_traces.c \
	$(srcdir)/neardal_traces_prv.h

libneardal_la_LIBADD = @gio_LIBS@ libgenerated.la
libneardal_la_LDFLAGS = -version-info @VERSION_INFO@
libneardal_la_includedir = $(includedir)/neardal
libneardal_la_include_HEADERS = neardal.h neardal_errors.h

nodist_libgenerated_la_SOURCES = \
	$(builddir)/neard_manager_proxy.c $(builddir)/neard_manager_proxy.h \
	$(builddir)/neard_tag_proxy.c $(builddir)/neard_tag_proxy.h \
	$(builddir)/neard_adapter_proxy.c $(builddir)/neard_adapter_proxy.h \
	$(builddir)/neardal_agent.c $(builddir)/neardal_agent.h \
	$(builddir)/dbus-object-manager.c $(builddir)/dbus-object-manager.h \
	$(builddir)/dbus-properties.c $(builddir)/dbus-properties.h
libgenerated_la_CFLAGS = -Wno-unused-parameter -Wno-pedantic

$(libneardal_la_SOURCES): libgenerated.la

neardal_agent.c neardal_agent.h: interface/org.neard.Agent.xml
	$(AM_V_GEN)gdbus-codegen --generate-c-code $(basename $@) $< \
	--interface-prefix org.neard. --c-namespace neardal \
	--c-generate-object-manager

define run-gdbus
$(AM_V_GEN)gdbus-codegen --generate-c-code $(basename $@) $<
endef

neard_adapter_proxy.c neard_adapter_proxy.h: interface/org.neard.Adapter.xml
	$(run-gdbus)

neard_tag_proxy.c neard_tag_proxy.h: interface/org.neard.Tag.xml
	$(run-gdbus)

neard_manager_proxy.c neard_manager_proxy.h: interface/org.neard.Manager.xml
	$(run-gdbus)

%.c: interface/%.xml
	$(run-gdbus) --interface-prefix="org.freedesktop.DBus."

CLEANFILES = $(nodist_libgenerated_la_SOURCES)

EXTRA_DIST = \
	interface/org.neard.Adapter.xml \
	interface/org.neard.Agent.xml \
	interface/org.neard.Manager.xml \
	interface/org.neard.Tag.xml \
	interface/dbus-object-manager.xml \
	interface/dbus-properties.xml
