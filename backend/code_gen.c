
/* Copyright (C) 
*  2013  alloc.young@gmail.com
* This program is free software; you can redistribute it and/or
* * modify it under the terms of the GNU General Public License
* * as published by the Free Software Foundation; either version 2
* * of the License, or (at your option) any later version.
* * 
* * This program is distributed in the hope that it will be useful,
* * but WITHOUT ANY WARRANTY; without even the implied warranty of
* * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* * GNU General Public License for more details.
* * 
* * You should have received a copy of the GNU General Public License
* * along with this program; if not, write to the Free Software
* * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
* * 
* */

/**
 * * @file code_gen.c
 * * @brief The c source code generation
 * * @author alloc alloc.young@gmail.com
 * * @version 7849
 * * @date 2013-3-12
 * */


#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "object.h"
#include "object_headfile.h"
#include "platform_code_gen.h"

extern symtab_t root_table;

void gen_code (tree_t * root, const char *name)
{
	symbol_list_t *list;
	symbol_t sym;
	device_attr_t *dev;
	device_t *dev_obj;

	printf("welcome to code generation backend\n");
	list = symbol_list_find(root_table, DEVICE_TYPE); 
	if(!list || list->next) {
		printf("device not right\n");
		exit(-1);
	}
	sym = list->sym;	
	symbol_list_free(list);
	dev = (device_attr_t *)sym->attr;
	dev_obj = create_device_tree(root);
	print_device_tree(dev_obj);
	device_realize(dev_obj);
	print_device_tree(dev_obj);
	gen_headerfiles(dev_obj, name);
	gen_cfile(dev_obj, name);
}