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
	PARAM_TYPE_NONE,
	PARAM_TYPE_INT,
	PARAM_TYPE_FLOAT,
	PARAM_TYPE_BOOL,
	PARAM_TYPE_STRING,
	PARAM_TYPE_LIST,
	PARAM_TYPE_LIST2,
	PARAM_TYPE_REF,
	PARAM_TYPE_UNDEF,
	PARAM_TYPE_MAX
} param_type_t;

typedef enum parameter_flag {
	PARAM_FLAG_NONE,
	PARAM_FLAG_DEFAULT,
	PARAM_FLAG_AUTO,
	PARAM_FLAG_MAX
} param_flag_t;

typedef struct param_value {
	int is_original;
	param_type_t type;
	param_flag_t flag;
	int is_const;
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
