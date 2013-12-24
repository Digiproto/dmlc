/*
 * base_types.h: 
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

#ifndef __BASE_TYPES_H__ 
#define __BASE_TYPES_H__  
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
/*
typedef unsigned long long uint64_t; 
typedef unsigned char uint8_t; 
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;
*/
typedef int64_t integer_t;
typedef uint64_t uinteger_t;
typedef int8_t int8;
typedef uint8_t uint8;
typedef int16_t int16;
typedef uint16_t uint16;
typedef int32_t int32;
typedef uint32_t uint32;
typedef int64_t int64;
typedef uint64_t uint64;
#ifndef __MINGW32__
typedef uint64 uintptr_t;
#endif
typedef int set_error_t;
typedef uint32 physical_address_t;
typedef physical_address_t logical_address_t;
typedef struct conf_object  conf_object_t;
#define MM_STRDUP strdup
#define MM_FREE free
#define MEMSET memset
#define MEMCPY memcpy
#define MM_ZALLOC(n) memset(malloc(n), 0, n)
#define SIM_c_get_port_interface SIM_get_port_interface
#define SIM_c_get_interface SIM_get_interface
#define conf_object_register SIM_object_register
#define UNUSED(x) (void)(x)
//#define UNUSED __attribute__((unused)) 
#define MASK(x) ((x) < 32 \
                 ? ((1 << (x)) - 1) \
                 : (x) < 64 \
                 ? ((UINT64_C(1) << (x)) - 1) \
                 : UINT64_C(0xffffffffffffffff))
#define MASK1(x) MASK((x) + 1)
#define MIX(x, y, mask) (((x) & ~(mask)) | ((y) & (mask)))

typedef enum{
	/* No exception */
	No_exp = 0,
	/* Memory allocation exception */
	Malloc_exp,
	/* File open exception */
	File_open_exp,
	/* DLL open exception */
	Dll_open_exp,
	/* Invalid argument exception */
	Invarg_exp,
	/* Invalid module exception */
	Invmod_exp,
	/* wrong format exception for config file parsing */
	Conf_format_exp,
	/* some reference excess the predefiend range. Such as the index out of array range */
	Excess_range_exp,
	/* Can not find the desirable result */
	Not_found_exp,
	/* Wrong executable file format */
	Exec_format_exp,
	/* Access exception */
	Access_exp,
	//Sim_PE_No_Exception = No_exp,
	/* Unknown exception */
	Unknown_exp
}exception_type_t;

#endif /* __BASE_TYPES_H__ */
