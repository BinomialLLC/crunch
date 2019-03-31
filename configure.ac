dnl Build system for Unix/macOS.
dnl For Windows, please use visual studio to build, not this autotools

AC_PREREQ([2.69])
AC_INIT([crnlib], [1.04], [info@binomial.info])
AC_CONFIG_MACRO_DIR([m4])
AM_INIT_AUTOMAKE([-Wall -Werror foreign subdir-objects])
AC_CONFIG_HEADERS([config.h])
AC_CANONICAL_HOST
AM_SILENT_RULES([yes])

AM_PROG_AR

AM_ENABLE_SHARED
AM_ENABLE_STATIC

AC_CHECK_HEADERS([limits.h \
                  malloc.h \
                  memory.h \
                  stddef.h \
                  stdlib.h \
                  string.h \
                  termios.h \
                  unistd.h \
                  utime.h \
                  cstring \
                  math.h \
                  assert.h \
                  deque \
                  sys/timex.h \
                  memory.h \
                  stdarg.h \
                  termios.h \
                  unistd.h \
                  sys/sysinfo.h \
                  process.h \
                  sys/stat.h \
                  sys/utime.h \
                  fnmatch.h \
                  dirent.h \
                  pthread.h \
                  locale \
                  limits.h \
                  algorithm \
                  errno.h \
                  map \
                  setjmp.h \
                  libgen.h \
                  minmax.h \
                  new \
                  intrin.h \
                  semaphore.h])

LT_INIT

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC
AC_PROG_INSTALL
AC_PROG_MAKE_SET

# Checks for libraries.
# FIXME: Replace `main' with a function in `-lpthread':
AC_CHECK_LIB([pthread], [pthread_mutex_lock])

# Checks for typedefs, structures, and compiler characteristics.
AC_CHECK_HEADER_STDBOOL
AC_C_INLINE
AC_TYPE_SIZE_T
AC_CHECK_TYPES([ptrdiff_t])

# Checks for library functions.
AC_FUNC_ERROR_AT_LINE
AC_FUNC_MALLOC
AC_FUNC_MKTIME
AC_FUNC_REALLOC
AC_CHECK_FUNCS([floor gettimeofday memchr memmove memset mkdir pow realpath sqrt strchr strtol utime])

AC_CONFIG_FILES([Makefile])
AC_OUTPUT
