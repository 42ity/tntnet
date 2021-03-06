# This is upstream tntnet-2.2.1 with tweaks for 42ity project
AC_INIT([tntnet], [2.2.1], [Tommi Maekitalo <tommi@tntnet.org>])
AC_CANONICAL_TARGET

AM_INIT_AUTOMAKE

abi_current=12
abi_revision=0
abi_age=0
sonumber=${abi_current}:${abi_revision}:${abi_age}
AC_SUBST(sonumber)

AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_HEADERS([framework/common/config.h])
AC_CONFIG_SRCDIR([framework/common/tntnet.cpp])

AC_SEARCH_LIBS(gethostbyname, nsl socket resolv)
AC_SEARCH_LIBS(dlopen, dl, , AC_MSG_ERROR([dlopen not found]))
AC_SEARCH_LIBS(compress, z)

AC_PROG_CXX
AC_PROG_LIBTOOL

AC_LANG(C++)
ACX_PTHREAD

LIBS="$PTHREAD_LIBS $LIBS"
CC="$PTHREAD_CC"
CXX="$PTHREAD_CXX"
CFLAGS="$CFLAGS $PTHREAD_CFLAGS"
CXXFLAGS="$CXXFLAGS $PTHREAD_CXXFLAGS"

AX_COMPILER_VENDOR
AS_IF([test "$ax_cv_cxx_compiler_vendor" = "ibm"],
      [CPPFLAGS="$CPPFLAGS -qrtti -qlanglvl=newexcp -D__NOLOCK_ON_INPUT -D__NOLOCK_ON_OUTPUT"],
      AX_CHECK_COMPILE_FLAG([-Wno-long-long], [CPPFLAGS="$CPPFLAGS -Wno-long-long"])
      AX_CHECK_COMPILE_FLAG([-Wall], [CPPFLAGS="$CPPFLAGS -Wall"])
      AX_CHECK_COMPILE_FLAG([-pedantic], [CPPFLAGS="$CPPFLAGS -pedantic"]))

AC_CHECK_HEADER([zlib.h], , AC_MSG_ERROR([zlib not found]))
AC_CHECK_HEADER([cxxtools/net/tcpsocket.h], , AC_MSG_ERROR([cxxtools headers not found]))
AC_CHECK_FUNCS([fopen64], ,[AM_CFLAGS=-DUSE_FILE32API])
AC_SUBST(AM_CFLAGS)

AC_MSG_CHECKING([if system provides getgrouplist() with semantics we expect])
AC_COMPILE_IFELSE(
    [AC_LANG_SOURCE([
#include <grp.h>
#include <pwd.h>
struct passwd pw;
gid_t groups;
int foundgroups = 1;
int gotgroups = getgrouplist("username", pw.pw_gid, &groups, &foundgroups);
])], AC_DEFINE(HAVE_GETGROUPLIST, [1], [Defined if system provides getgrouplist() with semantics we expect])
     AC_MSG_RESULT([yes]), AC_MSG_RESULT([no]))

AC_MSG_CHECKING([if system provides setgroups() with semantics we expect])
AC_COMPILE_IFELSE(
    [AC_LANG_SOURCE([
#include <sys/types.h>
#include <unistd.h>
#include <grp.h>
#include <pwd.h>
gid_t groups;
int errcode = setgroups(1, &groups);
])], AC_DEFINE(HAVE_SETGROUPS, [1], [Defined if system provides setgroups() with semantics we expect])
     AC_MSG_RESULT([yes]), AC_MSG_RESULT([no]))

AC_ARG_WITH([epoll],
  AS_HELP_STRING([--with-epoll=yes|no|probe], [use epoll]),
  [epoll_option=$withval],
  [epoll_option=probe])

case "$epoll_option" in
  yes)
      AC_DEFINE(WITH_EPOLL, [], [Define if epoll is supported])
      ;;
  no)
      ;;
  probe)
      AC_TRY_RUN([
        #include <sys/epoll.h>
        int main() { return epoll_create(10) <= 0; }
        ],
        [AC_DEFINE(WITH_EPOLL, [], [Define if epoll is supported])])
        ;;
    *)
      AC_MSG_ERROR([unknown epoll-value $epoll_option])
      ;;
esac

AC_ARG_WITH([sendfile],
  AS_HELP_STRING([--with-sendfile=yes|no|probe], [use sendfile]),
  [sendfile_option=$withval],
  [sendfile_option=probe])

case "$sendfile_option" in
  yes)
      AC_DEFINE(WITH_SENDFILE, [], [Define if sendfile is supported])
      ;;
  no)
      ;;
  probe)
      AC_CHECK_FUNCS([sendfile])
      ;;
    *)
      AC_MSG_ERROR([unknown sendfile-value $sendfile_option])
      ;;
esac

AC_CHECK_HEADERS([sys/sendfile.h])

#
# SSL
#
AC_ARG_WITH([ssl],
  AS_HELP_STRING([--with-ssl=gnutls|openssl|no], [build with SSL support (default is openssl)]),
  [ssl_option=$withval],
  [ssl_option=openssl])

case "$ssl_option" in
    gnutls)
        PKG_CHECK_MODULES([GNUTLS], [gnutls >= 2.11], [
            AC_MSG_RESULT([building with GnuTLS support])
            AC_DEFINE(WITH_GNUTLS, [], [Define to build with gnutls])
            with_gnutls=1
          ],
          [AC_MSG_ERROR(gnutls >= 2.11 not not found)])
        ;;
    openssl|yes)
        AC_CHECK_HEADER([openssl/ssl.h], [
            AC_MSG_RESULT([building with OpenSSL support])
            AC_DEFINE(WITH_OPENSSL, [], [Define to build with openssl])
            with_openssl=1
          ],
          [AC_MSG_ERROR(openssl-headers not not found)])
        ;;
    no)
        AC_MSG_RESULT([building without ssl support])
        ;;
    *)
        AC_MSG_ERROR([unknown ssl-value $ssl_option])
        ;;
esac

AM_CONDITIONAL(MAKE_GNUTLS, test x$with_gnutls = x1)
AM_CONDITIONAL(MAKE_OPENSSL, test x$with_openssl = x1)

#
# optional components
#
AC_ARG_WITH([server],
  AS_HELP_STRING([--with-server=yes|no], [build tntnet-server (default: yes)]),
  [with_server=$withval],
  [with_server=yes])

AM_CONDITIONAL(MAKE_SERVER, test x$with_server = xyes)

AC_ARG_WITH([sdk],
  AS_HELP_STRING([--with-sdk=yes|no], [build sdk (default: yes)]),
  [with_sdk=$withval],
  [with_sdk=yes])

AM_CONDITIONAL(MAKE_SDK, test x$with_sdk = xyes)

AC_ARG_WITH([demos],
  AS_HELP_STRING([--with-demos=yes|no], [build demos (default: same as with-sdk)]),
  [with_demos=$withval],
  [with_demos=$with_sdk])

AM_CONDITIONAL(MAKE_DEMOS, test "$with_demos" = "yes" -o "$with_demos" = "install")
AM_CONDITIONAL(INSTALL_DEMOS, test "$with_demos" = "install")

if test "$with_demos" = "yes"
then
  if test "$with_sdk" != "yes"
  then
    AC_MSG_ERROR([demos can't be built without building sdk])
  fi

  AC_CHECK_PROG(ZIP, zip, 1)
  if ! test "$ZIP"; then
    AC_MSG_ERROR([zip program not found (needed for calci18n demo)])
  fi
fi

AC_ARG_WITH([stressjob],
  AS_HELP_STRING([--with-stressjob=yes|no], [support for stressjob for testing]),
  [stressjob_option=$withval],
  [stressjob_option=no])

case "$stressjob_option" in
    yes)
        AC_DEFINE(WITH_STRESSJOB, [], [Define if stressjob option is set])
        ;;
    no)
        ;;
    *)
        AC_MSG_ERROR([unknown stressjob-value $stressjob_option])
        ;;
esac

AM_CONDITIONAL(MAKE_STRESSJOB, test "$stressjob_option" = "yes")

#
# others
#
AC_CHECK_FUNCS([setenv])

case "${host_cpu}-${host_os}" in
*-aix*)
    SHARED_LIB_FLAG=-qmkshrobj
    ;;
*-darwin*)
    SHARED_LIB_FLAG=-dynamiclib
    ;;
*)
    SHARED_LIB_FLAG=-shared
    ;;
esac
AC_SUBST(SHARED_LIB_FLAG)

AC_ARG_ENABLE([unittest],
  AS_HELP_STRING([--disable-unittest], [disable unittest]),
  [enable_unittest=$enableval],
  [enable_unittest=enable_unittest])

AM_CONDITIONAL(MAKE_UNITTEST, test "$enable_unittest" = "enable_unittest")

AC_ARG_ENABLE([locale],
  AS_HELP_STRING([--disable-locale], [disable support for locales]),
  [enable_locale=$enableval],
  [enable_locale=enable_locale])

if test "$enable_locale" != "no"
then
  AC_DEFINE(ENABLE_LOCALE, [], [Defined if locale is used])
fi

AC_CONFIG_FILES([tntnet-config], [chmod +x tntnet-config])

AC_CONFIG_FILES([
    Makefile
    framework/common/Makefile
    framework/runtime/Makefile
    framework/defcomp/Makefile
    sdk/tools/common/Makefile
    sdk/tools/ecppc/Makefile
    sdk/tools/ecppl/Makefile
    sdk/tools/ecppll/Makefile
    sdk/demos/Makefile
    sdk/demos/backgroundjob/Makefile
    sdk/demos/calc/Makefile
    sdk/demos/calcajax/Makefile
    sdk/demos/calcapp/Makefile
    sdk/demos/calcmvc/Makefile
    sdk/demos/calci18n/Makefile
    sdk/demos/chat/Makefile
    sdk/demos/comp/Makefile
    sdk/demos/config/Makefile
    sdk/demos/controls/Makefile
    sdk/demos/cookie/Makefile
    sdk/demos/hello/Makefile
    sdk/demos/savepoint/Makefile
    sdk/demos/session/Makefile
    sdk/demos/sprintf/Makefile
    sdk/demos/strings/Makefile
    sdk/demos/rajax-jquery/Makefile
    sdk/demos/upload/Makefile
    utest/Makefile
    doc/man/Makefile
    etc/Makefile
    pkgconfig/tntnet.pc
    ])

AC_OUTPUT
