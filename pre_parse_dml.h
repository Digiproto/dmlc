/*
 * pre_parse_dml.h:
 *
 * Copyright (C) 2013 Oubang Shen <shenoubang@gmail.com>
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

#ifndef __PRE_PARSE_DML_H__
#define __PRE_PARSE_DML_H__

typedef struct {
	char* name;
	int type;
} pre_dml_t;

typedef struct {
	const char* name;
	int type;		// int, string, etc
	int value_type; // auto, undefined
					// if it is assign, the value_type = -1;
} standard_param_t;

typedef struct {
	const char* name;
	char** value;
}special_param_t;

typedef enum {
	AUTO_VALUE = 1,
	DEFAULT_VALUE,
	ASSIGNED_VALUE,
	UNDEFINED_VALUE
}value_type_t;

int insert_pre_dml_struct(void);
int find_all_pre_dml_struct(void);
standard_param_t* get_standard_param(int type);

#endif /* __PRE_PARSE_DML_H__ */
