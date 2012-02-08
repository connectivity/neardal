neardal_LT_CURRENT = 0
neardal_LT_REVISION = 1
neardal_LT_AGE = 0

#Rule to generate the binding headers
neard-manager-proxy.h:  interface/org.neard.Manager.xml
	dbus-binding-tool --mode=glib-client $<> $@

neard-adapter-proxy.h:  interface/org.neard.Adapter.xml
	dbus-binding-tool --mode=glib-client $<> $@

neard-target-proxy.h:  interface/org.neard.Target.xml
	dbus-binding-tool --mode=glib-client $<> $@

neard-record-proxy.h:  interface/org.neard.Record.xml
	dbus-binding-tool --mode=glib-client $<> $@

BUILT_SOURCES =	neard-manager-proxy.h \
		neard-adapter-proxy.h \
		neard-target-proxy.h \
		neard-record-proxy.h

neardal_PRV_HDR = \
		neardal_adapter.h \
		neardal_manager.h \
		neardal_prv.h \
		neardal_record.h \
		neardal_target.h \
		neardal_tools.h

# We don't want to install this header
noinst_HEADERS = $(BUILT_SOURCES) $(neardal_PRV_HDR)

# We want to install these headers
dist_neardalheaders_HEADERS =	neardal.h \
				neardal_errors.h

CLEANFILES = $(BUILT_SOURCES)
EXTRA_DIST = interface/org.neard.Manager.xml interface/org.neard.Adapter.xml interface/org.neard.Target.xml interface/org.neard.Record.xml

lib_LTLIBRARIES =		libneardal.la
libneardal_la_SOURCES =	neardal.c \
				neardal_adapter.c \
				neardal_errors.c \
				neardal_manager.c \
				neardal_record.c \
				neardal_target.c \
				neardal_tools.c \
				neardal_traces.c

libneardal_la_LDFLAGS = -version-info $(neardal_LT_CURRENT):$(neardal_LT_REVISION):$(neardal_LT_AGE)
libneardal_la_CFLAGS = $(GLIB_CFLAGS) $(DBUS_CFLAGS)

libneardal_la_LIBADD = $(GLIB_LIBS) $(DBUS_LIBS)

neardalheadersdir = $(includedir)/neardal



