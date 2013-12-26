/*
 * gen_event.c: 
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
#include "gen_event.h"
extern object_t *DEV;

extern int method_to_generate;
void gen_device_event_class(device_t *dev, FILE *f) {
}

static void gen_event_code(object_t *obj, FILE *f) {
	const char *evt = obj->name;
	const char *dev = DEV->name;

	fprintf(f, "\nstatic void\n");
	fprintf(f, "_DML_EV_%s__callback(conf_object_t *obj, void *param)\n", evt);
	fprintf(f, "{\n");
	fprintf(f, "\t%s_t *_dev = (%s_t *)obj;\n", dev, dev);
	fprintf(f, "\t{\n");
	fprintf(f, "\t\tbool exec = 0;\n");
	fprintf(f, "\t\t\t{\n");
	fprintf(f, "\t\t\t\texec = _DML_M_%s__callback(_dev, param);\n", evt);
	fprintf(f, "\t\t\t\tif(exec) {\n");
	fprintf(f, "\t\t\t\t\tgoto exit;\n");
	fprintf(f, "\t\t\t\t}\n");
	fprintf(f, "\t\t\texit:\n");
	fprintf(f, "\t\t\t\treturn;\n");
	fprintf(f, "\t\t\t}\n");
	fprintf(f, "\t}\n");
	fprintf(f, "}\n");
	add_object_method(obj, "callback");
}

void gen_device_event_code(device_t *dev, FILE *f) {
	struct list_head *p;
	object_t *tmp;

	list_for_each(p, &dev->obj.events) {
		tmp = list_entry(p, object_t, entry);
		gen_event_code(tmp, f);
	}
}

void add_event_method(object_t *obj) {
	add_object_method(obj, "callback");
}
