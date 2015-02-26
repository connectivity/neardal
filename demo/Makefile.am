AM_CPPFLAGS = @gio_CFLAGS@ -I$(top_builddir)/lib -I$(top_srcdir)/lib

noinst_PROGRAMS=demo

demo_SOURCES = \
	$(srcdir)/demo.c

demo_LDADD = @gio_LIBS@ -L$(top_builddir)/lib -lneardal
