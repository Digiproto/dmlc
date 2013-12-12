/*
 * bank_access.h: 
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

#ifndef __BANK_ACCESS_H__
#define __BANK_ACCESS_H__  
#include <simics/base_types.h>

typedef struct bank_access {
	exception_type_t (*read)(conf_object_t *obj, physical_address_t addr, void *buf, size_t size);
	exception_type_t (*write)(conf_object_t *obj, physical_address_t addr, const void *buf, size_t size);
} bank_access_t;

struct bank_access_description {
	const char *name;
	const bank_access_t *access;
};

const bank_access_t *SIM_find_bank_access(conf_object_t *dev, const char *bank_name);
#endif /* __BANK_ACCESS_H__ */
