/*
 * gen_implement.c: 
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
#include "gen_implement.h"
extern object_t *DEV;
static void print_params_protoc(tree_t *params, FILE *f) {
	const char *type;
	const char *name;
	tree_t *node;

	if(params) {
		if(params->params.in_params) {
			fprintf(f, ", ");
			node = params->params.in_params;
			while(node) {
				type = get_type_info(node);
				name = get_cdecl_name(node);
				fprintf(f, "%s %s", type, name);
				node = node->common.sibling;
				if(node) {
					fprintf(f, ", ");
				}
			}
		}
	}
}

static void print_params_in(tree_t *params, FILE *f) {
	const char *name;
	tree_t *node;

	if(params) {
		if(params->params.in_params) {
			fprintf(f, ", ");
			node = params->params.in_params;
			while(node) {
				name = get_cdecl_name(node);
				fprintf(f, "%s", name);
				node = node->common.sibling;
				if(node) {
					fprintf(f, ", ");
				}
			}
		}
	}
}

static void gen_iface_method_header(object_t *obj, tree_t *method, FILE *f) {
	tree_t *params;
	const char *ret;

	params = method->method.params;
	if(!params->params.have_ret_param) {
		ret = "void";
	} else {
		ret = get_type_info(params->params.ret_params);
	}
	fprintf(f, "\nstatic %s\n", ret);
	fprintf(f, "_DML_IFACE_%s__%s", obj->qname, method->method.name);
	fprintf(f, "(conf_object_t *obj");	
	print_params_protoc(params, f);
	fprintf(f, ")");
}


static void gen_implement_method(object_t *obj, symbol_t sym, FILE *f) {
	implement_t *impl = (implement_t *)obj;
	method_attr_t *attr = (method_attr_t *)sym->attr;
	tree_t *method = attr->common.node;
	const char *ret = NULL;
	tree_t *params;
	const char *dev;
 	int has_ret = 0;

	params = method->method.params;
	if(!params->params.have_ret_param) {
		ret = "void";
	} else {
		ret = get_type_info(params->params.ret_params);
	}
	gen_iface_method_header(obj, method, f);	
	fprintf(f, "\n");
	fprintf(f, "{\n");
	dev = DEV->name;
	fprintf(f, "\t%s_t *_dev = (%s_t *)obj;\n", dev, dev);
	if(strcmp(ret, "void")) {
		has_ret = 1;	
	}
	if(has_ret) {
		fprintf(f, "\t%s result = 0;\n", ret);
	}
	fprintf(f, "\t{\n");
	fprintf(f, "\t\tbool exec;\n");
	fprintf(f, "\t\texec = _DML_M_%s__%s(_dev", obj->qname, method->method.name);
	print_params_in(params, f);
	if(has_ret) {
		fprintf(f, ", &result");
	}
	fprintf(f, ");\n");
	fprintf(f, "\t\tif(exec) {\n");
	fprintf(f, "\t\t\tgoto throw;\n");
	fprintf(f, "\t\t}\n");
	fprintf(f, "\t}\n");
	fprintf(f, "throw: ;\n");
	if(has_ret) {
		fprintf(f, "\treturn result;\n");
	}
	fprintf(f, "}\n");
	add_object_method(obj, method->method.name);
}

static void gen_implement_code(object_t *obj, FILE *f) {
	implement_t *imp = (implement_t *)obj;
	symtab_t table = obj->symtab->sibling;
    symbol_list_t *list = symbol_list_find_type(table, METHOD_TYPE);
    symbol_list_t *head = list;
    symbol_t sym;

    while(list) {
        sym = list->sym;
        gen_implement_method(obj, sym, f);
        list = list->next;
    }
    symbol_list_free(head);
}

static void gen_implement_method_header(object_t *obj, symbol_t sym, FILE *f) {
	tree_t *method;
	method_attr_t *attr;

	attr = (method_attr_t *)sym->attr;
	method = attr->common.node;
	gen_iface_method_header(obj, method, f);
	fprintf(f, ";\n");
}

static void gen_implement_header(object_t *obj, FILE *f) {
	implement_t *imp = (implement_t *)obj;
	symtab_t table = obj->symtab->sibling;
    symbol_list_t *list = symbol_list_find_type(table, METHOD_TYPE);
    symbol_list_t *head = list;
    symbol_t sym;

    while(list) {
        sym = list->sym;
        gen_implement_method_header(obj, sym, f);
        list = list->next;
    }
    symbol_list_free(head);
}

static void gen_iface(object_t *obj, FILE *f) {
	const char *name;
	const char *qname;
	implement_t *imp = (implement_t *)obj;
	symtab_t table = obj->symtab->sibling;
    symbol_list_t *list = symbol_list_find_type(table, METHOD_TYPE);
    symbol_list_t *head = list;
    symbol_t sym;
	tree_t *method;
	method_attr_t *attr;

	name = obj->name;
	qname = obj->qname;
	fprintf(f, "\nstatic const %s_interface_t %s_iface = {\n", name, qname);
    while(list) {
        sym = list->sym;
		attr = (method_attr_t *)sym->attr;	
		method = attr->common.node;
		fprintf(f, "\t.%s = _DML_IFACE_%s__%s,\n", method->method.name, qname,  method->method.name);
        list = list->next;
    }
	fprintf(f, "};\n");
    symbol_list_free(head);
}

static void gen_device_iface_desc(device_t *dev, FILE *f) {
	const char *dev_name = dev->obj.name;
	struct list_head *p;
	object_t *tmp;
	int i = 0;
	
	fprintf(f, "\nstatic const struct InterfaceDescription %s_ifaces[] = {\n", dev_name);
	list_for_each(p, &dev->implements) {
		tmp = list_entry(p, object_t, entry);
		if(!strcmp(tmp->name, "io_memory")) {
			continue;
		}
		fprintf(f, "\t[%d] = (struct InterfaceDescription ) {\n", i++);
		fprintf(f, "\t\t.name = \"%s\",\n", tmp->name);
		fprintf(f, "\t\t.iface = &%s_iface,\n", tmp->qname);
		fprintf(f, "\t},\n");
	}
	fprintf(f, "\t[%d] = {}\n", i);
	fprintf(f, "};\n");
}

void gen_device_implement_code(device_t *dev, FILE *f) {
	struct list_head *p;
	object_t *tmp;

	list_for_each(p, &dev->implements) {
		tmp = list_entry(p, object_t, entry);
		if(!strcmp(tmp->name, "io_memory")) {
			continue;
		}
		gen_implement_code(tmp, f);
	}
}

void gen_device_implement_desc(device_t *dev, FILE *f) {
	struct list_head *p;
	object_t *tmp;

	list_for_each(p, &dev->implements) {
		tmp = list_entry(p, object_t, entry);
		if(!strcmp(tmp->name, "io_memory")) {
			continue;
		}
		gen_iface(tmp, f);
	}
	gen_device_iface_desc(dev, f);
}

void gen_device_implement_header(device_t *dev, FILE *f) {
	struct list_head *p;
	object_t *tmp;

	list_for_each(p, &dev->implements) {
		tmp = list_entry(p, object_t, entry);
		if(!strcmp(tmp->name, "io_memory")) {
			continue;
		}
		gen_implement_header(tmp, f);
	}
}
