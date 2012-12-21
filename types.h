/* Copyright (C) 
* 2012 - Michael.Kang blackfin.kang@gmail.com
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* as published by the Free Software Foundation; either version 2
* of the License, or (at your option) any later version.
* 
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
* 
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
* 
*/
/**
* @file types.h
* @brief The type of variable
* @author Michael.Kang blackfin.kang@gmail.com
* @version 7849
* @date 2012-12-04
*/
#ifndef __TYPES_H__
#define __TYPES_H__
typedef enum{
	DEVICE_TYPE = 1,
	PARAMETER_TYPE,
	METHOD_TYPE,
	BANK_TYPE,
	REGISTER_TYPE,
	IMPORT_TYPE,
	TEMPLATE_TYPE,
	IS_TYPE,
	BITORDER_TYPE,
	EXTERN_TYPE
}ident_type_t;

typedef struct obj_variable{
	int type;
	char* name;
}objtype;

#endif
