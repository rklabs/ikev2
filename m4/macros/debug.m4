dnl  *********** Add debug support  *********** 
AC_ARG_ENABLE(debug,
  AS_HELP_STRING(
    [--enable-debug],
    [enable debugging, default: no]),
    [case "${enableval}" in
      yes) debug=true ;;
      no)  debug=false ;;
      *)   AC_MSG_ERROR([bad value ${enableval} for --enable-debug]) ;;
    esac],
    [debug=false])
AM_CONDITIONAL(IKEV2_DBG, test x"$debug" = x"true")
AM_COND_IF(IKEV2_DBG,
    AC_DEFINE(IKEV2_DBG, 1, [Define to 0 if this is a release build]),
    AC_DEFINE(IKEV2_DBG, 0, [Define to 1 or higher if this is a debug build]))
