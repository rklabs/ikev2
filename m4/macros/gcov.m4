dnl  *********** Add gcov support *********** 
AC_ARG_ENABLE(coverage,
  AS_HELP_STRING(
    [--enable-coverage],
    [enable coverage, default: no]),
    [case "${enableval}" in
      yes) coverage=true ;;
      no)  coverage=false ;;
      *)   AC_MSG_ERROR([bad value ${enableval} for --enable-coverage]) ;;
    esac],
    [debug=false])
AM_CONDITIONAL(IKEV2_GCOV, test x"$coverage" = x"true")
AM_COND_IF(IKEV2_GCOV,
    AC_DEFINE(IKEV2_GCOV, 1, [Define to 0 if this is a release build]),
    AC_DEFINE(IKEV2_GCOV, 0, [Define to 1 or higher if this is a debug build]))
