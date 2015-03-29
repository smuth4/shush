#
# $Id: configure.ac 1404 2008-03-08 23:25:46Z kalt $
#
AC_PREREQ(2.59)
AC_INIT([Shush], [], [shush-bugs@taranis.org], [shush])
AC_REVISION([$Id: configure.ac 1404 2008-03-08 23:25:46Z kalt $])
AC_CONFIG_SRCDIR([src/os.h])
AC_CONFIG_HEADER([src/config.h])

# Checks for programs.
AC_PROG_CC
test "x$GCC" = "xyes" && CFLAGS="$CFLAGS -fno-builtin-log"
AC_PROG_INSTALL
AC_PATH_PROG([SENDMAIL], [sendmail], [NOSENDMAIL], [/usr/lib:/usr/sbin:$PATH])
AC_DEFINE_UNQUOTED([SENDMAIL], ["$SENDMAIL"], [Path to sendmail program])
AC_PATH_PROG([CRONTAB], [crontab], [NOCRONTAB])
AC_DEFINE_UNQUOTED([CRONTAB], ["$CRONTAB"], [Path to crontab program])

# Pthreads, please..
sinclude(acx_pthread.m4)
ACX_PTHREAD([AC_DEFINE(HAVE_PTHREAD,1,[Define if you have POSIX threads libraries and header files.])
LIBS="$PTHREAD_LIBS $LIBS"
CFLAGS="$CFLAGS $PTHREAD_CFLAGS"
CC="$PTHREAD_CC"])

# Custom arguments
AC_ARG_WITH([pcre], AC_HELP_STRING([--with-pcre=DIR], [Support Perl Compatible Regular Expressions (default)]))
AC_ARG_WITH([pcre], AC_HELP_STRING([--without-pcre], [Do not support Perl Compatible Regular Expressions]))
if test "x$with_pcre" != "xno"; then
   if test "x$with_pcre" != "xyes" -a "x$with_pcre" != "x"; then
      if test ! -d "$with_pcre"; then
	 AC_MSG_ERROR([Invalid path for pcre: $with_pcre])
      fi
      CPPFLAGS="$CPPFLAGS -I$with_pcre/include"
      LDFLAGS="$LDFLAGS -L$with_pcre/lib"
   fi
fi

AC_ARG_WITH([syslog], AC_HELP_STRING([--with-syslog=FACILITY], [Define the default syslog facility used]))
AC_ARG_WITH([syslog], AC_HELP_STRING([--without-syslog], [Disable default use of syslog]))
if test "x$with_syslog" != "xno"; then
   if test "x$with_syslog" != "xyes" -a "x$with_syslog" != "x"; then
      AC_DEFINE_UNQUOTED([SYSLOG], "$with_syslog", [Define to a syslog facility to enable logging via syslog by default])
   else
      AC_DEFINE([SYSLOG], "cron")
   fi
else
      AC_DEFINE([SYSLOG], "")
fi

AC_ARG_WITH([sizelimit], AC_HELP_STRING([--with-sizelimit=SIZE], [Define the default output size limit for reports]))
if test "x$with_sizelimit" = "xyes"; then
   AC_MSG_ERROR([A size limit MUST be specified to set default!])
elif test "x$with_sizelimit" != "xno" -a "x$with_sizelimit" != "x"; then
   AC_DEFINE_UNQUOTED([MAXSZ], "$with_sizelimit", [Default MAXSZ value])
else
   AC_DEFINE([MAXSZ], "1000k")
fi

# Checks for libraries.
AC_SEARCH_LIBS([basename], [gen])
if test "x$with_pcre" != "xno"; then
   AC_SEARCH_LIBS([pcre_compile], [pcre], ,
	AC_MSG_WARN([Perl Compatible Regular Expressions library is missing.])
	with_pcre="no")
fi
AC_SEARCH_LIBS([MD5Data], [md])
AC_SEARCH_LIBS([md5_calc], [md5])

# Checks for header files.
AC_CHECK_HEADERS([md5.h paths.h pthread.h])
if test "x$with_pcre" != "xno"; then
   AC_CHECK_HEADERS([pcre.h])
fi

# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_PID_T
AC_CHECK_DECLS([sys_signame], , , [#include <signal.h>])

# Checks for library functions.
AC_CHECK_FUNCS([strlcpy MD5Data md5_calc])

AC_CONFIG_FILES([src/Makefile Makefile])
AC_OUTPUT
