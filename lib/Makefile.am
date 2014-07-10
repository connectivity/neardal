AM_CFLAGS = @gio_CFLAGS@ @NEARDAL_EXTRA_FLAGS@

lib_LTLIBRARIES = libneardal.la
noinst_LTLIBRARIES = libgenerated.la

libneardal_la_SOURCES = \
	neardal.c neardal_adapter.c neardal_agent_mgr.c neardal_device.c \
	neardal_manager.c neardal_record.c neardal_tag.c neardal_tools.c \
	neardal_traces.c
libneardal_la_LIBADD = @gio_LIBS@ libgenerated.la
libneardal_la_LDFLAGS = -version-info @VERSION_INFO@
libneardal_la_includedir = $(includedir)/neardal
libneardal_la_include_HEADERS = neardal.h neardal_errors.h

libgenerated_la_SOURCES = \
	neard_manager_proxy.c neard_manager_proxy.h \
	neard_record_proxy.c neard_record_proxy.h \
	neard_tag_proxy.c neard_tag_proxy.h \
	neard_device_proxy.c neard_device_proxy.h \
	neard_adapter_proxy.c neard_adapter_proxy.h \
	neardal_agent.c neardal_agent.h \
	dbus-object-manager.c dbus-object-manager.h \
	dbus-properties.c dbus-properties.h
libgenerated_la_CFLAGS = $(AM_CFLAGS) -Wno-unused-parameter -Wno-pedantic

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

neard_record_proxy.c neard_record_proxy.h: interface/org.neard.Record.xml
	$(run-gdbus)

neard_tag_proxy.c neard_tag_proxy.h: interface/org.neard.Tag.xml
	$(run-gdbus)

neard_manager_proxy.c neard_manager_proxy.h: interface/org.neard.Manager.xml
	$(run-gdbus)

neard_device_proxy.c neard_device_proxy.h: interface/org.neard.Device.xml
	$(run-gdbus)

%.c: interface/%.xml
	$(run-gdbus) --interface-prefix="org.freedesktop.DBus."

CLEANFILES = $(libgenerated_la_SOURCES)
