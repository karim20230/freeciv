## Process this file with automake to produce Makefile.in

SUBDIRS = enums

EXTRA_DIST = \
	generate_enums.py \
	generate_version_header.sh \
	liblua_config.h.in \
	freeciv_config.h.in \
	meson_fc_config.h.in \
	meson_freeciv_config.h.in \
	meson_liblua_config.h.in

BUILT_SOURCES = version_gen.h

version_gen.h: verh_generate
.INTERMEDIATE: verh_generate
verh_generate: generate_version_header.sh $(top_srcdir)/fc_version
	$(srcdir)/generate_version_header.sh version_gen.h
	touch verh_generate

CLEANFILES = version_gen.h
