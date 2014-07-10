AC_INIT([neardal], [0.8.0], [Neardal for Neard v0.8])
AC_CONFIG_AUX_DIR([.])
AM_INIT_AUTOMAKE([foreign])

AC_CONFIG_HEADERS([config.h])

m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([yes])])

AS_IF([test -n "${CFLAGS+set}"], [cflags_set=yes], [cflags_set=no])

AC_ARG_ENABLE([optimization],
	[AC_HELP_STRING([--disable-optimization], [disable optimization])],
	[AS_IF([test "$cflags_set" = "no" ], [CFLAGS="-O0 $CFLAGS"])],
	[AS_IF([test "$cflags_set" = "no" ], [CFLAGS="-O2 $CFLAGS"])])

AC_PROG_CC
AC_PROG_LIBTOOL

AC_ARG_ENABLE([debug],
	[AC_HELP_STRING([--enable-debug],
		[enable compiling with debugging information])],
	[CFLAGS="-DNEARDAL_DEBUG $CFLAGS"])

AS_IF([test "$ac_cv_prog_cc_g" = "yes"] && [test "$cflags_set" = "no" ],
	[CFLAGS="-g $CFLAGS"])

AC_ARG_ENABLE([trace],
	AC_HELP_STRING([--enable-trace], [compile with tracing enabled]),
	AC_MSG_NOTICE([NEARDAL will be compiled with tracing enabled])
	CFLAGS="-DNEARDAL_TRACES $CFLAGS") 

AC_MSG_NOTICE([NEARDAL will be compiled with CFLAGS=${CFLAGS}])

AC_PROG_SED	
VERSION_INFO=$(echo $VERSION | sed -e 's/\./\:/g')
AC_SUBST(VERSION_INFO)

PKG_CHECK_MODULES(gio, gio-unix-2.0 >= 2.30,
	AC_SUBST([gio_CFLAGS])
	AC_SUBST([gio_LIBS]),
	AC_MSG_ERROR(gio-unix-2.0 >= 2.30 is required))

AC_ARG_ENABLE([doc],
	AS_HELP_STRING([--enable-doc],
		[Build API documentation using Doxygen]))
AS_IF([test "$enable_doc" = "yes"],
	[AS_UNSET([enable_doc])]
	[AC_CHECK_PROG(enable_doc, doxygen, [yes], [no])]
)
AM_CONDITIONAL([HAVE_DOXYGEN], [test "$enable_doc" = "yes"])
AM_COND_IF([HAVE_DOXYGEN], [AC_CONFIG_FILES([doxygen.cfg])])

AC_CONFIG_FILES([Makefile lib/Makefile ncl/Makefile neardal.pc])
AC_OUTPUT
