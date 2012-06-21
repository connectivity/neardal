neardal_LT_CURRENT = 0
neardal_LT_REVISION = 1
neardal_LT_AGE = 0

#Rule to generate the binding headers
neard_manager_proxy.h:  interface/org.neard.Manager.xml
	dbus-binding-tool --mode=glib-client $<> $@
	
neard_adapter_proxy.h:  interface/org.neard.Adapter.xml
	dbus-binding-tool --mode=glib-client $<> $@

neard_tag_proxy.h:  	interface/org.neard.Tag.xml
	dbus-binding-tool --mode=glib-client $<> $@

neard_record_proxy.h:  interface/org.neard.Record.xml
	dbus-binding-tool --mode=glib-client $<> $@

BUILT_SOURCES =	neard_manager_proxy.h \
		neard_adapter_proxy.h \
		neard_tag_proxy.h \
		neard_record_proxy.h

neardal_PRV_HDR = \
		neardal_adapter.h \
		neardal_manager.h \
		neardal_prv.h \
		neardal_record.h \
		neardal_tag.h \
		neardal_traces_prv.h \
		neardal_tools.h

# We don't want to install this header
noinst_HEADERS = $(BUILT_SOURCES) $(neardal_PRV_HDR)

# We want to install these headers
dist_neardalheaders_HEADERS =	neardal.h \
				neardal_errors.h

CLEANFILES = $(BUILT_SOURCES)
EXTRA_DIST = interface/org.neard.Manager.xml interface/org.neard.Adapter.xml interface/org.neard.Tag.xml interface/org.neard.Record.xml

lib_LTLIBRARIES =		libneardal.la
libneardal_la_SOURCES =	neardal.c \
				neardal_adapter.c \
				neard_adapter_proxy.c \
				neardal_errors.c \
				neardal_manager.c \
				neard_manager_proxy.c \
				neardal_record.c \
				neard_record_proxy.c \
				neardal_tag.c \
				neard_tag_proxy.c \
				neardal_tools.c \
				neardal_traces.c

libneardal_la_LDFLAGS = -version-info $(neardal_LT_CURRENT):$(neardal_LT_REVISION):$(neardal_LT_AGE)
libneardal_la_CFLAGS = $(GLIB_CFLAGS) $(GIO_UNIX_CFLAGS)

libneardal_la_LIBADD = $(GLIB_LIBS) $(GIO_UNIX_LIBS)

neardalheadersdir = $(includedir)/neardal



