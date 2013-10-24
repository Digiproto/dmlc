/*
 * gen_debug.h: 
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

#ifndef __GEN_DEBUG_H__ 
#define __GEN_DEBUG_H__  
#include <stdio.h>

#define DEBUG_BACKEND
#ifdef DEBUG_BACKEND
enum {
	DEBUG_BACKEND_GENERAL,
	DEBUG_BACKEND_OBJ_SYM,
	DEBUG_BACKEND_OBJ,
	DEBUG_BACKEND_TEMPLATE,
	DEBUG_BACKEND_DML,
	DEBUG_BACKEND_NOTYPE
};

#define DBGBIT(x) (1 <<(DEBUG_BACKEND_##x))
extern int debugflags;
#define BE_DBG(what, fmt, ...) do { \
	if(debugflags & DBGBIT(what)) \
		fprintf(stderr, "backend: " fmt, ## __VA_ARGS__); \
	} while(0) 
#else 
#define BE_DBG(what, fmt, ...) do {} while(0)
#endif

#define my_DBG(fmt, ...) \
	BE_DBG(DML, fmt, ##__VA_ARGS__)
#endif /* __GEN_DEBUG_H__ */
