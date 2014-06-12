/*
 * gen_attribute.c: 
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
#include "gen_attribute.h"

extern object_t *DEV;

/**
 * @brief gen_attribute_set : generate the code of set method in attribute object
 *
 * @param obj : the object of attribute
 * @param f : file to be generated
 */
static void gen_attribute_set(object_t *obj, FILE *f) {
	attribute_t *attr = (attribute_t *)obj;
	int type = attr->alloc;
	const char *name;
	const char *type_name = attr->alloc_type;

	if(obj->is_array) {
		name = obj->a_name;	
	} else {
		name = obj->name;
	}
	fprintf(f, "\nstatic ");
	fprintf(f, "set_error_t");
	fprintf(f, "\n");

	fprintf(f, "%s_set(", name);
	fprintf(f, "void *_, ");
	fprintf(f, "conf_object_t *obj, attr_value_t *value, attr_value_t *_id)\n");
	fprintf(f, "{\n");
	if(attr->alloc_type) {
	fprintf(f, "\t%s_t *_dev = (%s_t *)obj;\n", DEV->name, DEV->name);
	fprintf(f, "\tattr_value_t *tmp;\n");
	if(obj->is_array) {
		fprintf(f, "\tunsigned index = SIM_attr_integer(*_id);\n");
		fprintf(f, "\ttmp = &value->u.list.vector[%s];\n", "index","index");
	} else {
		fprintf(f, "\ttmp = value;\n");
	}
	if(obj->is_array) {
		fprintf(f, "\t_dev->%s[%s]", obj->a_name, "index");
	} else {
		fprintf(f, "\t_dev->%s", obj->name);
	}
	fprintf(f, " = ");
	if(type == INT_T || type == LONG_T) {
		fprintf(f, "SIM_attr_integer(*tmp);\n");
	} else if(type == BOOL_T){
		fprintf(f, "SIM_attr_boolean(*tmp);\n");
	} else if (type == DOUBLE_T) {
		fprintf(f, "SIM_attr_floating(*tmp);\n");
	} else if (type == STRING_T) {
		fprintf(f, "SIM_attr_string(*tmp);\n");
	} else {
		fprintf(f, "SIM_attr_object(*tmp.u.obj);\n");
	}
	}
	fprintf(f, "\treturn 0;\n");
	fprintf(f, "}\n");
}

/**
 * @brief gen_attribute_get : generate the code of get method in attribute object
 *
 * @param obj : the object of attribute
 * @param f : file to be generated
 */
static void gen_attribute_get(object_t *obj, FILE *f) {
	attribute_t *attr = (attribute_t *)obj;
	int type = attr->alloc;
	const char *name;
	const char *type_name = attr->alloc_type;

	if(obj->is_array) {
		name = obj->a_name;	
	} else {
		name = obj->name;
	}
	fprintf(f, "\nstatic ");
	fprintf(f, "attr_value_t\n");
	fprintf(f, "%s_get(void *_, conf_object_t *obj, attr_value_t *_idx)\n", name);
	fprintf(f, "{\n");
	if(attr->alloc_type) {
	fprintf(f, "\t%s_t *_dev = (%s_t *)obj;\n", DEV->name, DEV->name);
	fprintf(f, "\tattr_value_t attr;\n");
	fprintf(f, "\tmemset(&attr, 0, sizeof(attr));\n");
	if(!obj->is_array) {
		fprintf(f, "\tattr_value_t tmp = SIM_make_attr_integer(_dev->%s);\n", name);
	} else {
		fprintf(f, "\tint i;\n");
		fprintf(f, "\tattr_value_t tmp = SIM_alloc_attr_list(%d);\n", obj->array_size);
		fprintf(f, "\tfor(i = 0;  i < %d; i++) {\n", obj->array_size); 
		if(type == INT_T || type == LONG_T) {
			fprintf(f, "\t\tattr_value_t index = SIM_make_attr_integer(_dev->%s[%s]);\n", name, "i");
		} else if(type == BOOL_T) {
			fprintf(f, "\t\tattr_value_t index = SIM_make_attr_boolean(_dev->%s[%s]);\n", name, "i");
		} else if(type == DOUBLE_T) {
			fprintf(f, "\t\tattr_value_t index = SIM_make_attr_floating(_dev->%s[%s]);\n", name, "i");
		} else if(type == STRING_T) {
			fprintf(f, "\t\tattr_value_t index = SIM_make_attr_string(_dev->%s[%s]);\n", name, "i");
		}
		fprintf(f, "\t\tSIM_attr_list_set_item(&tmp, %s, index);\n", "i");
		fprintf(f, "\t}\n");
	}
	fprintf(f, "\tattr = tmp;\n");
	} else {
		fprintf(f, "\tattr_value_t attr;\n");
	}
	fprintf(f, "\treturn attr;\n");
	fprintf(f, "}\n");
}

/**
 * @brief gen_attribute_code : generate the code of attribute
 *
 * @param obj : the object of attribute
 * @param f : file to be generated
 */
static void gen_attribute_code(object_t *obj, FILE *f) {
	attribute_t *attr = (attribute_t *)obj;
	const char *type = attr->alloc_type;
	const char *name;

	if(obj->is_array) {
		name = obj->a_name;	
	} else {
		name = obj->name;
	}
	gen_attribute_set(obj, f);
	gen_attribute_get(obj, f);
}

/**
 * @brief gen_device_attribute : entry to generate the code of attribute
 *
 * @param dev : the object of device
 * @param f : file to be generated
 */
void gen_device_attribute(device_t *dev, FILE *f) {
	struct list_head *p;
	object_t *obj;
	bank_t *bank;
	object_t *reg;
	int i, j;

	list_for_each(p, &dev->attributes) {
		obj = list_entry(p, object_t, entry);
		gen_attribute_code(obj, f);
	}
	/*
	for(i = 0; i < dev->bank_count; i++) {
		bank = (bank_t *)dev->banks[i];
		for(j = 0; j < bank->reg_count; j++) {
			reg = bank->regs[j]; 
			gen_attribute_code(reg, f);
		}
	}*/
}

/**
 * @brief gen_device_attribute_description : generate the information and struct of attributes
 *
 * @param dev : the object of device
 * @param f : file to be generated
 */
void gen_device_attribute_description(device_t *dev, FILE *f) {
       struct list_head *p;
       object_t *tmp;
       int i = 0;
        const char *name;

       gen_device_attribute(dev, f);
       fprintf(f, "\nconst struct AttributeDescription %s_attributes[] = {\n", dev->obj.name);
       list_for_each(p, &dev->attributes) {
               tmp = list_entry(p, object_t, entry);
                if(tmp->is_array) {
                        name = tmp->a_name;
                } else  {
                        name = tmp->name;
                }
               fprintf(f, "\t[%d] = (struct AttributeDescription) {\n", i);
               fprintf(f, "\t\t.name = \"%s\",\n", name);
               fprintf(f, "\t\t.set = %s_set,\n", name);
               fprintf(f, "\t\t.get = %s_get,\n", name);
               fprintf(f, "\t},\n");
               i++;
       }
       fprintf(f, "\t[%d] = {}\n", i);
       fprintf(f, "};\n");
}

