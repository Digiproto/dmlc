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
#include "skyeye_gen_attribute.h"

extern object_t *DEV;
extern void gen_inline_method(object_t *, const char *name);
/**
 * @brief gen_attribute_set : generate the code of set method in attribute object
 *
 * @param obj : the object of attribute
 * @param f : file to be generated
 */
static void gen_attribute_set(device_t* dev, object_t *obj, FILE *f) {
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
	fprintf(f, "exception_t");
	fprintf(f, "\n");
	fprintf(f, "set_attr_%s(void* arg, conf_object_t* conf_obj, attr_value_t* value, attr_value_t* idx){", name);
	fprintf(f, "\n\t%s_t *dev = (%s_t *)(conf_obj->obj);", dev->obj.name, dev->obj.name);
	fprintf(f, "\n\tdev->%s = SKY_attr_uinteger(*value);", name);
        fprintf(f, "\n\treturn No_exp;");
	fprintf(f, "\n}\n");
	return;
}

/**
 * @brief gen_attribute_get : generate the code of get method in attribute object
 *
 * @param obj : the object of attribute
 * @param f : file to be generated
 */
static void gen_attribute_get(device_t* dev, object_t *obj, FILE *f) {
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
	fprintf(f, "get_attr_%s(void* arg, conf_object_t* conf_obj, attr_value_t* idx){", name);
	fprintf(f, "\n\t%s_t* dev = (%s_t*)(conf_obj->obj);", dev->obj.name, dev->obj.name);
        fprintf(f, "\n\treturn SKY_make_attr_uinteger(dev->%s)", name);
	fprintf(f, "\n}\n");
}

/**
 * @brief gen_attribute_code : generate the code of attribute
 *
 * @param obj : the object of attribute
 * @param f : file to be generated
 */
static void gen_attribute_code(device_t* dev,object_t *obj, FILE *f) {
	attribute_t *attr = (attribute_t *)obj;
	const char *type = attr->alloc_type;
	const char *name;

	if(obj->is_array) {
		name = obj->a_name;	
	} else {
		name = obj->name;
	}
	gen_attribute_set(dev, obj, f);
	gen_attribute_get(dev, obj, f);
}

/**
 * @brief gen_device_attribute : entry to generate the code of attribute
 *
 * @param dev : the object of device
 * @param f : file to be generated
 */
void sky_gen_device_attribute(device_t *dev, FILE *f) {
	struct list_head *p;
	object_t *obj;
	bank_t *bank;
	object_t *reg;
	int i, j;

	list_for_each(p, &dev->attributes) {
		obj = list_entry(p, object_t, entry);
		gen_attribute_code(dev, obj, f);
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
void sky_gen_device_attribute_description(device_t *dev, FILE *f) {
       struct list_head *p;
       object_t *tmp;
       int i = 0;
        const char *name;

       sky_gen_device_attribute(dev, f);
       //fprintf(f, "\nconst struct AttributeDescription %s_attributes[] = {\n", dev->obj.name);
	fprintf(f, "static void %s_register_attribute(conf_class_t* class) {\n", dev->obj.name);
       list_for_each(p, &dev->attributes) {
               tmp = list_entry(p, object_t, entry);
                if(tmp->is_array) {
                        name = tmp->a_name;
                } else  {
                        name = tmp->name;
                }
		fprintf(f, "\n\tSKY_register_attribute(class, \"%s\", get_%s, NULL, set_%s, NULL, SKY_Attr_Optional, \"uinteger\", \"\");", name, name, name);
		#if 0
               fprintf(f, "\t[%d] = (struct AttributeDescription) {\n", i);
               fprintf(f, "\t\t.name = \"%s\",\n", name);
               fprintf(f, "\t\t.set = %s_set,\n", name);
               fprintf(f, "\t\t.get = %s_get,\n", name);
               fprintf(f, "\t},\n");
		#endif
               i++;
       }
       //fprintf(f, "\t[%d] = {}\n", i);
       //fprintf(f, "};\n");
	fprintf(f, "\n\treturn;");
	fprintf(f, "\n}\n");
}

