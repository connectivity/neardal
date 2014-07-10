noinst_PROGRAMS=ncl

ncl_SOURCES =	ncl.c \
		ncl_cmd.c

noinst_HEADERS = ncl.h

ncl_CFLAGS = @gio_CFLAGS@ -I../lib
ncl_LDADD = @gio_LIBS@ ../lib/libneardal.la
