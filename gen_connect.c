/*
 * gen_connect.c: 
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
#include "gen_connect.h"
extern object_t *DEV;
static void gen_connect_set(object_t *obj, FILE *f) {
	connect_t *con = (connect_t *)obj;
	const char *name = obj->name;
	const char *dev_name = DEV->name;
	struct list_head *p;
	object_t *tmp;
	const char *iface = NULL;
	int is_array = obj->is_array;

	fprintf(f, "\nstatic int %s_set(conf_object_t *obj, conf_object_t *peer, const char *port, int _idx) {\n", name);
	fprintf(f, "\t%s_t *_dev = (%s_t *)obj;\n", dev_name, dev_name);
	fprintf(f, "\tint ret = 0;\n");
	fprintf(f, "\tvoid const *iface = NULL;\n");
	fprintf(f, "\n");
	if(!is_array) {
		fprintf(f, "\tif(_dev->%s.obj == peer && _dev->%s.port == port) {\n", name, name);
	} else {
		fprintf(f, "\tif(_dev->%s[_idx].obj == peer && _dev->%s[_idx].port == port) {\n", name, name);
	}
	fprintf(f, "\t\treturn 0;\n");
	fprintf(f, "\t}\n");
	if(!is_array) {
		fprintf(f, "\t_dev->%s.obj = peer;\n", name);
	} else {
		fprintf(f, "\t_dev->%s[_idx].obj = peer;\n", name);
	}
	if(!is_array) {
		fprintf(f, "\tif(_dev->%s.port)\n", name);
	} else {
		fprintf(f, "\tif(_dev->%s[_idx].port)\n", name);
	}
	if(!is_array) {
		fprintf(f, "\t\tMM_FREE((void *)_dev->%s.port);\n", name);
	} else {
		fprintf(f, "\t\tMM_FREE((void *)_dev->%s[_idx].port);\n", name);
	}
	fprintf(f, "\tif(port) {\n");
	if(!is_array) {
		fprintf(f, "\t\t_dev->%s.port = MM_STRDUP(port);\n", name);
	} else {
		fprintf(f, "\t\t_dev->%s[_idx].port = MM_STRDUP(port);\n", name);
	}
	fprintf(f, "\t} else { \n");
	if(!is_array) {
		fprintf(f, "\t\t_dev->%s.port = NULL;\n", name);
	} else {
		fprintf(f, "\t\t_dev->%s[_idx].port = NULL;\n", name);
	}
	fprintf(f, "\t}\n");
	list_for_each(p, &obj->childs) {
		tmp = list_entry(p, object_t, entry);
		iface = tmp->name;
		fprintf(f, "\tif(port) {\n");
		fprintf(f, "\t\tiface = SIM_c_get_port_interface(peer, \"%s\", port);\n",iface);
		fprintf(f, "\t} else {\n");
		fprintf(f, "\t\tiface = SIM_c_get_interface(peer, \"%s\");\n", iface);
		fprintf(f, "\t}\n");
		if(!is_array) {
			fprintf(f, "\t_dev->%s.%s = iface;\n", name, iface);
		} else {
			fprintf(f, "\t_dev->%s[_idx].%s = iface;\n", name, iface);
		}
	}
	fprintf(f, "\treturn ret;\n");
	fprintf(f, "}\n");
}

static void gen_connect_get(object_t *obj, FILE *f) {
	const char *name = obj->name;
	/*dummy get function*/	
	fprintf(f, "\nstatic int %s_get(conf_object_t *obj, conf_object_t **peer, char **port, int index) {\n", name);
	fprintf(f, "\tint ret = 0;\n");
	fprintf(f, "\tUNUSED(ret);\n");
	fprintf(f, "\treturn ret;\n");
	fprintf(f, "}\n");
}

static void gen_connect_code(object_t *obj, FILE *f) {
	gen_connect_set(obj, f);
	gen_connect_get(obj, f);
}

void gen_device_connect_code(device_t *dev, FILE *f) {
	struct list_head *p;
	object_t *obj;

	list_for_each(p, &dev->connects) {
		obj = list_entry(p, object_t, entry);
		gen_connect_code(obj, f);
	}
}
