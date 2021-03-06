/*
 * platform.h: 
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

#ifndef __PLATFORM_H__ 
#define __PLATFORM_H__  
#include "object.h"
void pre_gen_code(device_t *dev, FILE *f);
void post_gen_code(device_t *dev, FILE *f);
void gen_object_struct(device_t *dev, FILE *f);
void gen_device_init(device_t *dev, FILE *f);
void gen_device_type_info(device_t * dev, FILE *f);
#if backend == 3
void gen_platform_device_info(device_t * dev, const char *out);
#endif
void gen_platform_device_module(device_t *dev, const char *out);
#endif /* __PLATFORM_H__ */
