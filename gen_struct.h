/*
 * gen_struct.h: 
 *
 * Copyright (C) 2013 alloc <alloc.young@gmail.com>
 * Skyeye Develop Group, for help please send mail to
 * <skyeye-developer@lists.gro.clinux.org>
 *
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#ifndef __GEN_STRUCT_H__ 
#define __GEN_STRUCT_H__  
#include "gen_common.h"

void gen_device_macros(device_t *dev, FILE *f);
void gen_device_struct(device_t *dev, FILE *f);
void gen_device_loggroup(device_t *dev, FILE *f);
void gen_device_toplevel_struct(device_t *dev, FILE *f);
void gen_device_typedef(device_t *t, FILE *f);
#endif /* __GEN_STRUCT_H__ */
