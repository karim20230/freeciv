/*****************************************************************************
 Freeciv - Copyright (C) 2005 - The Freeciv Project
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
*****************************************************************************/

#ifndef FC__API_COMMON_UTILITIES_H
#define FC__API_COMMON_UTILITIES_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* common/scriptcore */
#include "luascript_types.h"

struct lua_State;

int api_utilities_random(lua_State *L, int min, int max);

const Direction *api_utilities_str2dir(lua_State *L, const char *dir);
const Direction *api_utilities_dir_ccw(lua_State *L, Direction dir);
const Direction *api_utilities_dir_cw(lua_State *L, Direction dir);
const Direction *api_utilities_opposite_dir(lua_State *L, Direction dir);
bool api_utilities_direction_is_cardinal(lua_State *L, Direction dir);
int api_utilities_direction_id(lua_State *L, Direction dir);
const char *api_utilities_dir2str(lua_State *L, Direction dir);

const char *api_utilities_fc_version(lua_State *L);
const char *api_utilities_name_version(lua_State *L);
const char *api_utilities_comparable_version(lua_State *L);
const char *api_utilities_version_string(lua_State *L);

int api_utilities_versions_compare(lua_State *L, const char *ver1, const char *ver2);

void api_utilities_log_base(lua_State *L, int level, const char *message);

void api_utilities_deprecation_warning(lua_State *L, char *method,
                                       char *replacement,
                                       char *deprecated_since);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* FC__API_COMMON_UTILITIES_H */
