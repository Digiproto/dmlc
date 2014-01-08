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
extern symtab_t root_table;

static void gen_device_data_struct(object_t *obj, FILE *f) {
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
        fprintf(f, "typedef enum %s_log_group {\n", dev_name);
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
}
