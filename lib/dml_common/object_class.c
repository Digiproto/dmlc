/*
 * object_class.c: 
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
#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>
#include "simics/util/list.h"
#include "simics/core/object_class.h"
#include "simics/host-config.h"

#define DEFAULT_SO_DIR "$PWD/$OS/lib/"

#ifndef _win 
const char *suffix = "so";
#else 
const char *suffix = "dll";
#endif
const char *entry = "init_local";
static const char *def_so_dir = SYSTEM_LIB;
//static const char *def_so_dir = "/opt/skyeye/lib";

static LIST_HEAD(class_list);
typedef struct class_list {
	struct list_head entry;
	const char *cls_name;
	const conf_class_t *cls; 
} cls_list_t;
#define BUF_SIZE 1024
static char buf[BUF_SIZE];
int load_class(const char *dir, const char *so) {
	void *handle;
	void (*pf)(void);
	char *error;
	char *p = buf;

	snprintf(p, sizeof(buf), "%s/lib%s.%s", dir, so, suffix);
	handle = dlopen(buf, RTLD_LAZY);	
	if(!handle) {
		printf("%s\n", dlerror());
		exit(-1);
	}
	dlerror();
	pf = (void (*)(void))dlsym(handle, entry);
	if((error = dlerror()) != NULL) {
		printf("%s\n", error);
		exit(-1);
	}
	if(pf) {
		pf();
	}
	// text segment would destroy for mem if handle closed
	//dlclose(handle);
	return 0;
}

const conf_class_t *SIM_class_find(const char *cls) {
	struct list_head *p;
	cls_list_t *tmp;
	int i = 0;	
try_again:
	list_for_each(p, &class_list){
		tmp = list_entry(p,cls_list_t, entry); 
		if(!strcmp(tmp->cls->cls_name, cls)) {
			return tmp->cls; 
		}
	}
	if(i == 0) {
		load_class(def_so_dir, cls);
		i++;
		goto try_again;
	}
	return NULL;	
}

void SIM_register_conf_class(const char *name, const conf_class_t *cls) {
	cls_list_t *tmp;

	tmp = (cls_list_t *)malloc(sizeof(*tmp));
	if(tmp) {
		INIT_LIST_HEAD(&tmp->entry);
		tmp->cls = cls;
		tmp->cls_name = strdup(name);
		list_add_tail(&tmp->entry, &class_list);
	} else {
		printf("memory not enough\n");
	}
}

