/***********************************************************************
 Freeciv - Copyright (C) 2004 - Marcelo J. Burda
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
***********************************************************************/
#ifdef HAVE_CONFIG_H
#include <fc_config.h>
#endif

#include <math.h> /* sqrt */

/* utility */
#include "log.h"

/* common */
#include "game.h"
#include "map.h"

#include "mapgen_topology.h"

int ice_base_colatitude = 0 ;

/************************************************************************//**
  Returns the colatitude of this map position.  This is a value in the
  range of 0 to MAX_COLATITUDE (inclusive).
  This function is wanted to concentrate the topology information
  all generator code has to use  colatitude and others topology safe
  functions instead (x,y) coordinate to place terrains
  colatitude is 0 at poles and MAX_COLATITUDE at equator
****************************************************************************/
int map_colatitude(const struct tile *ptile)
{
  int latitude;
  fc_assert_ret_val(ptile != NULL, MAX_COLATITUDE / 2);

  latitude = map_signed_latitude(ptile);
  latitude = MAX(latitude, -latitude);

  return colat_from_abs_lat(latitude);
}

/************************************************************************//**
  Return TRUE if the map in a typical city radius is SINGULAR.  This is
  used to avoid putting (non-polar) land near the edge of the map.
****************************************************************************/
bool near_singularity(const struct tile *ptile)
{
  return is_singular_tile(ptile, CITY_MAP_DEFAULT_RADIUS);
}


/************************************************************************//**
  Set the map xsize and ysize based on a base size and ratio (in natural
  coordinates).
****************************************************************************/
static void set_sizes(double size, int Xratio, int Yratio)
{
  /* Some code in generator assumes even dimension, so this is set to 2.
   * Future topologies may also require even dimensions. */
  /* The generator works also for odd values; keep this here for autogenerated
   * height and width. */
  const int even = 2;

  /* In iso-maps we need to double the map.ysize factor, since xsize is
   * in native coordinates which are compressed 2x in the X direction. */
  const int iso = MAP_IS_ISOMETRIC ? 2 : 1;

  /* We have:
   *
   *   1000 * size = xsize * ysize
   *
   * And to satisfy the ratios and other constraints we set
   *
   *   xsize = i_size * xratio * even
   *   ysize = i_size * yratio * even * iso
   *
   * For any value of "i_size".  So with some substitution
   *
   *   1000 * size = i_size * i_size * xratio * yratio * even * even * iso
   *   i_size = sqrt(1000 * size / (xratio * yratio * even * even * iso))
   *
   * Make sure to round off i_size to preserve exact wanted ratios,
   * that may be importante for some topologies.
   */
  const int i_size
    = sqrt((float)(size)
	   / (float)(Xratio * Yratio * iso * even * even)) + 0.49;

  /* Now build xsize and ysize value as described above. */
  wld.map.xsize = Xratio * i_size * even;
  wld.map.ysize = Yratio * i_size * even * iso;

  /* Now make sure the size isn't too large for this ratio or the overall map
   * size (MAP_INDEX_SIZE) is larger than the maximal allowed size
   * (MAP_MAX_SIZE * 1000). If it is then decrease the size and try again. */
  if (wld.map.xsize > MAP_MAX_LINEAR_SIZE
      || wld.map.ysize > MAP_MAX_LINEAR_SIZE
      || MAP_INDEX_SIZE > MAP_MAX_SIZE * 1000) {
    fc_assert(size > 100.0);
    set_sizes(size - 100.0, Xratio, Yratio);
    return;
  }

  /* If the ratio is too big for some topology the simplest way to avoid
   * this error is to set the maximum size smaller for all topologies! */
  if (wld.map.server.size * 1000 > size + 900.0) {
    /* Warning when size is set uselessly big */
    log_error("Requested size of %d is too big for this topology.",
              wld.map.server.size);
  }

  /* xsize and ysize must be between MAP_MIN_LINEAR_SIZE and
   * MAP_MAX_LINEAR_SIZE. */
  wld.map.xsize = CLIP(MAP_MIN_LINEAR_SIZE, wld.map.xsize, MAP_MAX_LINEAR_SIZE);
  wld.map.ysize = CLIP(MAP_MIN_LINEAR_SIZE, wld.map.ysize, MAP_MAX_LINEAR_SIZE);

  log_normal(_("Creating a map of size %d x %d = %d tiles (%d requested)."),
             wld.map.xsize, wld.map.ysize, wld.map.xsize * wld.map.ysize,
             (int) size);
}

/************************************************************************//**
  Return the default ratios for known topologies.

 The factor x_ratio * y_ratio determines the accuracy of the size.
 Small ratios work better than large ones; 3:2 is not the same as 6:4
****************************************************************************/
static void get_ratios(int *x_ratio, int *y_ratio)
{
  if (current_wrap_has_flag(WRAP_X)) {
    if (current_wrap_has_flag(WRAP_Y)) {
      /* Ratios for torus map. */
      *x_ratio = 1;
      *y_ratio = 1;
    } else {
      /* Ratios for classic map. */
      *x_ratio = 3;
      *y_ratio = 2;
    }
  } else {
    if (current_wrap_has_flag(WRAP_Y)) {
      /* Ratios for uranus map. */
      *x_ratio = 2;
      *y_ratio = 3;
    } else {
      /* Ratios for flat map. */
      *x_ratio = 1;
      *y_ratio = 1;
    }
  }
}

/************************************************************************//**
  This function sets sizes in a topology-specific way then calls
  map_init_topology(). Set 'autosize' to TRUE if the xsize/ysize should be
  calculated.
****************************************************************************/
void generator_init_topology(bool autosize)
{
  int sqsize;
  double map_size;

  /* The server behavior to create the map is defined by 'map.server.mapsize'.
   * Calculate the xsize/ysize if it is not directly defined. */
  if (autosize) {
    int x_ratio, y_ratio;

    switch (wld.map.server.mapsize) {
    case MAPSIZE_XYSIZE:
      wld.map.server.size = (float)(wld.map.xsize * wld.map.ysize) / 1000.0 + 0.5;
      wld.map.server.tilesperplayer = ((map_num_tiles() * wld.map.server.landpercent)
                                   / (player_count() * 100));
      log_normal(_("Creating a map of size %d x %d = %d tiles (map size: "
                   "%d)."), wld.map.xsize, wld.map.ysize, wld.map.xsize * wld.map.ysize,
                 wld.map.server.size);
      break;

    case MAPSIZE_PLAYER:
      map_size = ((double) (player_count() * wld.map.server.tilesperplayer * 100)
                  / wld.map.server.landpercent);

      if (map_size < MAP_MIN_SIZE * 1000) {
        wld.map.server.size = MAP_MIN_SIZE;
        map_size = MAP_MIN_SIZE * 1000;
        log_normal(_("Map size calculated for %d (land) tiles per player "
                     "and %d player(s) too small. Setting map size to the "
                     "minimal size %d."), wld.map.server.tilesperplayer,
                   player_count(), wld.map.server.size);
      } else if (map_size > MAP_MAX_SIZE * 1000) {
        wld.map.server.size = MAP_MAX_SIZE;
        map_size = MAP_MAX_SIZE * 1000;
        log_normal(_("Map size calculated for %d (land) tiles per player "
                     "and %d player(s) too large. Setting map size to the "
                     "maximal size %d."), wld.map.server.tilesperplayer,
                   player_count(), wld.map.server.size);
      } else {
        wld.map.server.size = (double) map_size / 1000.0 + 0.5;
        log_normal(_("Setting map size to %d (approx. %d (land) tiles for "
                     "each of the %d player(s))."), wld.map.server.size,
                   wld.map.server.tilesperplayer, player_count());
      }
      get_ratios(&x_ratio, &y_ratio);
      set_sizes(map_size, x_ratio, y_ratio);
      break;

    case MAPSIZE_FULLSIZE:
      /* Set map.xsize and map.ysize based on map.size. */
      get_ratios(&x_ratio, &y_ratio);
      set_sizes(wld.map.server.size * 1000, x_ratio, y_ratio);
      wld.map.server.tilesperplayer = ((map_num_tiles() * wld.map.server.landpercent)
                                   / (player_count() * 100));
      break;
    }
  } else {
    wld.map.server.size = (double) map_num_tiles() / 1000.0 + 0.5;
    wld.map.server.tilesperplayer = ((map_num_tiles() * wld.map.server.landpercent)
                                 / (player_count() * 100));
  }

  sqsize = get_sqsize();

  /* initialize the ICE_BASE_LEVEL */

  /* if maps has strip like poles we get smaller poles
   * (less playables than island poles)
   *   5% for little maps
   *   2% for big ones, if map.server.temperature == 50
   * except if separate poles is set */
  if (wld.map.server.separatepoles) {
    /* with separatepoles option strip poles are useless */
    ice_base_colatitude =
        (MAX(0, 100 * COLD_LEVEL / 3 - 1 *  MAX_COLATITUDE)
         + 1 *  MAX_COLATITUDE * sqsize) / (100 * sqsize);
  } else {
    /* any way strip poles are not so playable as isle poles */
    ice_base_colatitude =
        (MAX(0, 100 * COLD_LEVEL / 3 - 2 * MAX_COLATITUDE)
         + 2 * MAX_COLATITUDE * sqsize) / (100 * sqsize);
  }

  /* Correction for single-pole and similar map types
   * The pole should be the same size relative to world size,
   * so the smaller the actual latitude range,
   * the smaller the ice base level has to be.
   * However: Don't scale down by more than a half. */
  if (MAP_REAL_LATITUDE_RANGE(wld.map) < (2 * MAP_MAX_LATITUDE)) {
    ice_base_colatitude = (ice_base_colatitude
                           * MAX(MAP_REAL_LATITUDE_RANGE(wld.map),
                                 MAP_MAX_LATITUDE)
                           / (2 * MAP_MAX_LATITUDE));
  }

  if (wld.map.server.huts_absolute >= 0) {
    wld.map.server.huts = wld.map.server.huts_absolute * 1000 / map_num_tiles();
    wld.map.server.huts_absolute = -1;
  }

  map_init_topology();
}

/************************************************************************//**
  An estimate of the linear (1-dimensional) size of the map.
****************************************************************************/
int get_sqsize(void)
{
  int sqsize = sqrt(MAP_INDEX_SIZE / 1000);

  return MAX(1, sqsize);
}
