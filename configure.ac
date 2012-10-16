AC_INIT([libneardal], [0.1.0])
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
	CFLAGS="$CFLAGS -g -O0"
 	echo 'NEARDAL will be compiled in debug mode. CFLAGS='${CFLAGS}
else
	CFLAGS="$CFLAGS -O2"
 	echo 'NEARDAL will not be compiled in debug mode. CFLAGS='${CFLAGS}
fi

## Traces ?
AC_ARG_ENABLE(traces, AC_HELP_STRING([--disable-traces],
			[remove traces while compiling]), [
 	if (test "${enableval}" = "no"); then
 		traces=no
 		echo 'NEARDAL will be compiled without traces... ('NEARDAL_TRACES' will not be defined)'
 	fi
])
if (test "${traces}" = "yes"); then
	CFLAGS="$CFLAGS -DNEARDAL_TRACES"
 	echo 'NEARDAL will be compiled with traces...('NEARDAL_TRACES' will be defined)'
fi

## Check dependances
PKG_CHECK_MODULES(GLIB, glib-2.0 >= 2.22, dummy=yes,
				AC_MSG_ERROR(GLib >= 2.22 is required))
AC_SUBST(GLIB_CFLAGS)
AC_SUBST(GLIB_LIBS)

PKG_CHECK_MODULES(DBUS, dbus-glib-1, [have_dbus="yes"], [have_dbus="no"])
AC_SUBST(DBUS_CFLAGS)
AC_SUBST(DBUS_LIBS)

AC_CHECK_PROGS([DOXYGEN], [doxygen])
if test -z "$DOXYGEN";
   then AC_MSG_WARN([Doxygen not found - continuing without Doxygen support])
fi
AM_CONDITIONAL([HAVE_DOXYGEN],
[test -n "$DOXYGEN"])AM_COND_IF([HAVE_DOXYGEN], [AC_CONFIG_FILES([doxygen.cfg])])

AC_CONFIG_FILES([Makefile lib/Makefile ncl/Makefile neardal.pc])


AC_OUTPUT
