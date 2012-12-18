AC_INIT([neardal], 0.8.0, Neardal for Neard v0.8)
AM_INIT_AUTOMAKE([foreign -Wall -Werror])

AC_PROG_LIBTOOL
AC_CONFIG_HEADERS([config.h])

m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([yes])])

## Default values
CFLAGS="-W -Wall"
## Compiled in debug mode
debug=yes
## Compiled with traces
traces=no

AC_PROG_CC
AM_PROG_CC_C_O

## Compiler option
## Debug ?
AC_ARG_ENABLE(debug, AC_HELP_STRING([--disable-debug],
			[disable compiling with debugging information]), [
	if (test "${enableval}" = "no" &&
				test "${ac_cv_prog_cc_g}" = "yes"); then
		debug=no
	fi
])

if (test "${debug}" = "yes"); then
	CFLAGS="$CFLAGS -g -O0 -DNEARDAL_DEBUG"
	echo 'NEARDAL will be compiled in debug mode.(NEARDAL_DEBUG will be defined)'
else
	CFLAGS="$CFLAGS -O2"
	echo 'NEARDAL will not be compiled in debug mode. (NEARDAL_DEBUG will not be defined)'
fi

## Traces ?
AC_ARG_ENABLE(traces, AC_HELP_STRING([--enable-traces],
			[add debugging traces while compiling]), [
 	if (test "${enableval}" = "yes"); then
 		traces=yes
 	fi
])
if (test "${traces}" = "yes"); then
	CFLAGS="$CFLAGS -DNEARDAL_TRACES"
 	echo "NEARDAL will be compiled with debugging traces...(NEARDAL_TRACES will be defined)"
else
	echo "NEARDAL will be compiled without debugging traces... (NEARDAL_TRACES will not be defined)"
fi
echo 'NEARDAL will be compiled with CFLAGS='${CFLAGS}

## Check dependances
GLIB_REQUIRED=2.30.0
PKG_CHECK_MODULES(GLIB, glib-2.0 >= $GLIB_REQUIRED, dummy=yes,
				AC_MSG_ERROR(GLib >= $GLIB_REQUIRED is required))
AC_SUBST(GLIB_CFLAGS)
AC_SUBST(GLIB_LIBS)

PKG_CHECK_MODULES(GIO_UNIX, gio-unix-2.0 >= $GLIB_REQUIRED, dummy=yes,
				AC_MSG_ERROR(gio-unix >= $GLIB_REQUIRED is required))
AC_SUBST(GIO_UNIX_CFLAGS)
AC_SUBST(GIO_UNIX_LIBS)

AC_CHECK_PROGS([DOXYGEN], [doxygen])
if test -z "$DOXYGEN";
   then AC_MSG_WARN([Doxygen not found - continuing without Doxygen support])
fi
AM_CONDITIONAL([HAVE_DOXYGEN],
[test -n "$DOXYGEN"])AM_COND_IF([HAVE_DOXYGEN], [AC_CONFIG_FILES([doxygen.cfg])])

AC_CONFIG_FILES([Makefile lib/Makefile ncl/Makefile neardal.pc])


AC_OUTPUT
