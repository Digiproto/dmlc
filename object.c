/*
 * object.c: 
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
/**
 * @file object.c
 * @brief object
 * @author alloc, alloc.young@gmail.com
 * @version 
 * @date 2013-05-10
 */

#include "object.h"
#include <string.h>
#include "gen_debug.h"
extern symtab_t root_table;
object_t *OBJ;
object_t *DEV;
static const char tab[8][8] =
    { {""}, {"\t"}, {"\t\t"}, {"\t\t\t"}, {"\t\t\t\t"}, {"\t\t\t\t\t"} };

static const char *string_concat_with(const char *s1,const char *s2,const char *connector ){
	int len = strlen(s1) + strlen(s2) + 1 + strlen(connector);
	char *d = gdml_zmalloc(len);
	strncpy(d,s1,strlen(s1));
	strcat(d,connector);
	strcat(d,s2);
	return d;
}

const char *get_obj_qname(object_t *obj){
	return obj->qname;
}

const char *get_obj_ref(object_t *obj){
	return obj->dotname;
}

static void process_object_names(object_t *obj) {
	if(OBJ && (!strcmp(obj->obj_type,"register") || !strcmp(obj->obj_type,"field") || !strcmp(obj->obj_type, "interface"))){
		obj->qname = string_concat_with(OBJ->qname,obj->name,"__");
		obj->dotname = string_concat_with(OBJ->dotname,obj->name,".");
	}
	else{
		obj->qname = obj->name;
		if(!strcmp(obj->obj_type,"device"))
			obj->dotname = "_dev";
		else {
			obj->dotname = string_concat_with(OBJ->dotname,obj->name,"->");
		}
	}
}

static symbol_t object_symbol_find(symtab_t table, const char *name, type_t type) {
	symbol_t sym = NULL;
	object_t *obj;

	obj = table->obj;
	BE_DBG(OBJ_SYM, "IN object: try to search symbol %s, %s\n", name, obj->name);
	sym = _symbol_find(table->table, name, type);
	if(sym) {
		sym->owner = table->obj;
		BE_DBG(OBJ_SYM, "symbol found %s in object\n", sym->name);
		return sym;
	} else if (table->sibling){
		BE_DBG(OBJ_SYM, "before sibling, %p; root table  %p , cb %p\n", table->sibling, root_table, root_table->cb);
		sym = symbol_find(table->sibling, name, type);
		if(sym) {
			sym->owner = table->obj;
			BE_DBG(OBJ_SYM, "symbol %s found in sibling \n", sym->name);
			return sym;
		} else {
			BE_DBG(OBJ_SYM, "symbol %s not found in sibling\n", name);
		}
	}
	BE_DBG(OBJ_SYM, "here goto parent\n");
	if(table->parent) {
		sym = symbol_find(table->parent, name, type);
	}
	BE_DBG(OBJ_SYM, "symbol %s found in parent symtable\n", sym->name);
	return sym;

}

static symbol_t object_symbol_find_notype(symtab_t table, const char *name) {
	symbol_t sym = NULL;
	object_t *obj;

	obj = table->obj;
	BE_DBG(OBJ_SYM,"try to search symbol %s in object %s\n", name, obj->name);
	sym = _symbol_find_notype(table->table, name);
	if(sym) {
		sym->owner = table->obj;
		BE_DBG(OBJ_SYM, "symbol found %s in object\n", sym->name);
		return sym;
	} else if (table->sibling){
		BE_DBG(OBJ_SYM,"before sibling, %p; root table  %p , cb %p num: %d\n", table->sibling, root_table, root_table->cb, table->sibling->table_num);
		sym = symbol_find_notype(table->sibling, name);
		if(sym) {
			sym->owner = table->obj;
			BE_DBG(OBJ_SYM, "symbol %s found in sibling \n", sym->name);
			return sym;
		} else {
			BE_DBG(OBJ_SYM, "symbol %s not found in sibling\n", name);
		}
	}
	BE_DBG(OBJ_SYM, "here goto parent\n");
	if(table->parent) {
		sym = symbol_find_notype(table->parent, name);
	}
	BE_DBG(OBJ_SYM, "symbol %s found in parent symtable\n", sym->name);
	return sym;

}

static object_type_t get_object_encoding(const char *name) {
	if(!strcmp(name, "device")) {
		return Obj_Type_Device;
	} else if(!strcmp(name, "bank")) {
		return Obj_Type_Bank;
	} else if(!strcmp(name, "register")) {
		return Obj_Type_Register;
	} else if(!strcmp(name, "field")) {
		return Obj_Type_Field;
	}
	return Obj_Type_None;
}

static void process_object_relationship(object_t *obj);
static void init_object(object_t *obj, const char *name, const char *type, tree_t *node, symtab_t table) {	
	int i;
	device_t *dev = (device_t *)DEV;
	struct list_head *list;

	obj->name = name;
	obj->parent = OBJ;
	obj->obj_type = type;
	obj->encoding = get_object_encoding(type);
	obj->node = node;
	obj->symtab = symtab_create_with_cb(OBJECT_TYPE, object_symbol_find, object_symbol_find_notype);
	if(!table) {
		obj->symtab->sibling = symtab_create(TMP_TYPE);
	} else {
		obj->symtab->sibling = table;
	}
	obj->symtab->sibling->parent  = NULL;
	obj->symtab->obj = obj;
	if(OBJ) {
		obj->symtab->parent = OBJ->symtab;
	} else {
		obj->symtab->parent = NULL;
	}
	BE_DBG(OBJ, "obj %s, symtab %p , sibling %p, parent table %p\n", obj->name, obj->symtab, obj->symtab->sibling, obj->symtab->parent);
	struct list_head *p = &obj->childs;
	for(i = 0; i < 6; i++, p++){
		INIT_LIST_HEAD(p);
	}
	process_object_names(obj);
	process_object_relationship(obj);
}

static void process_object_relationship(object_t *obj) {
	device_t *dev = (device_t *)DEV;
	struct list_head *list;

	if(OBJ){
		obj->symtab->parent = OBJ->symtab;
		if(!strcmp(obj->obj_type, "connect")) {
			list = &dev->connects; 	
		} else if(!strcmp(obj->obj_type, "implement")) {
			list = &dev->implements;
		} else if(!strcmp(obj->obj_type, "attribute")) {
			list = &dev->attributes;
		} else {
			list = &OBJ->childs;
		}

		list_add_tail(&obj->entry, list);	
		symbol_insert(OBJ->symtab, obj->name, OBJECT_TYPE, obj);
	} 
}

static void create_bank_objs(symtab_t table);
static void create_connect_objs(symtab_t table);
static void create_attribute_objs(symtab_t table);
object_t *create_device_object(symbol_t sym){
	int i = 0;
	struct list_head *p;

	device_t *dev =  (device_t *)gdml_zmalloc(sizeof(*dev));
	if(OBJ)
		BE_DBG(OBJ, "device exists\n");
	device_attr_t *dev_attr = (device_attr_t *)sym->attr;
	BE_DBG(OBJ, "device %s found\n",sym->name);
	init_object(&dev->obj, sym->name, "device", dev_attr->common.node, root_table);
	p = &dev->constants;
	for(i = 0; i < 6; i++, p++) {
		INIT_LIST_HEAD(p);
	}
	OBJ = &dev->obj;
	DEV = OBJ;
	create_bank_objs(root_table);
	create_connect_objs(root_table);
	create_attribute_objs(root_table);
	return &dev->obj;
}

static void create_bank_object(symbol_t sym);
void create_bank_objs(symtab_t table) {
	symbol_list_t *list = symbol_list_find(table, BANK_TYPE);
	symbol_list_t *head = list;
	object_t *obj = OBJ;

	while(list) {
		create_bank_object(list->sym);	
		/*restore OBJ to device*/
		OBJ = obj;
		list = list->next;
	}
	symbol_list_free(head);
}

static void create_field_object(symbol_t sym){
	if(!OBJ || strcmp(OBJ->obj_type,"register"))
		BE_DBG(OBJ, "field %s not in right place\n",sym->name);
	BE_DBG(OBJ, "found %s in register %s\n", sym->name, OBJ->name);
	field_t *fld = gdml_zmalloc(sizeof(*fld));
	field_attr_t *field_attr = (field_attr_t *)sym->attr;
	init_object(&fld->obj,sym->name,"field",field_attr->common.node, field_attr->common.table);
}


static void create_dummy_field(void) {
	field_t *fld = (field_t *)gdml_zmalloc(sizeof(*fld));
	fld->is_dummy = 1;	
	init_object(&fld->obj, "dummy","field",NULL, NULL);
}

static void create_field_objs(symtab_t symtab){
	symbol_list_t *list = NULL;
	symbol_list_t *head = list;
	symbol_t sym;
	object_t *obj = OBJ;
	int i = 0;
	
	if(!symtab) {
		create_dummy_field();
		return;
	} else {
		list = symbol_list_find(symtab, FIELD_TYPE);
	}
	while(list){
		sym = list->sym;
		create_field_object(sym);
		OBJ = obj;
		list = list->next;
		i++;
	};
	if(i == 0) {
		create_dummy_field();
	}
	symbol_list_free(head);
}

static void create_register_object(symbol_t sym){
	if(!OBJ || strcmp(OBJ->obj_type,"bank"))
		BE_DBG(OBJ, "register %s not in right place\n",sym->name);
	BE_DBG(OBJ, "register %s found in bank %s\n", sym->name, OBJ->name);
	dml_register_t *reg = (dml_register_t *)gdml_zmalloc(sizeof(*reg));
	register_attr_t *reg_attr = (register_attr_t *)(sym->attr);
	symtab_t table = reg_attr->common.table;

	init_object(&reg->obj,sym->name,"register",reg_attr->common.node, table);
	OBJ = &reg->obj;
	create_field_objs(table);
	//print_all_symbol(reg->obj.symtab);
}

static void create_iface_object(symbol_t sym) {
	if(!OBJ || strcmp(OBJ->obj_type,"connect"))
		BE_DBG(OBJ, "iface %s not in right place\n",sym->name);
	BE_DBG(OBJ, "iface %s found in connect %s\n", sym->name, OBJ->name);
	interface_t *iface = (interface_t *)gdml_zmalloc(sizeof(*iface));
	interface_attr_t *attr = (interface_attr_t *)sym->attr;
	symtab_t table = attr->common.table;
	init_object(&iface->obj, sym->name, "interface", attr->common.node, table);
}

static void create_ifaces_objs(symtab_t table) {
	symbol_list_t *list, *head;
	symbol_t sym;
	object_t *obj;

	obj = OBJ;
	list = head = symbol_list_find(table, INTERFACE_TYPE);
	while(list) {
		sym = list->sym;
		create_iface_object(sym);
		OBJ = obj;
		list = list->next;
	}
	symbol_list_free(head);
}

static void create_connect_object(symbol_t sym) {
	if(!OBJ || strcmp(OBJ->obj_type, "device")) 
		BE_DBG(OBJ, "connect %s not in right place\n",sym->name);
	BE_DBG(OBJ, "connect %s found in device %s", sym->name, OBJ->name);
	connect_t *con = (connect_t *)gdml_zmalloc(sizeof(*con));
	connect_attr_t *attr = (connect_attr_t *)(sym->attr);
	symtab_t table = attr->common.table;

	init_object(&con->obj, sym->name, "connect", attr->common.node, table);
	OBJ = &con->obj;
	create_ifaces_objs(table);
}

static void create_attribute_object(symbol_t sym) {
	if(!OBJ || strcmp(OBJ->obj_type, "device")) 
		BE_DBG(OBJ, "attribute %s not in right place\n",sym->name);
	BE_DBG(OBJ, "attribute %s found in device %s", sym->name, OBJ->name);
	attribute_t *att = (attribute_t *)gdml_zmalloc(sizeof(*att));
	attribute_attr_t *attr = (attribute_attr_t *)(sym->attr);
	symtab_t table = attr->common.table;

	init_object(&att->obj, sym->name, "attribute", attr->common.node, table);
}

static void create_connect_objs(symtab_t table) {
	symbol_list_t *list = symbol_list_find(table, CONNECT_TYPE);
	symbol_list_t *head = list;
	symbol_t sym;
	object_t *obj = OBJ;

	while(list) {
		sym = list->sym;
		create_connect_object(sym);
		OBJ = obj;
		list = list->next;
	}
	symbol_list_free(head);
} 

static void create_attribute_objs(symtab_t table) {
	symbol_list_t *list = symbol_list_find(table, ATTRIBUTE_TYPE);
	symbol_list_t *head = list;
	symbol_t sym;
	object_t *obj = OBJ;

	while(list) {
		sym = list->sym;
		create_attribute_object(sym);
		OBJ = obj;
		list = list->next;
	}
	symbol_list_free(head);
} 

static void create_register_objs(symtab_t table){
	symbol_list_t *list = symbol_list_find(table, REGISTER_TYPE);
	symbol_list_t *head = list;
	symbol_t sym;

	object_t *obj = OBJ;
	while(list){
		sym = list->sym;
		create_register_object(sym);
		OBJ = obj;
		list = list->next;
	}
	symbol_list_free(head);
}

static void create_bank_object(symbol_t sym){
	if(!OBJ || strcmp(OBJ->obj_type,"device"))
		BE_DBG(OBJ, "bank %s not in right place\n",sym->name);
	bank_t *bank = gdml_zmalloc(sizeof(*bank));
	BE_DBG(OBJ, "bank %s found in device %s\n",sym->name, OBJ->name);
	bank_attr_t *b = (bank_attr_t *)(sym->attr);
	symtab_t table = b->common.table;
	init_object(&bank->obj,sym->name,"bank",b->common.node, table);		
	OBJ = &bank->obj;
	create_register_objs(table);
	//print_all_symbol(table);
}

device_t *create_device_tree(tree_t  *root){
	object_t *obj;

	//print_all_symbol(root_table);
	symbol_list_t *dev_list = symbol_list_find(root_table, DEVICE_TYPE);
	if(!dev_list || dev_list->next) {
		BE_DBG(OBJ, "device not correct\n");
		exit(-1);
	}
	symbol_t sym = dev_list->sym;
	obj = create_device_object(sym);
	symbol_list_free(dev_list);
	return (device_t *)DEV;
}
static void add_object_templates(object_t *obj, tree_t *node);
void create_template_name(object_t *obj, const char *name) {
	symbol_t sym;
	template_attr_t *tem_attr;
	struct template_name *tmp;

	sym = symbol_find(root_table, name, TEMPLATE_TYPE);
	if(sym) {
		tmp = (struct template_name *)gdml_zmalloc(sizeof(*tmp));
		tmp->name = strdup(name);
		INIT_LIST_HEAD(&tmp->entry);
		tem_attr = sym->attr;	
		tmp->node = tem_attr->common.node;
		list_add_tail(&obj->templates, &tmp->entry);
	} else {
		BE_DBG(OBJ, "template %s not found \n", name);
	}
}

static void field_realize(object_t *obj) {
	field_t *fld = (field_t *)obj;
	tree_t *bitrange;
	tree_t *templates = NULL;
	tree_t *expr;

	if(fld->is_dummy) {
		goto end;
	} 
	bitrange = obj->node->field.bitrange;	
	expr = bitrange->array.expr;
	fld->is_fixed = bitrange->array.is_fix;
	fld->high = expr->int_cst.value; 
	if(fld->is_fixed) {
		fld->len = 1;
	} else {
		expr = bitrange->array.expr_end;
		fld->low = expr->int_cst.value;
		fld->len = fld->high - fld->low + 1;
	}
	templates = obj->node->field.templates;
end:
	add_object_templates(obj, templates);	
}

static void register_realize(object_t *obj) {
	dml_register_t *reg  = (dml_register_t *)obj;
	bank_t *bank = (bank_t *)obj->parent; 
	register_attr_t *reg_attr;
	int i = 0;
	int register_size = bank->register_size;
	struct list_head *p;
	object_t *tmp;
	arraydef_attr_t *arraydef = NULL;

	reg_attr = reg->obj.node->common.attr;
	reg->is_array = reg_attr->is_array;
	obj->is_array = reg->is_array;
	reg->size = reg_attr->size;
	if(!reg->size || reg->size == -1) {
		reg->size = register_size;
	}
	if(reg->is_array) {
		arraydef = reg_attr->arraydef;
		reg->array_size = arraydef->high - arraydef->low + 1;
	}
	reg->offset = reg_attr->offset;
	list_for_each(p, &obj->childs) {
		i++;
	}
	reg->field_count = i;
	reg->fields = (object_t **)gdml_zmalloc(sizeof(obj) * reg->field_count); 
	i = 0;
	list_for_each(p, &obj->childs) {
		tmp = list_entry(p, object_t, entry);
		reg->fields[i] = tmp;
		i++;
		field_realize(tmp);
	}
	add_object_templates(obj, obj->node->reg.templates);
}

static void bank_calculate_register_offset(object_t *obj) {
	bank_t *bank = (bank_t *)obj;
	dml_register_t *reg;
	register_attr_t *reg_attr;
	int i = 0;
	int offset = 0;
	int register_size = bank->register_size;
	int size;

	for(i = 0; i < bank->reg_count; i++) {
		reg = (dml_register_t *)bank->regs[i];
		if(reg->offset == -1) {
			reg->offset = offset;
		}
		offset += reg->size;		
	}
}
static void bank_realize(object_t *obj) {
	bank_t *bank = (bank_t *)obj;
	bank_attr_t *attr = obj->node->common.attr;
	symbol_t sym;
	int reg_size;
	object_t *tmp;
	tree_t *node;
	struct list_head *p;
	int i = 0;
	int offset = 0;
	parameter_attr_t *param;
	paramspec_t *spec;
	expression_t *expr;
	
	BE_DBG(OBJ, "object name %s\n", obj->name);
	sym = symbol_find(obj->symtab, "register_size", PARAMETER_TYPE);
	if(sym) {
		param = (parameter_attr_t *)sym->attr;
		spec = param->spec;
		expr = spec->expr;
		if(!expr->is_const) {
		} else {
			reg_size = expr->const_expr->int_value;
		}
	} else {
		reg_size = 4;
	}

	bank->register_size = reg_size;	
	list_for_each(p, &obj->childs) {
		i++;	
	}
	tmp = (object_t *)gdml_zmalloc(sizeof(tmp) * i);
	bank->regs = (object_t **)tmp;
	bank->reg_count = i;
	bank->size = bank->register_size * bank->reg_count;
	i = 0;
	list_for_each(p, &obj->childs) {
		tmp = list_entry(p, object_t, entry);
		bank->regs[i] = tmp;
		i++;
		register_realize(tmp);
	}
	bank_calculate_register_offset(obj);
	add_object_templates(obj, obj->node->bank.templates);
}

static void connect_realize(object_t *obj) {
	
}

static const char *get_attribute_type(const char *alloc_type) {
	const char *type;

	if(!strcmp(alloc_type, "\"double\"")) {
		type = "double";	
	} else if(!strcmp(alloc_type, "\"string\"")) {
		type = "string";
	} else if(!strcmp(alloc_type, "\"uint8\"")) {
		type = "uint8";
	} else if(!strcmp(alloc_type, "\"uint16\"")) {
		type = "uint16";
	} else if(!strcmp(alloc_type, "\"uint32\"")) {
		type = "uint32";
	} else if(!strcmp(alloc_type, "\"uint64\"")) {
		type = "uint64";
	} else if(!strcmp(alloc_type, "\"int8\"")) {
		type = "int8";
	} else if(!strcmp(alloc_type, "\"int16\"")) {
		type = "int16";
	} else if(!strcmp(alloc_type, "\"int32\"")) {
		type = "int32";
	} else if(!strcmp(alloc_type, "\"int64\"")) {
		type = "int64";
	}
	return type;
}

static void attribute_realize(object_t *obj) {
	tree_t *node;
	symbol_t sym;
	const char *alloc_type;
    parameter_attr_t* attr;
	const char *type;
    paramspec_t* spec; 
	attribute_t *attr_obj = (attribute_t *)obj;

	sym  = symbol_find(obj->symtab, "allocate_type", PARAMETER_TYPE);
	attr = (parameter_attr_t *)sym->attr;
	spec = attr->spec;
	alloc_type = get_attribute_type(spec->str);
	attr_obj->alloc_type = alloc_type;
}

void device_realize(device_t *dev) {
	struct list_head *p;
	object_t *tmp;
	int i = 0;

	list_for_each(p, &dev->obj.childs) {
		tmp = list_entry(p, object_t, entry);
		bank_realize(tmp);
		i++;
	}
	dev->bank_count = i;
	dev->banks = (object_t **)gdml_zmalloc(sizeof(tmp) * i);
	i = 0;
	list_for_each(p, &dev->obj.childs) {
		tmp = list_entry(p, object_t, entry);
		dev->banks[i] = tmp;
		i++;
	}	
	list_for_each(p, &dev->connects) {
		tmp = list_entry(p, object_t, entry);
		connect_realize(tmp);
	}
	list_for_each(p, &dev->attributes) {
		tmp = list_entry(p, object_t, entry);
		attribute_realize(tmp);
	}
	add_object_templates(&dev->obj, NULL);
}

void print_object(object_t *obj, int tab_count) {
	const char *pos = (const char *)tab[tab_count];
	BE_DBG(OBJ, "%sobject type %s, name %s, symtab %p, sibling %p, symtab parent %p\n", pos, obj->obj_type, obj->name, obj->symtab, obj->symtab->sibling, obj->symtab->parent);	
	struct list_head *p;
	object_t *tmp;

	list_for_each(p, &obj->childs) {
		tmp = list_entry(p, object_t, entry);
		print_object(tmp, tab_count + 1);
	}
}

void print_device_tree(device_t *dev) {
	int tab = 0;
	object_t *tmp;
	struct list_head *p;

	print_object(&dev->obj, tab);	
	list_for_each(p, &dev->connects) {
		tmp = list_entry(p, object_t, entry);
		print_object(tmp, 1);
	}
	list_for_each(p, &dev->attributes) {
		tmp = list_entry(p, object_t, entry);
		print_object(tmp, 1);
	}
}

int in_template = 0;
static LIST_HEAD(templates); 

static void add_template(template_def_t  *def){
	struct list_head *p;
	struct template_def *tp;
	list_for_each(p,&templates){
		tp = list_entry(p,struct template_def,entry);
		if(!strcmp(tp->name, def->name)){
			BE_DBG(TEMPLATE, "template %s redifinition,previous definition is here",def->name);
			return;
		}
	}
	list_add_tail(&templates,&def->entry);
}

static struct template_def *lookup_template(const char *name)
{
	struct list_head *p;
	struct template_def *tp;
	list_for_each(p, &templates){
		tp = list_entry(p,struct template_def,entry);
		if(!strcmp(tp->name, name)){
			return tp;
		}
	}
	return NULL;
}

struct template_def *create_template_def(tree_t *t){
	if(in_template)
		BE_DBG(TEMPLATE, "canot define template in template definition\n");
	if(strcmp(OBJ->obj_type,"device"))
		BE_DBG(TEMPLATE, "template definition must at top level\n");
	struct template_def *temp = malloc(sizeof(*temp));
	temp->name = t->common.name;
	temp->node = t;
	int i = 0;
	struct list_head *p = &temp->entry; 
	for(; i < 3; i++,p++){
		INIT_LIST_HEAD(p);
	}
	add_template(temp);
	return temp;
}

static void add_default_template(object_t *obj){
	symtab_t table;	

	create_template_name(obj, obj->obj_type);
	table = obj->symtab->sibling;
	if(table) {
		add_template_to_table(table, obj->obj_type);
	}
}

static void process_device_template(object_t *obj){
	symtab_t table = obj->symtab;
	device_t *dev = (device_t *)obj;
	symbol_t sym;
	int i;
	param_value_t tmp;

	param_value_t *val = gdml_zmalloc(sizeof(*val));	
	val->type = param_type_list;
	val->u.list.size = dev->bank_count;
	val->u.list.vector = gdml_zmalloc(sizeof(*val) * val->u.list.size);
	for(i = 0; i < dev->bank_count; i++) {
		tmp.type = param_type_ref;
		tmp.u.ref = dev->banks[i];
		val->u.list.vector[i] = tmp;
	}
	symbol_insert(table, "banks", PARAMETER_TYPE, val);
	val = (param_value_t *)gdml_zmalloc(sizeof(*val));	
	val->type = param_type_ref;
	val->u.ref = obj;
	symbol_insert(table, "obj", PARAMETER_TYPE, val);
	val = (param_value_t *)gdml_zmalloc(sizeof(*val));	
	val->type = param_type_ref;
	val->u.ref = obj;
	symbol_insert(table, "logobj", PARAMETER_TYPE, val);
	symbol_insert(table, "dev", OBJECT_TYPE, obj);
}

static void process_bank_template(object_t *obj){
	symtab_t table = obj->symtab;
	bank_t *bank = (bank_t *)obj;
	int i;
	param_value_t tmp;

	param_value_t *val = gdml_zmalloc(sizeof(*val));
	val->type = param_type_list;
	val->u.list.size = bank->reg_count;
	val->u.list.vector = gdml_zmalloc(sizeof(*val) * bank->reg_count);
	for(i = 0; i < bank->reg_count; i++) {
		tmp.type = param_type_ref;
		tmp.u.ref = bank->regs[i];
		val->u.list.vector[i] = tmp;
	}
	symbol_insert(table, "mapped_registers", PARAMETER_TYPE, val);
	val = gdml_zmalloc(sizeof(*val));
	val->type = param_type_list;
	val->u.list.size = 0;
	symbol_insert(table, "unmapped_registers", PARAMETER_TYPE, val);
}

static void process_register_template(object_t *obj){
	symtab_t table = obj->symtab;
	dml_register_t *reg = (dml_register_t *)obj;
	int i;
	param_value_t tmp;
	object_attr_t *attr;
	tree_t *node;
	
	node = obj->node;
	attr = node->common.attr;
	param_value_t *val = gdml_zmalloc(sizeof(*val));
	val->type = param_type_list;
	val->u.list.size = reg->field_count;
	val->u.list.vector = gdml_zmalloc(sizeof(*val) * reg->field_count);
	for(i = 0; i < reg->field_count; i++) {
		tmp.type = param_type_ref;
		tmp.u.ref = reg->fields[i];
		val->u.list.vector[i] = tmp;
	}
	symbol_insert(table, "fields", PARAMETER_TYPE, val);
	val = gdml_zmalloc(sizeof(*val));
	val->type = param_type_string;
	val->u.string = strdup(obj->name);
	symbol_insert(table, "_regname", PARAMETER_TYPE, val);
	val = gdml_zmalloc(sizeof(*val));
	val->type = param_type_int;
	val->u.integer = 0;
	symbol_insert(table, "hard_reset_value", PARAMETER_TYPE, val);

	val = gdml_zmalloc(sizeof(*val));
	val->type = param_type_int;
	bank_t *bank = (bank_t *)reg->obj.parent;
	val->u.integer = bank->register_size * 8;
	symbol_insert(table, "bitsize", PARAMETER_TYPE, val);

	val = gdml_zmalloc(sizeof(*val));
	val->type = param_type_bool;
	val->u.boolean = 1;
	if(attr->reg.offset == -1) {
		val->u.boolean = 0;
	}
	symbol_insert(table, "allocate", PARAMETER_TYPE, val);
}

static void process_field_template(object_t *obj){
	symtab_t table = obj->symtab;
	field_t *field = (field_t *)obj;

	param_value_t *val = gdml_zmalloc(sizeof(*val));
	val->type = param_type_bool;
	if(field->is_dummy) {
		val->u.boolean = 0;
	} else {
		val->u.boolean = 1;
	}
	symbol_insert(table, "explicit", PARAMETER_TYPE, val);
	val = gdml_zmalloc(sizeof(*val));
	val->type = param_type_int;
	val->u.integer = field->low;
	symbol_insert(table, "lsb", PARAMETER_TYPE, val);
	val = gdml_zmalloc(sizeof(*val));
	val->type = param_type_int;
	val->u.integer = field->high;
	symbol_insert(table, "msb", PARAMETER_TYPE, val);

	val = gdml_zmalloc(sizeof(*val));
	val->type = param_type_int;
	val->u.integer = field->len;
	symbol_insert(table, "bitsize", PARAMETER_TYPE, val);
	val = gdml_zmalloc(sizeof(*val));
	val->type = param_type_int;
	val->u.integer = 0;
	symbol_insert(table, "hard_reset_value", PARAMETER_TYPE, val);
}

/*process parameters including auto parameters */
static void process_object_template(object_t *obj, struct template_name *name){
	/*process parameters including auto parameters */
	tree_t *node = name->node;
	parameter_attr_t *attr;
	template_attr_t *tmp_attr;
	tmp_attr = node->common.attr;
	symtab_t table = tmp_attr->table;
	symbol_list_t *list;
	symbol_t sym;
	paramspec_t *spec;

	BE_DBG(TEMPLATE, "template name %s, obj name %s\n", name->name, obj->name);
	//list = symbol_list_find(table, PARAMETER_TYPE);
}

static void process_basic_template(object_t *obj) {
	/*hardcode object auto parameter in object symtab*/
	symtab_t table = obj->symtab;
	param_value_t *val = gdml_zmalloc(sizeof(*val));
	val->type = param_type_ref;
	val->u.ref = obj->parent;
	symbol_insert(table, "parent", PARAMETER_TYPE, val);
	val = gdml_zmalloc(sizeof(*val));
	val->type = param_type_string;
	val->u.string = strdup(obj->name);
	symbol_insert(table, "name", PARAMETER_TYPE, val);
	val = gdml_zmalloc(sizeof(*val));
	val->type = param_type_string;
	val->u.string = strdup(obj->qname);
	symbol_insert(table, "qname", PARAMETER_TYPE, val);
	val = gdml_zmalloc(sizeof(*val));
	val->type = param_type_ref;
	val->u.ref = obj;
	symbol_insert(table, "this", PARAMETER_TYPE, val);
}

void object_none_temp_fn(object_t *obj) {
	BE_DBG(TEMPLATE, "some error happend\n");
}

static void (*specific_temp_fn[])(object_t *obj) = {
	[Obj_Type_None] = object_none_temp_fn,
	[Obj_Type_Device] = process_device_template,
	[Obj_Type_Bank] = process_bank_template,
	[Obj_Type_Register] = process_register_template,
	[Obj_Type_Field] = process_field_template,
};

static void process_default_template(object_t *obj) {
	process_basic_template(obj);
	specific_temp_fn[obj->encoding](obj);
	BE_DBG(TEMPLATE, "start ........\nobject Name %s symbols\n", obj->name);
	//print_all_symbol(obj->symtab);
	BE_DBG(TEMPLATE, "end .......");
}

static void process_object_templates(object_t *obj){
	struct template_name *name;
	struct list_head *p;

	process_default_template(obj);
	list_for_each(p, &obj->templates) {
		name = list_entry(p,struct template_name,entry);
		if(name)
			process_object_template(obj, name);
	}
}

static void add_object_templates(object_t *obj, tree_t *t){
	tree_t *it = t;
	struct tree_ident *ident;
	struct template_name *temp;
	
	/*while(it){
		ident = (struct tree_ident *)it;
		create_template_name(obj, ident->str);
		it = it->common.sibling;
	}*/
	add_default_template(obj);
	process_object_templates(obj);
}


int method_to_generate = 0;
void add_object_method(object_t *obj,const char *name){
	struct method_name *m;
	struct list_head *p;
	symbol_t sym;
	method_attr_t *attr;

	if(!strcmp(obj->obj_type,"bank") && (!strcmp(name,"read_access") || !strcmp(name,"write_access"))){
		return;
	}

	list_for_each(p,&obj->method_generated){
		m = list_entry(p,struct method_name,entry);
		if(!strcmp(m->name,name))
			return;
	}

	list_for_each(p,&obj->methods){
		m = list_entry(p,struct method_name,entry);
		if(!strcmp(m->name,name))
			return;
	}
	m = gdml_zmalloc(sizeof(*m));
	m->name = strdup(name);
	INIT_LIST_HEAD(&m->entry);
	BE_DBG(OBJ, "add object %s, name, %s\n", obj->name, name);
	sym = object_symbol_find(obj->symtab, name, METHOD_TYPE);
	if(!sym)
		BE_DBG(OBJ, "object %s has no method %s\n",obj->name, name);
	else {
		attr = sym->attr;	
		m->method = attr->common.node;	
	}
	method_to_generate++;
	list_add_tail(&m->entry, &obj->methods);
}

int object_method_generated(object_t *obj, struct method_name *m){
	struct list_head *p;
	struct method_name *t;

	list_for_each(p,&obj->method_generated){
		t = list_entry(p, struct method_name, entry);
		if(!strcmp(t->name,m->name))
			return 1;
	}
	return 0;
}

void add_object_generated_method(object_t *obj){
	struct list_head *p;
	int i = 0;

	list_for_each(p,&obj->methods){
		i++;
	}

	method_to_generate -= i;
	list_splice_init(&obj->methods, &obj->method_generated);
}
