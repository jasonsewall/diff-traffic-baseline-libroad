noinst_PROGRAMS = road-test interval-test sumo-test hwm-test sumo-xml-to-hwm svg-write make-grid osm-import qaatsi-grid

EXTRA_DIST = arcball.hpp visual_geometric.hpp

road_test_SOURCES  = road-test.cpp
road_test_CPPFLAGS = $(GLIBMM_CFLAGS) $(LIBXMLPP_CFLAGS) $(CAIRO_CFLAGS) $(BOOST_CPPFLAGS) $(TVMET_CFLAGS) $(CXXFLAGS) -I$(top_srcdir)
road_test_LDFLAGS  = $(LDFLAGS)
road_test_LDADD    = $(top_builddir)/libroad/libroad.la

interval_test_SOURCES  = interval-test.cpp
interval_test_CPPFLAGS = $(GLIBMM_CFLAGS) $(LIBXMLPP_CFLAGS) $(CAIRO_CFLAGS) $(BOOST_CPPFLAGS) $(CXXFLAGS) -I$(top_srcdir)
interval_test_LDFLAGS  = $(LDFLAGS)
interval_test_LDADD    = $(top_builddir)/libroad/libroad.la

sumo_test_SOURCES  = sumo-test.cpp
sumo_test_CPPFLAGS = $(GLIBMM_CFLAGS) $(LIBXMLPP_CFLAGS) $(CAIRO_CFLAGS) $(BOOST_CPPFLAGS) $(TVMET_CFLAGS) $(CXXFLAGS) -I$(top_srcdir)
sumo_test_LDFLAGS  = $(LDFLAGS)
sumo_test_LDADD    = $(top_builddir)/libroad/libroad.la

hwm_test_SOURCES  = hwm-test.cpp
hwm_test_CPPFLAGS = $(GLIBMM_CFLAGS) $(LIBXMLPP_CFLAGS) $(CAIRO_CFLAGS) $(BOOST_CPPFLAGS) $(TVMET_CFLAGS) $(CXXFLAGS) -I$(top_srcdir)
hwm_test_LDFLAGS  = $(LDFLAGS)
hwm_test_LDADD    = $(top_builddir)/libroad/libroad.la

sumo_xml_to_hwm_SOURCES  = sumo-xml-to-hwm.cpp
sumo_xml_to_hwm_CPPFLAGS = $(GLIBMM_CFLAGS) $(LIBXMLPP_CFLAGS) $(CAIRO_CFLAGS) $(BOOST_CPPFLAGS) $(TVMET_CFLAGS) $(CXXFLAGS) -I$(top_srcdir)
sumo_xml_to_hwm_LDFLAGS  = $(LDFLAGS)
sumo_xml_to_hwm_LDADD    = $(top_builddir)/libroad/libroad.la

svg_write_SOURCES  = svg-write.cpp
svg_write_CPPFLAGS = $(GLIBMM_CFLAGS) $(LIBXMLPP_CFLAGS) $(CAIRO_CFLAGS) $(BOOST_CPPFLAGS) $(TVMET_CFLAGS) $(CXXFLAGS) -I$(top_srcdir)
svg_write_LDFLAGS  = $(LDFLAGS)
svg_write_LDADD    = $(top_builddir)/libroad/libroad.la

make_grid_SOURCES = make-grid.cpp
make_grid_CPPFLAGS = $(GLIBMM_CFLAGS) $(LIBXMLPP_CFLAGS) $(CAIRO_CFLAGS) $(BOOST_CPPFLAGS) $(TVMET_CFLAGS) $(CXXFLAGS) -I$(top_srcdir)
make_grid_LDFLAGS  = $(LDFLAGS)
make_grid_LDADD    = $(top_builddir)/libroad/libroad.la

osm_import_SOURCES = osm-import.cpp
osm_import_CPPFLAGS = $(GLIBMM_CFLAGS) $(LIBXMLPP_CFLAGS) $(CAIRO_CFLAGS) $(BOOST_CPPFLAGS) $(TVMET_CFLAGS) $(CXXFLAGS) -I$(top_srcdir)
osm_import_LDFLAGS  = $(LDFLAGS)
osm_import_LDADD    = $(top_builddir)/libroad/libroad.la

qaatsi_grid_SOURCES  = qaatsi-grid.cpp
qaatsi_grid_CPPFLAGS = $(GLIBMM_CFLAGS) $(LIBXMLPP_CFLAGS) $(CAIRO_CFLAGS) $(BOOST_CPPFLAGS) $(TVMET_CFLAGS) $(CXXFLAGS) -I$(top_srcdir)
qaatsi_grid_LDFLAGS  = $(LDFLAGS)
qaatsi_grid_LDADD    = $(top_builddir)/libroad/libroad.la

if DO_IMAGE
noinst_PROGRAMS += mesh-extract-test displace-polylines read-scene

mesh_extract_test_SOURCES = mesh-extract-test.cpp
mesh_extract_test_CPPFLAGS = $(GLIBMM_CFLAGS) $(LIBXMLPP_CFLAGS) $(CAIRO_CFLAGS) $(BOOST_CPPFLAGS) $(TVMET_CFLAGS) $(CAIRO_CFLAGS) $(CXXFLAGS) -I$(top_srcdir)
mesh_extract_test_LDFLAGS  = $(CAIRO_LIBS) $(LDFLAGS)
mesh_extract_test_LDADD    = $(top_builddir)/libroad/libroad.la $(top_builddir)/libroad/libroad_image.la

displace_polylines_SOURCES = displace-polylines.cpp
displace_polylines_CPPFLAGS = $(GLIBMM_CFLAGS) $(LIBXMLPP_CFLAGS) $(CAIRO_CFLAGS) $(BOOST_CPPFLAGS) $(TVMET_CFLAGS) $(MAGICKPP_CXXFLAGS) $(CXXFLAGS) -I$(top_srcdir)
displace_polylines_LDFLAGS  = $(MAGICKPP_LDFLAGS) $(LDFLAGS)
displace_polylines_LDADD    = $(top_builddir)/libroad/libroad.la $(top_builddir)/libroad/libroad_image.la

read_scene_SOURCES  = read-scene.cpp
read_scene_CPPFLAGS = $(GLIBMM_CFLAGS) $(LIBXMLPP_CFLAGS) $(CAIRO_CFLAGS) $(BOOST_CPPFLAGS) $(TVMET_CFLAGS) $(MAGICKPP_CXXFLAGS) $(CXXFLAGS) -I$(top_srcdir)
read_scene_LDFLAGS  = $(MAGICKPP_LDFLAGS) $(LDFLAGS)
read_scene_LDADD    = $(top_builddir)/libroad/libroad.la
endif

if DO_VISUAL
if DO_IMAGE
noinst_PROGRAMS += arc-road-test view-network micro view-osm cairo-network

arc_road_test_SOURCES = arc-road-test.cpp
arc_road_test_CPPFLAGS = $(GLIBMM_CFLAGS) $(LIBXMLPP_CFLAGS) $(CAIRO_CFLAGS) $(BOOST_CPPFLAGS) $(TVMET_CFLAGS) $(FLTK_CXXFLAGS) $(CXXFLAGS) -I$(top_srcdir)
arc_road_test_LDFLAGS  = $(LDFLAGS) $(FLTK_LDFLAGS) $(FLTK_GL_LDFLAGS) $(GLEW_LIBS)
arc_road_test_LDADD    = $(top_builddir)/libroad/libroad_visual.la $(top_builddir)/libroad/libroad.la $(top_builddir)/libroad/libroad_image.la

view_network_SOURCES = view-network.cpp
view_network_CPPFLAGS = $(GLIBMM_CFLAGS) $(LIBXMLPP_CFLAGS) $(CAIRO_CFLAGS) $(BOOST_CPPFLAGS) $(TVMET_CFLAGS) $(FLTK_CXXFLAGS) $(CXXFLAGS) -I$(top_srcdir)
view_network_LDFLAGS  = $(LDFLAGS) $(FLTK_LDFLAGS) $(FLTK_GL_LDFLAGS) $(GLEW_LIBS)
view_network_LDADD    = $(top_builddir)/libroad/libroad_visual.la $(top_builddir)/libroad/libroad.la $(top_builddir)/libroad/libroad_image.la

micro_SOURCES  = micro.cpp
micro_CPPFLAGS = $(GLIBMM_CFLAGS) $(LIBXMLPP_CFLAGS) $(CAIRO_CFLAGS) $(BOOST_CPPFLAGS) $(TVMET_CFLAGS) $(FLTK_CXXFLAGS) $(CXXFLAGS) -I$(top_srcdir)
micro_LDFLAGS  = $(LDFLAGS) $(FLTK_LDFLAGS) $(FLTK_GL_LDFLAGS) $(GLEW_LIBS)
micro_LDADD    = $(top_builddir)/libroad/libroad_visual.la $(top_builddir)/libroad/libroad.la $(top_builddir)/libroad/libroad_image.la

view_osm_SOURCES  = view-osm.cpp
view_osm_CPPFLAGS = $(GLIBMM_CFLAGS) $(LIBXMLPP_CFLAGS) $(CAIRO_CFLAGS) $(BOOST_CPPFLAGS) $(TVMET_CFLAGS) $(MAGICKPP_CXXFLAGS) $(CAIRO_CFLAGS) $(CXXFLAGS) -I$(top_srcdir)
view_osm_LDFLAGS  = $(CAIRO_LIBS) $(MAGICKPP_LDFLAGS) $(FLTK_LDFLAGS) $(FLTK_GL_LDFLAGS) $(GLEW_LIBS) $(LDFLAGS)
view_osm_LDADD    = $(top_builddir)/libroad/libroad.la $(top_builddir)/libroad/libroad_visual.la $(top_builddir)/libroad/libroad_image.la

cairo_network_SOURCES  = cairo-network.cpp
cairo_network_CPPFLAGS = $(GLIBMM_CFLAGS) $(LIBXMLPP_CFLAGS) $(CAIRO_CFLAGS) $(BOOST_CPPFLAGS) $(TVMET_CFLAGS) $(MAGICKPP_CXXFLAGS) $(CAIRO_CFLAGS) $(CXXFLAGS) -I$(top_srcdir)
cairo_network_LDFLAGS  = $(CAIRO_LIBS) $(MAGICKPP_LDFLAGS) $(FLTK_LDFLAGS) $(FLTK_GL_LDFLAGS) $(GLEW_LIBS) $(LDFLAGS)
cairo_network_LDADD    = $(top_builddir)/libroad/libroad.la $(top_builddir)/libroad/libroad_visual.la $(top_builddir)/libroad/libroad_image.la
endif
endif
