/*
 * parameter_type.h: 
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

#ifndef __PARAMETER_TYPE_H__ 
#define __PARAMETER_TYPE_H__  
#include <stdio.h>
typedef enum parameter_type {
	param_type_none,
	param_type_int,
	param_type_float,
	param_type_bool,
	param_type_string,
	param_type_list,
	param_type_ref,
	param_type_undef,
	param_type_max
} param_type_t;


typedef struct param_value {
	param_type_t type;
	union {
		int integer;
		const char *string;
		float floating;
		int boolean;
		struct {
			int size;
			struct param_value *vector;
		} list;
		void *ref; 
	} u;
} param_value_t;


#endif /* __PARAMETER_TYPE_H__ */
