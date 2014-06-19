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
#include "qemu-common.h"
#include <stdint.h>

typedef int64_t integer_t;
typedef uint64_t uinteger_t;
//typedef int8_t int8;
//typedef uint8_t uint8;
typedef int16_t int16;
typedef uint16_t uint16;
typedef uint8 uint1;
typedef uint8 uint2;
typedef uint8 uint3;
typedef uint8 uint7;
typedef uint8 uint6;
typedef uint8 uint5;
typedef uint8 uint4;
typedef uint16 uint13;
typedef uint64 uint31;

//typedef int32_t int32;
//typedef uint32_t uint32;
//typedef int64_t int64;
//typedef uint64_t uint64;
//typedef uint64 uintptr_t;
typedef int set_error_t;
typedef int exception_type_t;
typedef uint32 physical_address_t;
typedef physical_address_t logical_address_t;
typedef uint64 generic_address_t;
typedef DeviceState conf_object_t;
typedef struct { const uint8 *data; size_t len; } bytes_t;
typedef struct { int integer; char *string; } tuple_int_string_t;
typedef struct { int valid; physical_address_t address; physical_address_t block_start; physical_address_t block_end; } physical_block_t;
typedef struct { size_t len; uint8 *str; } byte_string_t;
typedef struct { uint8 *data; size_t len; } buffer_t;
typedef int access_t;
typedef int cpu_endian_t;
typedef int ireg_info_t;
typedef int phy_speed_t;
typedef integer_t nano_secs_t;
typedef int pcie_message_type_t;
typedef int addr_space_t;
typedef int swap_mode_t;
typedef int sim_exception_t;
typedef uint64 cycles_t;
typedef uint64 pc_step_t;
#define MM_STRDUP strdup
#define MM_FREE free
#define SIM_c_get_port_interface qdev_get_port_interface
#define SIM_c_get_interface qdev_get_interface
#define UNUSED(x) (void)(x)
#define MASK(x) ((x) < 32 \
                 ? ((1 << (x)) - 1) \
                 : (x) < 64 \
                 ? ((UINT64_C(1) << (x)) - 1) \
                 : UINT64_C(0xffffffffffffffff))
#define MASK1(x) MASK((x) + 1)
#define MIX(x, y, mask) (((x) & ~(mask)) | ((y) & (mask)))

#endif /* __BASE_TYPES_H__ */
