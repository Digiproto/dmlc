/*
 * gen_struct.c:
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
#include "gen_struct.h"
#include "node.h"
extern symtab_t root_table;

static int is_simics_dir(const char *dir) {
	const char *simics_keyword_str = "simics";
	char *ret;

	ret = strstr(dir, simics_keyword_str);
	if(ret) {
		return 1;
	}
	return 0;
}

static void gen_data_struct(object_t *obj, FILE *f) {
	object_t *parent = obj->parent;
	tree_t *node = obj->node;
	const char *name;
	const char *type;

	name = get_cdecl_name(node);
	type = get_type_info(node);
	fprintf(f, "%s %s;\n", type, name);
}

static void gen_device_data_struct(device_t *dev, FILE *f) {
	struct list_head *p;
	object_t *tmp;

	list_for_each(p, &dev->obj.data) {
		tmp = list_entry(p, object_t, entry);
		printf("add \n");
		fprintf(f, "\t");
		gen_data_struct(tmp, f);
	}
}


/**
 * @brief gen_field_struct : generate the filed into struct
 *
 * @param obj : the object of filed
 * @param f : header file of struct
 */
static void gen_field_struct(object_t *obj, FILE *f) {
    field_t *fld = (field_t *)obj;
    const char *type;

    type = bits2str(fld->len);
	obj->attr_type = type;
    fprintf(f, "\t\t\t%s %s;\n", type, obj->name);
}

/**
 * @brief gen_iface_struct : generate the interface into struct
 *
 * @param obj : the object of interface
 * @param f : header file of struct
 */
static void gen_iface_struct(object_t *obj, FILE *f) {
    fprintf(f, "\t\tconst %s_interface_t *%s;\n", obj->name, obj->name);
}

/**
 * @brief gen_connect_struct : generate the content of conncet struct
 *
 * @param obj : the object of connect
 * @param f : header file of struct
 */
static void gen_connect_struct(object_t *obj, FILE *f) {
    connect_t *con = (connect_t *)obj;
    struct list_head *p;
    object_t *tmp;

    fprintf(f, "\tstruct {\n");
    fprintf(f, "\t\tconst char *port;\n");
    fprintf(f, "\t\tconf_object_t *obj;\n");
    list_for_each(p, &obj->childs) {
        tmp = list_entry(p, object_t, entry);
        gen_iface_struct(tmp, f);
    }
	if(!obj->is_array) {
    	fprintf(f, "\t} %s;\n", obj->name);
	} else {
    	fprintf(f, "\t} %s[%d];\n", obj->a_name, obj->array_size);
	}
}

/**
 * @brief gen_attribute_struct : generate the content of attribute struct
 *
 * @param obj : the object of attribute
 * @param f : header file of struct
 */
static void gen_attribute_struct(object_t *obj, FILE *f) {
	attribute_t *attr = (attribute_t *)obj;

	if(!obj->is_array) {
		fprintf(f, "\t%s %s;\n", attr->alloc_type, obj->name);
	} else {
		fprintf(f, "\t%s %s[%d];\n", attr->alloc_type, obj->a_name, obj->array_size);
	}
}

/**
 * @brief gen_device_attribute : generate the attribute struct
 *
 * @param dev : the device object
 * @param f : header file of struct
 */
static void gen_device_attribute(device_t *dev, FILE *f) {
    struct list_head *p;
    object_t *obj;

    list_for_each(p, &dev->attributes) {
        obj = list_entry(p, object_t, entry);
        gen_attribute_struct(obj, f);
    }
}

/**
 * @brief gen_device_connect : generate the connect struct relied on device connect list
 *
 * @param dev : the device object
 * @param f : heade file of struct
 */
static void gen_device_connect(device_t *dev, FILE *f) {
    struct list_head *p;
    object_t *obj;

    list_for_each(p, &dev->connects) {
        obj = list_entry(p, object_t, entry);
        gen_connect_struct(obj, f);
    }
}

/**
 * @brief gen_device_notifier : generate the nodifier in struct
 *
 * @param dev : the device object
 * @param f : header file of struct
 */
static void gen_device_notifier(device_t *dev, FILE *f) {
	fprintf(f, "\tNotifier notifier;\n");
}

/**
 * @brief gen_register_struct : generate the content of register
 *
 * @param obj : the object of register
 * @param f : header file of struct
 */
static void gen_register_struct(object_t *obj, FILE *f) {
    object_t *fld;
    dml_register_t *reg = (dml_register_t *)obj;
    const char *type;
    int reg_size;
    struct list_head *p;
    field_t *first;

    reg_size = reg->size;
    p = obj->childs.next;
    fld = list_entry(p, object_t, entry);
    first = (field_t *)fld;
    type = size2str(reg_size);
    if(first->is_dummy) {
        /*dummy  field*/
        type = size2str(reg_size);
		if(!reg->is_array) {
        	fprintf(f, "\t\t%s %s;\n", type, obj->name);
		} else {

        	fprintf(f, "\t\t%s %s[%d];\n", type, obj->a_name, reg->array_size);
		}
        obj->attr_type = type;
    } else {
        fprintf(f, "\t\tstruct {\n");
        list_for_each(p, &obj->childs) {
            fld = list_entry(p, object_t, entry);
            gen_field_struct(fld, f);
        }
		if(!reg->is_array) {
        	fprintf(f, "\t\t} %s;\n", obj->name);
		} else {
        	fprintf(f, "\t\t} %s[%d];\n", obj->a_name, reg->array_size);
		}
		type = size2str(reg_size);
        obj->attr_type = type;
    }
}

/**
 * @brief gen_bank_struct : generate the content of bank
 *
 * @param obj : the bank object
 * @param f : header file of struct
 */
static void gen_bank_struct(object_t *obj, FILE *f) {
    object_t *reg;
    struct list_head *p;
    bank_t *bank = (bank_t *)obj;
    bank_attr_t *attr = obj->node->common.attr;

    fprintf(f, "\tstruct {\n");
    list_for_each(p, &obj->childs) {
        reg = list_entry(p, object_t, entry);
        gen_register_struct(reg, f);
    }
    fprintf(f, "\t} %s;\n", obj->name);
#if backend != 3
    fprintf(f, "\tMemoryRegion mr_%s;\n", obj->name);
#endif
}

/**
 * @brief gen_banks_struct : generate the bank struct relied bank list of device
 *
 * @param dev : the device object
 * @param f : header file of struct
 */
static void gen_banks_struct(device_t *dev, FILE *f) {
    object_t *obj;
    struct list_head *p;

    list_for_each(p, &dev->obj.childs) {
        obj = list_entry(p, object_t, entry);
        gen_bank_struct(obj, f);
    }
}

/**
 * @brief gen_bank_marco : generate the macro for bank size
 *
 * @param obj : the object of bank
 * @param f : header file of struct
 */
static void gen_bank_marco(object_t *obj, FILE *f) {
    bank_t *bank = (bank_t *)obj;
    char *cap = to_upper(obj->name);
    fprintf(f, "\n#define %s_MR_SIZE 0x%x\n", cap, bank->size);
    free(cap);
}

/**
 * @brief gen_banks_macro : entry to generate macro for every bank size based on bank list of device
 *
 * @param dev : the device object
 * @param f : header file of struct
 */
static void gen_banks_macro(device_t *dev, FILE *f) {
    object_t *obj;
    struct list_head *p;

    list_for_each(p, &dev->obj.childs) {
        obj = list_entry(p, object_t, entry);
        gen_bank_marco(obj, f);
    }
}

/**
 * @brief gen_device_loggroup : generate the loggroup struct
 *
 * @param dev : the device object
 * @param f : header file of struct
 */
void gen_device_loggroup(device_t *dev, FILE *f) {
    symbol_list_t *list,*head;
	const char *dev_name = dev->obj.name;
    symbol_t sym;
    int i;

    list = head = symbol_list_find_type(root_table, LOGGROUP_TYPE);
    if(list) {
        fprintf(f, "\ntypedef enum %s_log_group {\n", dev_name);
        i = 1;
        while(list) {
            sym = list->sym;
            fprintf(f, "\t%s = %d", sym->name, i);
            list = list->next;
            if(list) {
                fprintf(f, ",\n");
            } else {
                fprintf(f, "\n");
            }
            i <<= 1;
        }
        fprintf(f, "} %s_log_group_t;\n", dev_name);
    }
	symbol_list_free(head);
}

static int cdecl_is_complex_type(cdecl_t *type) {
	int ret;

	switch(type->common.categ) {
		case STRUCT_T:
		case LAYOUT_T:
		case BITFIELDS_T:
			ret = 1;
			break;
		default:
			ret = 0;
	}
	return ret;
}

static void handle_complex_case(cdecl_t *type, FILE *f, int pos);
extern FILE *out;
static void handle_struct(cdecl_t *type, FILE *f, int pos) {
	symtab_t table;
	symbol_t sym;

	out = f;
	D_n(pos, "struct {\n");
	table = type->struc.table;
	sym = table->list;
	while(sym) {
		cdecl_t *type = sym->attr;
		if(cdecl_is_complex_type(type)) {
			handle_complex_case(type, f, pos + 1);
		} else {
			const char *name = get_cdecl_name(type->node);
			const char *stype = get_type_info(type->node);
			D_n(pos+1, "%s %s;\n", stype, name);
		}
		sym = sym->lnext;
	}
	D_n(pos, "} %s;\n", type->var_name);
}

static void handle_layout(cdecl_t *type, FILE *f, int pos) {
	out = f;
	int size;
	const char *name;

	size = type->common.size;
	name = type->var_name;
	D_n(pos, "struct { char data[%d] } %s;\n", size, name);
}

static void handle_bitfields(cdecl_t *type, FILE *f, int pos) {
	out = f;
	int size;

	size = type->bitfields.size;
	D_n(pos, "uint%d %s;\n", size, type->var_name);
}

static void handle_complex_case(cdecl_t *type, FILE *f, int pos) {
	switch (type->common.categ) {
		case STRUCT_T:
			handle_struct(type, f, pos);
			break;
		case LAYOUT_T:
			handle_layout(type, f, pos);
			break;
		case BITFIELDS_T:
			handle_bitfields(type, f, pos);
		default:
			break;
	}
}

static void gen_structtypedef(node_entry_t *entry, FILE *f) {
	symbol_t sym;

	sym = entry->sym;
	fprintf(f, "typedef struct %s %s;\n", sym->name, sym->name);
}

static void gen_structdef(node_entry_t *entry, FILE *f) {
	symbol_t sym;
	struct_attr_t *attr;
	symtab_t table;
	tree_t *node;

	sym = entry->sym;
	attr = sym->attr;
	node = attr->common.node;
	table = attr->table;
	symbol_t tmp = table->list;
	fprintf(f, "\nstruct %s {\n", sym->name);
	//fprintf(stderr, "struct name %s, file %s, line %d\n",sym->name, node->common.location.file->name, node->common.location.first_line);
	while(tmp) {
		cdecl_t *type = tmp->attr;
		if(!cdecl_is_complex_type(type)) {
			const char *name;
			const char *stype;
			name = get_cdecl_name(type->node);
			stype = get_type_info(type->node);
			fprintf(f, "\t%s %s;\n", stype, name);
		} else {
			handle_complex_case(type, f, 1);
		}
		tmp = tmp->lnext;
	}
	fprintf(f, "};\n");
}

void gen_device_toplevel_struct(device_t *dev, FILE *f) {
	node_entry_t *tmp;
	struct list_head *p;

	list_for_each(p, &dev->struct_defs) {
		tmp = list_entry(p, node_entry_t, entry);
		gen_structtypedef(tmp, f);
	}

	list_for_each(p, &dev->struct_defs) {
		tmp = list_entry(p, node_entry_t, entry);
		gen_structdef(tmp, f);
	}
}

static void gen_typedef(node_entry_t *entry, FILE *f) {
	symbol_t sym;
	cdecl_t *type;
	tree_t *node;

	sym = entry->sym;
	type = sym->attr;
	node = type->node;
	const char *fname = node->common.location.file->name;
	if(is_simics_dir(fname)) {
		return;
	}
	const char *stype = get_type_info(node);
	const char *name = get_cdecl_name(node);
	out = f;
	gen_src_loc(&node->common.location);
	fprintf(f, "typedef %s %s;\n", stype, name);
}

void gen_device_typedef(device_t *dev, FILE *f) {
	struct list_head *p;
	node_entry_t *tmp;

	list_for_each(p, &dev->typedefs) {
		tmp = list_entry(p, node_entry_t, entry);
		gen_typedef(tmp, f);
	}
}


/**
 * @brief gen_device_macros : generate the macro about device
 *
 * @param dev : the device object
 * @param f : header file of struct
 */
void gen_device_macros(device_t *dev, FILE *f) {
	gen_banks_macro(dev, f);
}

/**
 * @brief gen_device_struct : generate the content of device struct
 *
 * @param dev : the object of device
 * @param f : file of struct header file
 */
void gen_device_struct(device_t *dev, FILE *f) {
	gen_banks_struct(dev, f);
	gen_device_connect(dev, f);
	gen_device_attribute(dev, f);
#if backend == 1
	gen_device_notifier(dev, f);
#endif
	gen_device_data_struct(dev, f);
}
