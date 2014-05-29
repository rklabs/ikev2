dnl Process this file with autoconf to produce a configure script.
dnl Created by Anjuta application wizard.

AC_INIT(ikev2, 1.0)

AC_CONFIG_HEADERS([config.h])

AM_INIT_AUTOMAKE([subdir-objects -Wall -Werror dist-bzip2])
AC_CONFIG_MACRO_DIR([m4])

dnl ********** rkadam enable verbose make ******************************
AM_SILENT_RULES([no])

AC_PROG_CXX

dnl ***************************************************************************
dnl Internationalization
dnl ***************************************************************************
IT_PROG_INTLTOOL([0.35.0])

GETTEXT_PACKAGE=ikev2
AC_SUBST(GETTEXT_PACKAGE)
AC_DEFINE_UNQUOTED(GETTEXT_PACKAGE,"$GETTEXT_PACKAGE", [GETTEXT package name])
AM_GLIB_GNU_GETTEXT

dnl ************ rkadam Adding this for C++11 support ********************
AC_SUBST([AM_CXXFLAGS], ["-std=c++11"])

AC_CHECK_HEADERS(iostream)
AC_CHECK_HEADERS(vector)
AC_CHECK_HEADERS(regex)

LT_INIT

AC_OUTPUT([
Makefile
src/Makefile
po/Makefile.in
])
