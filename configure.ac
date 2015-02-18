dnl Process this file with autoconf to produce a configure script.
dnl Created by Anjuta application wizard.

AC_INIT(ikev2, 1.0)

AC_CONFIG_HEADERS([config.h])

AM_INIT_AUTOMAKE([1.9 foreign subdir-objects])

AC_CONFIG_MACRO_DIR([m4])

dnl Custom macro to check openssl libraries and headers
m4_include(m4/macros/openssl.m4)
AX_CHECK_OPENSSL()

dnl ********** enable verbose make ******************************
AM_SILENT_RULES([no])

dnl Required since -g -O2 flags are always set by default
if test -z $CXXFLAGS; then
    CXXFLAGS='-O2'
fi

AC_PROG_CXX

dnl ***************************************************************************
dnl Internationalization
dnl ***************************************************************************
IT_PROG_INTLTOOL([0.35.0])

GETTEXT_PACKAGE=ikev2
AC_SUBST(GETTEXT_PACKAGE)
AC_DEFINE_UNQUOTED(GETTEXT_PACKAGE,"$GETTEXT_PACKAGE", [GETTEXT package name])
AM_GLIB_GNU_GETTEXT

dnl ************ rkadam Adding C++11 support ********************
AC_SUBST([AM_CXXFLAGS], ["-std=c++11"])

dnl ********************** libtool support *************************
LT_INIT

m4_include(m4/macros/debug.m4)
m4_include(m4/macros/gcov.m4)

AC_OUTPUT([
    Makefile
    src/Makefile
    test/Makefile
    po/Makefile.in
])
