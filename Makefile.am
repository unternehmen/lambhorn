bin_PROGRAMS = lambhorn
lambhorn_SOURCES = lambhorn.c raster.c raster.h
AM_CPPFLAGS = $(SDL_CFLAGS) $(SDL_IMAGE_CFLAGS) $(GL_CFLAGS) $(GLU_CFLAGS)
LDADD = $(SDL_LIBS) $(SDL_IMAGE_LIBS) $(GL_LIBS) $(GLU_LIBS)
ACLOCAL_AMFLAGS = -I m4

check_PROGRAMS = \
  tests/raster/testrasterwidth0 \
  tests/raster/testrasterwidth1 \
  tests/raster/testrasterwidth7 \
  tests/raster/testrasterwidth8 \
  tests/raster/testrasterwidth9 \
  tests/raster/testrasterchunkbottomleft \
  tests/raster/testrasterchunkbottomright \
  tests/raster/testrasterchunktopleft \
  tests/raster/testrasterchunktopright
rastermodule = raster.c raster.h
tests_raster_testrasterwidth0_SOURCES = tests/raster/testrasterwidth0.c $(rastermodule)
tests_raster_testrasterwidth1_SOURCES = tests/raster/testrasterwidth1.c $(rastermodule)
tests_raster_testrasterwidth7_SOURCES = tests/raster/testrasterwidth7.c $(rastermodule)
tests_raster_testrasterwidth8_SOURCES = tests/raster/testrasterwidth8.c $(rastermodule)
tests_raster_testrasterwidth9_SOURCES = tests/raster/testrasterwidth9.c $(rastermodule)
tests_raster_testrasterchunkbottomleft_SOURCES = tests/raster/testrasterchunkbottomleft.c $(rastermodule)
tests_raster_testrasterchunkbottomright_SOURCES = tests/raster/testrasterchunkbottomright.c $(rastermodule)
tests_raster_testrasterchunktopleft_SOURCES = tests/raster/testrasterchunktopleft.c $(rastermodule)
tests_raster_testrasterchunktopright_SOURCES = tests/raster/testrasterchunktopright.c $(rastermodule)
TESTS = $(check_PROGRAMS)
