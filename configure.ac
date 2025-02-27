AC_PREREQ(2.59)
AC_INIT([libroad],[0.01],[jasonsewall@gmail.com])
AC_CONFIG_AUX_DIR([config])
AM_INIT_AUTOMAKE([-Wall -Werror foreign])
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_SRCDIR([libroad/road_rep.hpp])
AC_CONFIG_HEADERS([config.h])
PKG_PROG_PKG_CONFIG

AC_LANG([C++])

AC_PROG_LIBTOOL

AC_ARG_ENABLE([tvmet-optimize],
  [AS_HELP_STRING([--enable-tvmet-optimize],
    [turn on tvmet's optimization flags @<:@default=no@:>@])],
  [tvmet_optimize=$enableval],
  [tvmet_optimize=no])

AC_ARG_ENABLE([boost-optimize],
  [AS_HELP_STRING([--enable-boost-optimize],
    [turn on boost's optimization flags @<:@default=no@:>@])],
  [boost_optimize=$enableval],
  [boost_optimize=no])

GIT_VERSION=`./GIT-VERSION-GEN`
AC_SUBST(GIT_VERSION)
AH_TEMPLATE([GIT_VERSION],
	    [Git revision of this build])

AC_DEFINE_UNQUOTED(GIT_VERSION, "$GIT_VERSION")

AH_TEMPLATE([HOSTNAME],
	    [Build system])

AC_DEFINE_UNQUOTED(HOSTNAME, "$HOSTNAME")

BUILD_DATE=`date --rfc-3339=seconds`
AC_SUBST(BUILD_DATE)
AH_TEMPLATE([BUILD_DATE],
	    [Build date (actually configuration date)])

AC_DEFINE_UNQUOTED(BUILD_DATE, "$BUILD_DATE")

AH_TEMPLATE([CONFIGURE_ARGS],
	    [Arguments passed to ./configure])
AC_DEFINE_UNQUOTED(CONFIGURE_ARGS, "$ac_configure_args")

AC_ARG_ENABLE([visual],
  [AS_HELP_STRING([--enable-visual],
    [build graphical viewer components @<:@default=check@:>@])],
  [enable_visual=$enableval],
  [enable_visual=check])

AC_ARG_ENABLE([images],
  [AS_HELP_STRING([--enable-images],
    [build components that write images @<:@default=check@:>@])],
  [enable_images=$enableval],
  [enable_images=check])

# Checks for programs.
AC_PROG_CXX
LT_INIT
PKG_CHECK_MODULES(LIBXMLPP, libxml++-2.6 >= 2.10.0)
PKG_CHECK_MODULES(GLIBMM, glibmm-2.4 >= 2.12.0)

AC_CHECK_PROG(have_tvmet_conf, tvmet-config, [yes], [no])
AS_IF([test x"$have_tvmet_conf" != xno],
	    [TVMET_CFLAGS="`tvmet-config --cxxflags` `tvmet-config --includes `"
	    TVMET_LIBS="`tvmet-config --libs`"
	    AC_SUBST([TVMET_CFLAGS])
	    AC_SUBST([TVMET_LIBS])],
       [AC_MSG_ERROR([Could not find tvmet-config. Please install tvmet!])])

AS_IF([test x"$tvmet_optimize" != xno],
      [TVMET_CFLAGS+=" -finline-limit=5000 --param max-inline-insns-single=5000 --param inline-unit-growth=5000 --param large-function-growth=5000 -DTVMET_OPTIMIZE -O2 -ftemplate-depth-200 -fomit-frame-pointer"],
      [TVMET_CFLAGS+=" -DTVMET_DEBUG"])

AS_IF([test x"$enable_visual" != xno],
            [AC_CHECK_PROG(have_fltk, fltk-config, [yes], [no])
	     PUSH_LIBS=$LIBS
	     LIBS=
	     AC_SEARCH_LIBS([glewInit], [GLEW], [have_glew=yes], [have_glew=no])
	     GLEW_LIBS=$LIBS
	     LIBS=$PUSH_LIBS
             AS_IF([test \( x"$have_fltk" = xno -o x"$have_glew" = xno \) -a x"$enable_visual" != xcheck],
             AC_MSG_FAILURE([--enable-visual given but test for fltk-config or glew failed!]))])

AS_IF([test x"$enable_visual" != xno -a x"$have_fltk" = xyes -a x"$have_glew" = xyes],
            [visual_ok=yes
             FLTK_CXXFLAGS=`fltk-config --cxxflags`
             FLTK_LDFLAGS=`fltk-config --ldflags`
             FLTK_GL_CXXFLAGS=`fltk-config --cxxflags --use-gl`
             FLTK_GL_LDFLAGS=`fltk-config --ldflags --use-gl`
             AC_SUBST([FLTK_CXXFLAGS])
             AC_SUBST([FLTK_LDFLAGS])
             AC_SUBST([FLTK_GL_CXXFLAGS])
             AC_SUBST([FLTK_GL_LDFLAGS])
	     AC_SUBST([GLEW_LIBS])],
      [visual_ok=no])

AM_CONDITIONAL(DO_VISUAL, test x"$visual_ok" = xyes)

AH_TEMPLATE([HAVE_CAIRO],
	    [Define to 1 if you have <cairo.h>])

AS_IF([test x"$enable_images" != xno],
      [AC_CHECK_PROG(have_magickpp, Magick++-config, [yes], [no])
       AS_IF([test x"$have_magickpp" = xyes],
             [MAGICKPP_CXXFLAGS=`Magick++-config --cppflags`
              MAGICKPP_LDFLAGS=`Magick++-config --libs`
              AC_SUBST([MAGICKPP_CXXFLAGS])
              AC_SUBST([MAGICKPP_LDFLAGS])
	      PKG_CHECK_MODULES([CAIRO], [cairo > 1.2],
      	                        [image_ok=yes
				 AC_DEFINE([HAVE_CAIRO], [1])]
                                [AS_IF([test x"$enable_images" != xcheck],
	 		               AC_MSG_FAILURE([--enable-images given but test for cairo failed!]),
				       [image_ok=yes])])],
             [AC_MSG_FAILURE([--enable-images given but test for Magick++ failed!])])],
      [image_ok=no])

AM_CONDITIONAL(DO_IMAGE, test x"$image_ok" = xyes)

# Checks for libraries.
BOOST_REQUIRE([1.39])
BOOST_FOREACH
BOOST_UTILITY
BOOST_TOKENIZER
BOOST_FORMAT
BOOST_IOSTREAMS
BOOST_CONVERSION
BOOST_SYSTEM
BOOST_FILESYSTEM

AS_IF([test x"$boost_optimize" != xno],
      [BOOST_CPPFLAGS+=" -DBOOST_DISABLE_ASSERTS"],
      [])

# Checks for header files.

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_C_INLINE
AC_TYPE_SIZE_T

# Checks for library functions.
AC_FUNC_MALLOC
AC_SEARCH_LIBS([clock_gettime], [rt])
AC_CHECK_FUNCS([floor memset pow sqrt strdup])

AC_CONFIG_FILES([Makefile \
                 libroad/Makefile \
		 test/Makefile \
		 etc/Makefile \
		 etc/arc-chop/Makefile
		])
AC_OUTPUT

echo "
  $PACKAGE_NAME version $PACKAGE_VERSION-$GIT_VERSION
  Configuration host.......: $HOSTNAME
  Configuration date.......: $BUILD_DATE
  Configuration flags......: $ac_configure_args
  Prefix...................: $prefix
  TVMET_CFLAGS.............: $TVMET_CFLAGS
  TVMET_LIBS...............: $TVMET_LIBS
  GLIBMM_CFLAGS............: $GLIBMM_CFLAGS
  GLIBMM_LIBS..............: $GLIBMM_LIBS
  LIBXMLPP_CFLAGS..........: $LIBXMLPP_CFLAGS
  LIBXMLPP_LIBS............: $LIBXMLPP_LIBS
  BOOST_CPPFLAGS...........: $BOOST_CPPFLAGS
  BOOST_SYSTEM_LIBS:.......: $BOOST_SYSTEM_LIBS
  BOOST_SYSTEM_LDFLAGS.....: $BOOST_SYSTEM_LDFLAGS
  BOOST_FILESYSTEM_LIBS:...: $BOOST_FILESYSTEM_LIBS
  BOOST_FILESYSTEM_LDFLAGS.: $BOOST_FILESYSTEM_LDFLAGS
  BOOST_IOSTREAMS_LIBS.....: $BOOST_IOSTREAMS_LIBS
  BOOST_IOSTREAMS_LDFLAGS..: $BOOST_IOSTREAMS_LDFLAGS
  C++ Compiler.............: $CXX $CXXFLAGS $CPPFLAGS
  Linker...................: $LD $LDFLAGS $LIBS"
if test x"$visual_ok" = xyes; then
echo "
  Building visual: yes
    FLTK_CXXFLAGS...: $FLTK_CXXFLAGS
    FLTK_LDFLAGS....: $FLTK_LDFLAGS
    FLTK_GL_CXXFLAGS: $FLTK_GL_CXXFLAGS
    FLTK_GL_LDFLAGS.: $FLTK_GL_LDFLAGS
    GLEW_LIBS.......: $GLEW_LIBS"
else
echo "
  Building visual: no"
fi
if test x"$image_ok" = xyes; then
echo "
  Building images: yes
    CAIRO_CFLAGS......: $CAIRO_CFLAGS
    CAIRO_LIBS........: $CAIRO_LIBS
    MAGICKPP_CXXFLAGS.: $MAGICKPP_CXXFLAGS
    MAGICKPP_LDFLAGS..: $MAGICKPP_LDFLAGS"
else
echo "
  Building images: no"
fi
echo "
  C++ Compiler...: $CXX $CXXFLAGS $CPPFLAGS
  Linker.........: $LD $LDFLAGS $LIBS
"
