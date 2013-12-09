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
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include <stdarg.h>
#include "object.h"
#include "gen_debug.h"
#include "info_output.h"
#include "expression.h"
#include "gen_common.h"

static int asprintf(char **sptr, const char *fmt, ...) {
	int retval, wanted;
	va_list argv;

	va_start(argv, fmt);
	wanted = vsnprintf(*sptr = NULL, 0, fmt, argv);
	if((wanted < 0) || ((*sptr = malloc(1 + wanted)) == NULL))
		return -1;
	va_end(argv);
	va_start(argv, fmt);
	retval = vsprintf(*sptr, fmt, argv);
	va_end(argv);
	return retval;
}

extern obj_ref_t *OBJ;
extern symtab_t root_table;
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

static arraydef_attr_t *_get_arraydef(object_attr_t *attr) {
	arraydef_attr_t *array_attr = NULL;

    if ((attr->common.obj_type == REGISTER_TYPE)) {
        array_attr = attr->reg.arraydef;
    }
    else if ((attr->common.obj_type == ATTRIBUTE_TYPE)) {
        array_attr = attr->attribute.arraydef;
    }
    else if ((attr->common.obj_type == GROUP_TYPE)) {
        array_attr = attr->group.arraydef;
    }
    else if ((attr->common.obj_type == PORT_TYPE)) {
        array_attr = attr->port.arraydef;
    }
    else if ((attr->common.obj_type == CONNECT_TYPE)) {
        array_attr = attr->connect.arraydef;
    }
	return array_attr;
}

static void process_object_names(object_t *obj) {
	/*
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
	*/
	char *name = NULL;
	int depth;
	arraydef_attr_t *array;
	object_attr_t *attr;
	const char *index;
	tree_t *node;

	if(!strcmp(obj->obj_type, "device")) {
		obj->qname = obj->name;		
		obj->dotname = "_dev";
		obj->is_array = 0;
		obj->depth = 0;
		return;
	}
	if(obj->is_array) {
		depth = obj->parent->depth + 1;
	} else {
		depth = obj->parent->depth; 
	}
	obj->depth = depth;
	if(obj->parent && !strcmp(obj->parent->obj_type, "device")) {
		obj->qname = obj->name;
		if(!obj->is_array) {
			obj->dotname = string_concat_with(obj->parent->dotname, obj->name, "->");
		} else {
			asprintf(&name, "%s->%s[_idx%d]", obj->parent->dotname, obj->name, obj->depth -1);
			obj->dotname = name;
		}
	} else {
		obj->qname = string_concat_with(obj->parent->qname, obj->name, "__");
		if(!obj->is_array) {
			obj->dotname = string_concat_with(obj->parent->dotname, obj->name, ".");
		} else {
			asprintf(&name, "%s.%s[_idx%d]", obj->parent->dotname, obj->name, obj->depth - 1);	
			obj->dotname = name;
		}
	}
	if(obj->is_array) {
		asprintf(&name, "_idx%d", obj->depth);
		node = create_string_node(name);
		obj->i_node = obj->d_node = node;
		/*rename object */
		array = _get_arraydef(obj->node->common.attr);
		if(array->fix_array) {
			index = "i";		
		} else {
			index = array->ident;
		}
		asprintf(&name, "%s[$%s]", obj->name, index);	
		obj->a_name = obj->name;
		obj->name = name;
	} else {
		node = create_string_node(obj->name);
		obj->i_node = obj->d_node = node;
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
		BE_DBG(OBJ_SYM, "symbol %s found in parent symtable\n", name);
	}
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
	//BE_DBG(OBJ_SYM, "symbol %s found in parent symtable\n", sym->name);
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
	} else if(!strcmp(name, "attribute")) {
		return Obj_Type_Attribute;
	} else if(!strcmp(name, "connect")) {
		return Obj_Type_Connect;
	} else if(!strcmp(name, "port")) {
		return Obj_Type_Port;
	} else if(!strcmp(name, "implement")) {
		return Obj_Type_Implement;
	} else if(!strcmp(name, "data")) {
		return Obj_Type_Data;
	} else if(!strcmp(name, "event")) {
		return Obj_Type_Event;
	} else if(!strcmp(name, "group")) {
		return Obj_Type_Group;
	}
	return Obj_Type_None;
}

extern void check_reg_table(void);
static void process_object_relationship(object_t *obj);
static void init_object(object_t *obj, object_t *parent, const char *name, const char *type, tree_t *node, symtab_t table) {	
	int i;
	device_t *dev = (device_t *)DEV;
	struct list_head *list;

	obj->name = name;
	obj->parent = parent;
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
	if(parent) {
		obj->symtab->parent = parent->symtab;
	} else {
		obj->symtab->parent = NULL;
	}
	if(parent) {
		BE_DBG(OBJ, "obj %s, symtab %p , sibling %p, parent table %p\n", obj->name, obj->symtab, obj->symtab->sibling, obj->symtab->parent);
	}
	struct list_head *p = &obj->childs;
	for(i = 0; i < 8; i++, p++){
		INIT_LIST_HEAD(p);
	}
	process_object_relationship(obj);
}

static void process_object_relationship(object_t *obj) {
	device_t *dev = (device_t *)DEV;
	struct list_head *list = NULL;
	object_type_t type = Obj_Type_None;
	object_t *parent = obj->parent;

	if(parent){
		obj->symtab->parent = parent->symtab;
		if(!strcmp(obj->obj_type, "connect")) {
			if (!strcmp(parent->obj_type, "device")) {
				list = &dev->connects;
			} else if (!strcmp(parent->obj_type, "port")) {
				list = &((dml_port_t*)parent)->connects;
			} else {
				error("this object is not allowed here\n");
			}
		} else if(!strcmp(obj->obj_type, "implement")) {
			if(!strcmp(parent->obj_type, "device")) {
				list = &dev->implements;
			} else if (!strcmp(parent->obj_type, "bank")){
				list = &((bank_t*)parent)->implements;
			} else if (!strcmp(parent->obj_type, "port")) {
				list = &((dml_port_t*)parent)->implements;
			} else {
				error("this object is not allowed here\n");
			}
		} else if(!strcmp(obj->obj_type, "attribute")) {
			if (!strcmp(parent->obj_type, "device")) {
				list = &dev->attributes;
			} else if (!strcmp(parent->obj_type, "group")) {
				list = &((group_t*)parent)->attributes;
			} else if (!strcmp(parent->obj_type, "port")) {
				list = &((dml_port_t*)parent)->attributes;
			} else {
				error("this object is not allowed here\n");
			}
		} else if(!strcmp(obj->obj_type, "port")) {
			list = &dev->ports; 
		} else if(!strcmp(obj->obj_type, "event")) {
				list = &parent->events;
		} else if(!strcmp(obj->obj_type, "data")) {
			list = &parent->data;
		} else if(!strcmp(obj->obj_type, "group")) {
			/*only banks, and groups*/
			if(!strcmp(parent->obj_type, "bank")) {
				bank_t *bank = (bank_t *)parent;
				list = &bank->groups;
			} else if(!strcmp(parent->obj_type, "group")) {
				group_t *gp = (group_t *)parent;
				list = &gp->groups;
			}
		} else {
			list = &parent->childs;
		}
		list_add_tail(&obj->entry, list);	
		symbol_insert(parent->symtab, obj->name, OBJECT_TYPE, obj);
	} 
}

static void process_default_template(object_t *obj);
static void create_none_object(object_t *obj, symbol_t sym) {
	printf("who are you %s something error happened\n", sym->name);
}

static void create_objs(object_t *obj, type_t type);
static object_t *create_dummy_field(object_t *obj) {
	static int index = 1;
	char *name;

	field_t *fld = (field_t *)gdml_zmalloc(sizeof(*fld));
	fld->is_dummy = 1;	
	asprintf(&name, "%s_%s%d", obj->name, "dummy", index);
	init_object(&fld->obj, obj, name, "field", NULL, NULL);
	index++;
	return &fld->obj;
}

static void create_field_object(object_t *obj, symbol_t sym){
	field_t *fld = gdml_zmalloc(sizeof(*fld));
	field_attr_t *field_attr = (field_attr_t *)sym->attr;
	init_object(&fld->obj, obj, sym->name, "field", field_attr->common.node, field_attr->common.table);
	create_objs(&fld->obj, DATA_TYPE);
}

static void create_register_object(object_t *obj, symbol_t sym){
	dml_register_t *reg = (dml_register_t *)gdml_zmalloc(sizeof(*reg));
	register_attr_t *reg_attr = (register_attr_t *)(sym->attr);
	symtab_t table = reg_attr->common.table;

	init_object(&reg->obj, obj, sym->name,"register",reg_attr->common.node, table);
	create_objs(&reg->obj, FIELD_TYPE);
	create_objs(&reg->obj, DATA_TYPE);
	create_objs(&reg->obj, EVENT_TYPE);
	//print_all_symbol(reg->obj.symtab);
}

static void create_iface_object(object_t *obj, symbol_t sym) {
	interface_t *iface = (interface_t *)gdml_zmalloc(sizeof(*iface));
	interface_attr_t *attr = (interface_attr_t *)sym->attr;
	symtab_t table = attr->common.table;
	init_object(&iface->obj, obj, sym->name, "interface", attr->common.node, table);
}

static void create_connect_object(object_t *obj, symbol_t sym) {
	connect_t *con = (connect_t *)gdml_zmalloc(sizeof(*con));
	connect_attr_t *attr = (connect_attr_t *)(sym->attr);
	symtab_t table = attr->common.table;
	init_object(&con->obj, obj, sym->name, "connect", attr->common.node, table);
	create_objs(&con->obj, INTERFACE_TYPE);
	create_objs(&con->obj, DATA_TYPE);	
	create_objs(&con->obj, EVENT_TYPE);
}

static void create_attribute_object(object_t *obj, symbol_t sym) {
	attribute_t *att = (attribute_t *)gdml_zmalloc(sizeof(*att));
	attribute_attr_t *attr = (attribute_attr_t *)(sym->attr);
	symtab_t table = attr->common.table;
	init_object(&att->obj, obj, sym->name, "attribute", attr->common.node, table);
	create_objs(&att->obj, DATA_TYPE);
	create_objs(&att->obj, EVENT_TYPE);
}

static void create_implement_object(object_t *obj, symbol_t sym){
	implement_t *impl = (implement_t *)gdml_zmalloc(sizeof (*impl));
	implement_attr_t *attr = (implement_attr_t *)sym->attr;
	symtab_t table = attr->common.table;
	init_object(&impl->obj, obj, sym->name, "implement", attr->common.node, table);	
}

static void create_data_object(object_t *obj, symbol_t sym) {
	data_t *data = (data_t *)gdml_zmalloc(sizeof (*data));
	init_object(&data->obj, obj, sym->name, "data", NULL, NULL );
}

static void create_port_object(object_t *obj, symbol_t sym) {
	if(!obj || strcmp(obj->obj_type, "device")) {
		BE_DBG(OBJ, "port should in top level\n");
	}
	dml_port_t *port = (dml_port_t *)gdml_zmalloc(sizeof(*port));
	port_attr_t *attr = (port_attr_t *)sym->attr;
	symtab_t table = attr->common.table;
	init_object(&port->obj, obj, sym->name, "port", attr->common.node, table);
	create_objs(&port->obj, IMPLEMENT_TYPE);
	create_objs(&port->obj, DATA_TYPE);
	create_objs(&port->obj, EVENT_TYPE);
	INIT_LIST_HEAD(&port->connects);
	INIT_LIST_HEAD(&port->implements);
	INIT_LIST_HEAD(&port->attributes);
	create_objs(&port->obj, CONNECT_TYPE);
	create_objs(&port->obj, IMPLEMENT_TYPE);
	create_objs(&port->obj, ATTRIBUTE_TYPE);
}

static void create_event_object(object_t *obj, symbol_t sym) {
	if(!obj || strcmp(obj->obj_type, "device")) {
		BE_DBG(OBJ, "event should in top level\n");
	}
	event_t *event = (event_t *)gdml_zmalloc(sizeof(*event));
	event_attr_t *attr = (event_attr_t *)sym->attr; 
	symtab_t table = attr->common.table;
	init_object(&event->obj, obj, sym->name, "event", attr->common.node, table);
	create_objs(&event->obj, DATA_TYPE);
}

static void create_group_object(object_t *obj, symbol_t sym) {
	group_t *gp = (group_t *)gdml_zmalloc(sizeof(*gp));
	group_attr_t *attr = (group_attr_t *)sym->attr;
	symtab_t table;

	table = attr->common.table;
	gp->obj.is_abstract = 1;
	INIT_LIST_HEAD(&gp->groups);
	INIT_LIST_HEAD(&gp->attributes);
	INIT_LIST_HEAD(&gp->registers);
	init_object(&gp->obj, obj, sym->name, "group", attr->common.node, table);
	create_objs(&gp->obj, REGISTER_TYPE);
	create_objs(&gp->obj, GROUP_TYPE);
	create_objs(&gp->obj, DATA_TYPE);
	create_objs(&gp->obj, EVENT_TYPE);
	create_objs(&gp->obj, ATTRIBUTE_TYPE);
}

static void create_bank_object(object_t *obj, symbol_t sym){
	bank_t *bank = gdml_zmalloc(sizeof(*bank));
	INIT_LIST_HEAD(&bank->groups);
	bank_attr_t *b = (bank_attr_t *)(sym->attr);
	symtab_t table = b->common.table;
	init_object(&bank->obj, obj, sym->name,"bank",b->common.node, table);		
	create_objs(&bank->obj, REGISTER_TYPE);
	create_objs(&bank->obj, DATA_TYPE);
	create_objs(&bank->obj, EVENT_TYPE);
	INIT_LIST_HEAD(&bank->groups);
	INIT_LIST_HEAD(&bank->implements);
	INIT_LIST_HEAD(&bank->attributes);
	create_objs(&bank->obj, GROUP_TYPE);
	create_objs(&bank->obj, IMPLEMENT_TYPE);
	create_objs(&bank->obj, ATTRIBUTE_TYPE);
	printf("bank created %s\n", sym->name);
	//print_all_symbol(table);
}


static void (*create_func[])(object_t *obj, symbol_t sym) = {
	[Obj_Type_None] = create_none_object,
	[Obj_Type_Device] = create_none_object,
	[Obj_Type_Bank] = create_bank_object,
	[Obj_Type_Register] = create_register_object,
	[Obj_Type_Field]  = create_field_object,
	[Obj_Type_Attribute] = create_attribute_object,
	[Obj_Type_Connect]  = create_connect_object,
	[Obj_Type_Port]    = create_port_object,
	[Obj_Type_Implement] = create_implement_object,
	[Obj_Type_Interface] = create_iface_object,
	[Obj_Type_Data]     = create_data_object,
	[Obj_Type_Event]    = create_event_object,
	[Obj_Type_Group]    = create_group_object
};

static object_type_t type2obj_type(type_t type) {
	object_type_t ret;

	switch(type) {
		case DEVICE_TYPE:
			ret = Obj_Type_Device;
			break;
		case BANK_TYPE:
			ret = Obj_Type_Bank;
			break;
		case REGISTER_TYPE:
			ret = Obj_Type_Register;
			break;
		case FIELD_TYPE:
			ret = Obj_Type_Field;
			break;
		case CONNECT_TYPE:
			ret = Obj_Type_Connect;
			break;
		case PORT_TYPE:
			ret = Obj_Type_Port;
			break;
		case IMPLEMENT_TYPE:
			ret = Obj_Type_Implement;
			break;
		case INTERFACE_TYPE:
			ret = Obj_Type_Interface;
			break;
		case ATTRIBUTE_TYPE:
			ret = Obj_Type_Attribute;
			break;
		case DATA_TYPE:
			ret = Obj_Type_Data;
			break;
		case EVENT_TYPE:
			ret = Obj_Type_Event;
			break;
		default:
			ret = Obj_Type_None;
	}
	return ret;
}

static void create_objs(object_t *obj, type_t type) {
	symtab_t table = obj->symtab->sibling;
    symbol_list_t *list = symbol_list_find_type(table, type);
    symbol_list_t *head = list;
	object_type_t obj_type;

	obj_type = type2obj_type(type);
    while(list) {
		create_func[obj_type](obj, list->sym);
        /*restore OBJ to device*/
        list = list->next;
    }
    symbol_list_free(head);
}

object_t *create_device_object(symbol_t sym){
	int i = 0;
	struct list_head *p;
	object_t *obj;

	device_t *dev =  (device_t *)gdml_zmalloc(sizeof(*dev));
	if(DEV)
		BE_DBG(OBJ, "device exists\n");
	device_attr_t *dev_attr = (device_attr_t *)sym->attr;
	BE_DBG(OBJ, "device %s found\n",sym->name);
	init_object(&dev->obj, NULL, sym->name, "device", dev_attr->common.node, root_table);
	p = &dev->constants;
	for(i = 0; i < 5; i++, p++) {
		INIT_LIST_HEAD(p);
	}
	obj = &dev->obj;
	DEV = obj;
	create_objs(obj, BANK_TYPE);
	create_objs(obj,CONNECT_TYPE );
	create_objs(obj, ATTRIBUTE_TYPE);
	create_objs(obj, IMPLEMENT_TYPE);
	create_objs(obj, PORT_TYPE);
	create_objs(obj, EVENT_TYPE);
	create_objs(obj, DATA_TYPE);
	return obj;
}

device_t *create_device_tree(tree_t  *root){
	object_t *obj;

	//print_all_symbol(root_table);
	symbol_list_t *dev_list = symbol_list_find_type(root_table, DEVICE_TYPE);
	if(!dev_list || dev_list->next) {
		printf("device not correct\n");
		exit(-1);
	}
	symbol_t sym = dev_list->sym;
	obj = create_device_object(sym);
	symbol_list_free(dev_list);
	return (device_t *)obj;
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
		list_add_tail(&tmp->entry, &obj->templates);
		//list_add_tail(&obj->templates, &tmp->entry);
	} else {
		BE_DBG(OBJ, "template %s not found \n", name);
	}
}

static void field_realize(object_t *obj) {
	field_t *fld = (field_t *)obj;
	tree_t *bitrange;
	tree_t *expr;
	tree_t *templates = (obj->node) ? obj->node->field.templates : NULL;

	add_object_templates(obj, templates);
	if(fld->is_dummy) {
		process_object_names(obj);
		process_object_templates(obj);	
		return;
	} 

	/* calculate the expression about parameters
	 * that defined in the field block */
	//parse_parameter(obj->symtab->sibling);
	//process_object_templates(obj);
	/* parse default parameters about registers
	 * such as: size, offset, array*/
	parse_field_attr(obj->node, obj->symtab->parent);
	/* parse the elements that in filed table*/
	//parse_field(obj->node, obj->symtab->sibling);
	parse_field(obj->node, obj->symtab);
	bitrange = obj->node->field.bitrange;	
	if (bitrange) {
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
	}
	process_object_names(obj);
	process_object_templates(obj);
	return;
}

static int get_binopnode_constant(tree_t *t, type_t op, int *result) {
	tree_t *left, *right;
	int ret = -1;

	left = t->binary.left;
	right = t->binary.right;
	if(t->common.type != BINARY_TYPE || t->binary.type != op) {
		*result = -1;
		return ret;
	}
	if(left->common.type == INTEGER_TYPE) {
		*result = left->int_cst.value;
		ret = 0;
	} else if(right->common.type == INTEGER_TYPE) {
		*result = right->int_cst.value;
		ret = 1;
	}
	return ret;
}

static int get_reg_offset(paramspec_t *t, int *interval) {
	tree_t *node;
	int offset = 0;
	tree_t *tmp;
	int ret;

	if((t->value->type == PARAM_TYPE_INT) && t->value->is_const) {
		offset = t->value->u.integer;
		*interval = 0;
		return offset;
	} else if(t->value->type == PARAM_TYPE_UNDEF) {
		return -1;
	} else {
		node = t->expr_node;
		ret = get_binopnode_constant(node, ADD_TYPE, &offset);
		if(ret < 0) {
			/*wrong format base + $i * register_size */
			BE_DBG(GENERAL, "wrong register offset format, node type %d, expected format %d(+)\n", node->common.type, ADD_TYPE);
			exit(-1);
			
		} else if(!ret) {
			tmp = node->binary.right;
		} else {
			tmp = node->binary.left;
		}
		ret = get_binopnode_constant(tmp, MUL_TYPE, interval);
		if(ret < 0) {
			BE_DBG(GENERAL, "wrong register offset format, node type %d, expected format %d(*)\n", node->common.type, MUL_TYPE);
		}
	}
	return offset;
}

static void event_realize(object_t *obj);
static void register_realize(object_t *obj) {
	dml_register_t *reg  = (dml_register_t *)obj;
	bank_t *bank = (bank_t *)obj->parent; 
	register_attr_t *reg_attr;
	int i = 0;
	int register_size = bank->register_size;
	struct list_head *p;
	object_t *tmp;
	arraydef_attr_t *arraydef = NULL;
	symbol_t sym;
	parameter_attr_t *parameter_attr;
	const char *index;
	int depth;

	list_for_each(p, &obj->childs) {
		i++;
	}
	if(i == 0) {
		/*default one dummy field*/
		i = 1;
	}
	reg->field_count = i;
	reg->fields = (object_t **)gdml_zmalloc(sizeof(obj) * reg->field_count);
	i = 0;
	list_for_each(p, &obj->childs) {
		tmp = list_entry(p, object_t, entry);
		reg->fields[i] = tmp;
		i++;
	}
	if(i == 0) {
		/*no field,create a dummy one*/
		tmp = create_dummy_field(obj); 
		reg->fields[0] = tmp;
	}

	add_object_templates(obj, obj->node->reg.templates);
	/* parse the attribute about registers
	 * such as: size, offset, array*/
	parse_register_attr(reg->obj.node, obj->symtab->parent);
	/* parse the elements that in the register table */
	parse_register(obj->node, obj->symtab->sibling);

	reg_attr = reg->obj.node->common.attr;
	reg->is_array = reg_attr->is_array;
	obj->is_array = reg->is_array;
	reg->size = reg_attr->size;
	/*take default value, if not specified*/
	if(!reg->size || reg->size == -1) {
		reg->size = register_size;
	}
	process_object_names(obj);
	if(reg->is_array) {
		arraydef = reg_attr->arraydef;
		if(!arraydef->fix_array) {
			reg->array_size = arraydef->high - arraydef->low + 1;
		} else {
			reg->array_size = arraydef->fix_array;
			/*default index*/
		}
		obj->array_size = reg->array_size;
	}
	reg->offset = reg_attr->offset;
	reg->is_undefined = 0;
	if(reg->offset == -1) {
		/*search for parameter offset */
		sym = symbol_find(obj->symtab, "offset", PARAMETER_TYPE);
		if(!sym) {
			reg->is_undefined = 1;
		} else {
			parameter_attr  = (parameter_attr_t *)sym->attr;		
			reg->offset = get_reg_offset(parameter_attr->param_spec, &reg->interval);
			if(reg->offset == -1) {
				reg->is_undefined = 1;
			}
			if(reg->interval == -1) {
				BE_DBG(GENERAL, "register array interval not right\n");
				exit(-1);
			}
			BE_DBG(GENERAL, "reg offset 0x%x\n", reg->offset);
		}	
	}
	list_for_each(p, &obj->childs) {
		tmp = list_entry(p, object_t, entry);
		field_realize(tmp);
	}
	list_for_each(p, &obj->events) {
		tmp = list_entry(p, object_t, entry);
		event_realize(tmp);
	}
	process_object_templates(obj);
}

static void bank_calculate_register_offset(object_t *obj) {
	bank_t *bank = (bank_t *)obj;
	dml_register_t *reg;
	register_attr_t *reg_attr;
	int i = 0;
	int offset = 0;
	int register_size = bank->register_size;
	int size;
	int last = 0;
	
	
	for(i = 0; i < bank->reg_count; i++) {
		reg = (dml_register_t *)bank->regs[i];
		/*skip undefined registers, only used as variables, not intended for access */
		if(reg->is_undefined) {
			continue;
		}
		/*if user not specify an offset, we stuff it based on its order, FIXME maybe wrong*/
		if(reg->offset == -1) {
			reg->offset = offset;
		}
		/*check register offset cross bounry cases, not allowed*/
		if(reg->offset < offset) {
			BE_DBG(GENERAL, "offset cross boundry! reg->offset = %d, offset = %d, name = %s\n", reg->offset, offset, reg->obj.name);
			//exit(-1);
		} else if(reg->offset > offset) {
			offset = reg->offset;
		}
		if(reg->is_array) {
			size = reg->array_size * reg->size;
		} else {
			size = reg->size;
		}
		offset += size;		
	}	
	/*take the register total size as bank size in a conservative way. Maybe some alignment should make */
	bank->size = offset;
}

static void attribute_realize(object_t *obj);
static void implement_realize(object_t* obj);
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
	param_value_t* value;

	list_for_each(p, &obj->childs) {
		i++;
	}
	tmp = (object_t *)gdml_zmalloc(sizeof(tmp) * i);
	bank->regs = (object_t **)tmp;
	bank->reg_count = i;
	i = 0;
	list_for_each(p, &obj->childs) {
		tmp = list_entry(p, object_t, entry);
		bank->regs[i] = tmp;
		i++;
	}

	add_object_templates(obj, obj->node->bank.templates);
	/* parse the symbols, parameters and check expressions
	 * that in the bank table */
	parse_bank(obj->node, obj->symtab->sibling);
	
	BE_DBG(OBJ, "object name %s\n", obj->name);
	sym = symbol_find(obj->symtab, "register_size", PARAMETER_TYPE);
	if(sym) {
		param = (parameter_attr_t *)sym->attr;
		spec = param->param_spec;
		value = spec->value;
		if(!value->is_const) {
			reg_size = 4;
			//PERROR("the register_size require a constant value",
			//		spec->expr_node->common.location);
		} else {
			reg_size = value->u.integer;
		}
	} else {
		reg_size = 4;
	}
	process_object_names(obj);

	bank->register_size = reg_size;	
	list_for_each(p, &obj->childs) {
		tmp = list_entry(p, object_t, entry);
		register_realize(tmp);
	}
	list_for_each(p, &bank->attributes) {
		tmp = list_entry(p, object_t, entry);
		attribute_realize(tmp);
	}
	list_for_each(p, &bank->implements) {
		tmp = list_entry(p, object_t, entry);
		implement_realize(tmp);
	}
	list_for_each(p, &obj->events) {
		tmp = list_entry(p, object_t, entry);
		event_realize(tmp);
	}
	/*sort the register in user specifid order*/
	bank_calculate_register_offset(obj);
	process_object_templates(obj);
}

static void connect_realize(object_t *obj) {
	arraydef_attr_t *array_def;
	tree_t *node;
	connect_attr_t *attr;

	add_object_templates(obj, obj->node->connect.templates);
	/* parse the connect arraydef expression */
	parse_connect_attr(obj->node, obj->symtab->parent);
	/* parse elements and calculate expressions that
	 * in  connect table*/
	parse_connect(obj->node, obj->symtab->sibling);

	node = obj->node;
	attr = node->common.attr;
	obj->is_array = attr->is_array;
	process_object_names(obj);
	if(obj->is_array) {
		array_def = attr->arraydef;
		obj->array_size = array_def->fix_array;
		if(!obj->array_size) {
			obj->array_size = array_def->high - array_def->low + 1;
		}
	}
	struct list_head *p;
	object_t *tmp;
	list_for_each(p, &obj->events) {
		tmp = list_entry(p, object_t, entry);
		event_realize(tmp);
	}
}

static const char* get_allocate_type(const char *alloc_type, attribute_t* attr) {
	const char* type = NULL;

	if(!strcmp(alloc_type, "\"double\"")) {
		type = "double";
		attr->alloc = DOUBLE_T;
	} else if(!strcmp(alloc_type, "\"string\"")) {
		type = "string";
		attr->alloc = STRING_T;
	} else if(!strcmp(alloc_type, "\"uint8\"")) {
		type = "uint8";
		attr->alloc = INT_T;
	} else if(!strcmp(alloc_type, "\"uint16\"")) {
		type = "uint16";
		attr->alloc = INT_T;
	} else if(!strcmp(alloc_type, "\"uint32\"")) {
		type = "uint32";
		attr->alloc = INT_T;
	} else if(!strcmp(alloc_type, "\"uint64\"")) {
		type = "uint64";
		attr->alloc = LONG_T;
	} else if(!strcmp(alloc_type, "\"int8\"")) {
		type = "int8";
		attr->alloc = INT_T;
	} else if(!strcmp(alloc_type, "\"int16\"")) {
		type = "int16";
		attr->alloc = INT_T;
	} else if(!strcmp(alloc_type, "\"int32\"")) {
		type = "int32";
		attr->alloc = INT_T;
	} else if(!strcmp(alloc_type, "\"int64\"")) {
		type = "int64";
		attr->alloc = LONG_T;
	} else if (!strcmp(alloc_type, "\"bool\"")) {
		type = "bool";
		attr->alloc = INT_T;
	}else {
		fprintf(stderr, "The attribute alloc type not wright '%s'\n", alloc_type);
		exit(-1);
	}
	return type;
}

static const char* get_configuration(const char* str) {
	if (str == NULL)
		return NULL;
	const char* config = NULL;
	if (!strcmp(str, "\"required\"")) {
		config = "required";
	} else if (!strcmp(str, "\"optional\"")) {
		config = "optional";
	} else if (!strcmp(str, "\"pseudo\"")) {
		config = "pseudo";
	} else if (!strcmp(str, "\"none\"")) {
		config = "none";
	} else {
		fprintf(stderr, "The attribute configuration value no right '%s'\n", str);
		exit(-1);
	}

	return config;
}

static const char* get_type(const char* str, attribute_t* attr) {
	assert(str != NULL);
	const char* type = NULL;
	if (!strcmp(str, "\"i\"")) {
		type = "i";
		attr->ty = INT_T;
	} else if (!strcmp(str, "\"f\"")) {
		type = "f";
		attr->ty = FLOAT_T;
	} else if (!strcmp(str, "\"s\"")) {
		type = "s";
		attr->ty = STRING_T;
	} else if (!strcmp(str, "\"b\"")) {
		type = "b";
		attr->ty = BOOL_T;
	} else if (!strcmp(str, "\"o\"")) {
		type = "o";
		attr->ty = INT_T;
	} else if (!strcmp(str, "\"a\"")) {
		type = "a";
		attr->ty = FLOAT_T;
	} else {
		type = "i";
		attr->ty = INT_T;
	}
	return type;
}

#define get_param_spec(attr, spec) \
	attr = (parameter_attr_t *)sym->attr; \
	spec = attr->param_spec;
static void attribute_realize(object_t *obj) {
	tree_t *node;
	symbol_t sym;
	const char* alloc_type;
	parameter_attr_t* attr;
	paramspec_t* spec;
	attribute_t *attr_obj = (attribute_t *)obj;

	add_object_templates(obj, obj->node->attribute.templates);
	/* parse the arraydef about attribute */
	parse_attribute_attr(obj->node, obj->symtab->parent);
	attribute_attr_t* attribute_attr = obj->node->common.attr;
	attribute_attr->attr_obj = attr_obj;
	/* parse the elements that in the attribute table*/
	parse_attribute(obj->node, obj->symtab->sibling);

	sym = defined_symbol(obj->symtab->sibling, "allocate_type", PARAMETER_TYPE);
	if (sym) {
		get_param_spec(attr, spec);
		alloc_type = get_allocate_type(spec->value->u.string, attr_obj);
		attr_obj->alloc_type = alloc_type;
	}
	sym = defined_symbol(obj->symtab->sibling, "configuration", PARAMETER_TYPE);
	if (sym) {
		get_param_spec(attr, spec);
		attr_obj->configuration = get_configuration(spec->value->u.string);
	}
	sym = defined_symbol(obj->symtab->sibling, "persistent", PARAMETER_TYPE);
	sym = defined_symbol(obj->symtab->sibling, "internal", PARAMETER_TYPE);
	sym = defined_symbol(obj->symtab->sibling, "attr_type", PARAMETER_TYPE);
	if (sym) {
		get_param_spec(attr, spec);
		if (spec->value->u.string != NULL) {
			attr_obj->type = get_type(spec->value->u.string, attr_obj);
		} else {
			attr_obj->type = strdup("i");
			attr_obj->ty = INT_T;
		}
	}
	sym = defined_symbol(obj->symtab, "type", PARAMETER_TYPE);
	if (sym) {
		get_param_spec(attr, spec);
		if (spec->value->u.string != NULL) {
			attr_obj->type = get_type(spec->value->u.string, attr_obj);
		} else {
			attr_obj->type = strdup("i");
			attr_obj->ty = INT_T;
		}
	}
	struct list_head *p;
	object_t *tmp;
	list_for_each(p, &obj->events) {
		tmp = list_entry(p, object_t, entry);
		event_realize(tmp);
	}
	process_object_names(obj);
	return;
}

static void implement_realize(object_t* obj) {
	parse_implement(obj->node, obj->symtab->sibling);
	process_object_names(obj);
	return;
}

static void port_realize(object_t *obj) {
	struct list_head *p;
	object_t *tmp;
	int i = 0;
	dml_port_t *port;

	list_for_each(p, &obj->childs) {
		i++;
	}
	port = (dml_port_t *)obj;
	port->num = i;
	port->impls = (object_t **)gdml_zmalloc(sizeof(*port->impls) * i);
	process_object_names(obj);
	i = 0;
	list_for_each(p, &obj->childs) {
		tmp = list_entry(p, object_t, entry);
		implement_realize(tmp);
		port->impls[i] = tmp;
		i++;
	}
	list_for_each(p, &obj->events) {
		tmp = list_entry(p, object_t, entry);
		event_realize(tmp);
	}
	list_for_each(p, &port->attributes) {
		tmp = list_entry(p, object_t, entry);
		attribute_realize(tmp);
	}
	list_for_each(p, &port->connects) {
		tmp = list_entry(p, object_t, entry);
		connect_realize(tmp);
	}
}

static void event_realize(object_t *obj) {
	parse_event(obj->node, obj->symtab->sibling);
	add_object_templates(obj, NULL);
	process_object_names(obj);
}

void device_realize(device_t *dev) {
	struct list_head *p;
	object_t *tmp;
	int i = 0;

	list_for_each(p, &dev->obj.childs) {
		i++;
	}
	dev->banks = (object_t **)gdml_zmalloc(sizeof(tmp) * i);
	i = 0;
	list_for_each(p, &dev->obj.childs) {
		tmp = list_entry(p, object_t, entry);
		dev->banks[i] = tmp;
		i++;
	}
	dev->bank_count = i;
	dev->obj.depth = 0;

	/* add default templates "template device" into device,
	 * and insert symbols into table of templates */
	add_object_templates(&dev->obj, NULL);
	/* In parsing, we only insert the object and method symbol
	 * into table, and create the table, but the constant, parameters
	 * and expression are not handled, we should insert the constant,
	 * and parameters into table, and calculate expressions,
	 * as something can be detemined after the object instantiated*/
	parse_device(dev->obj.node, dev->obj.symtab->sibling);

	process_object_names(&dev->obj);
	list_for_each(p, &dev->obj.childs) {
		tmp = list_entry(p, object_t, entry);
		bank_realize(tmp);
	}
	list_for_each(p, &dev->connects) {
		tmp = list_entry(p, object_t, entry);
		connect_realize(tmp);
	}
	list_for_each(p, &dev->attributes) {
		tmp = list_entry(p, object_t, entry);
		attribute_realize(tmp);
	}
	list_for_each(p, &dev->implements) {
		tmp = list_entry(p, object_t, entry);
		implement_realize(tmp);
	}
	list_for_each(p, &dev->ports) {
		tmp = list_entry(p, object_t, entry);
		port_realize(tmp);
	}
	list_for_each(p, &dev->obj.events) {
		tmp = list_entry(p, object_t, entry);
		event_realize(tmp);
	}
	process_object_templates(&dev->obj);
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

static void print_rely_temp(struct template_list* list) {
	struct template_list* tmp = list;
	int i = 0;
	while (tmp) {
		printf("temp_list[%d]: %s, pre: %s\n", i++, tmp->name, tmp->src_name);
		tmp = tmp->next;
	}
}

int add_temp_in_list(struct template_list* list, struct template_list* new) {
	assert(list != NULL);
	assert(new != NULL);
	struct template_list* tmp = list;
	struct template_list* pre = list;

	/* call this function only for debugging to
	 * print the templates that in list */
	//print_rely_temp(list);
	while (tmp) {
		if (strcmp(tmp->name, new->name) == 0) {
			fprintf(stderr, "error: template '%s' is confilct in template '%s' and '%s'\n",
					new->name, tmp->src_name, new->src_name);
			/* TODO: handle the error */
			exit(-1);
			return 1;
		}
		pre = tmp;
		tmp = tmp->next;
	}
	pre->next = new;

	return 0;
}

extern struct template_def *create_template_def(const char* name, struct template_list* rely_list);
static void add_rely_templates(struct template_def* def, struct template_table* templates, struct template_list* rely_list) {
	assert(def != NULL);
	if (templates == NULL) {
		return;
	}

	int i = 0;
	struct list_head* list = &def->templates;
	struct template_table* tmp = templates;
	struct template_name* new_temp = NULL;
	struct template_list* new = NULL;
	while (tmp) {
		new = (struct template_list*)gdml_zmalloc(sizeof(struct template_list));
		new->src_name = strdup(def->name);
		new->name = strdup(tmp->template_name);
		if (add_temp_in_list(rely_list, new)) {
			free((void*)(new->src_name));
			free((void*)(new->name));
			free((void*)new);
		}
		new_temp = (struct template_name*)gdml_zmalloc(sizeof(struct template_name));
		new_temp->name = strdup(tmp->template_name);
		INIT_LIST_HEAD(&new_temp->entry);
		list_add_tail(&new_temp->entry, list);
		new_temp->def = create_template_def(new_temp->name, rely_list);
		tmp = tmp->next;
	}

	return;
}

struct template_def *create_template_def(const char* name, struct template_list* list){
	assert(name != NULL);

	symbol_t symbol = symbol_find(root_table, name, TEMPLATE_TYPE);
	template_attr_t* attr = symbol->attr;
	tree_t* node = attr->common.node;
	symtab_t temp_table = attr->table;
	struct template_table* templates = temp_table->template_table;
	struct template_def* def = (struct template_def*)gdml_zmalloc(sizeof(struct template_def));
	def->name = strdup(name);

	int i = 0;
	struct list_head *head = &def->entry;
	for (i = 0; i < 3; i++, head++) {
		INIT_LIST_HEAD(head);
	}
	/* print_rely_temp is only for debugging, it will
	 * print the templates that relied */
	//print_rely_temp(list);
	add_rely_templates(def, templates, list);

	/* one template can be contained by many kind of object,
	 * but it can be parsed and calculated only once */
	if (temp_table->is_parsed == 1)
		return def;

	/* in template, we can only insert the parameters and method into
	template talbe, and check them until use it when object use them */
	obj_spec_t* tmp = node->temp.spec;
	tree_t* block = NULL;
	tree_t* statement = NULL;
	while (tmp) {
		block = tmp->node->spec.block;
		if (block != NULL)
			statement = block->block.statement;
		if ((block == NULL) || (statement == NULL))
			return def;
		my_DBG("-----------------start parsing template %s-------------\n", name);
		while (statement) {
			if (statement->common.type == PARAMETER_TYPE ||
				statement->common.type == METHOD_TYPE ||
				statement->common.type == CDECL_TYPE) {
				statement->common.parse(statement, attr->table);
			}
			statement = statement->common.sibling;
		}
		my_DBG("-----------------finish parsing template %s-------------\n", name);
		tmp = tmp->next;
	}
	temp_table->is_parsed = 1;

	return def;
}

/*
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
*/

static void obj_templates_list(object_t* obj, const char* name) {
	assert(obj != NULL);
	assert(name != NULL);
	struct template_table* templates = NULL;
	symtab_t table = obj->symtab->sibling;

	create_template_name(obj, obj->obj_type);
	if (table) {
		templates = table->template_table;
		while (templates) {
			create_template_name(obj, templates->template_name);
			templates = templates->next;
		}
	}

	return;
}

static void add_default_template(object_t *obj){
	symtab_t table;	

	create_template_name(obj, obj->obj_type);
	table = obj->symtab->sibling;
	if(table) {
		BE_DBG(OBJ, "add default template %s\n", obj->obj_type);
		check_undef_template(table);
		add_template_to_table(table, obj->obj_type, 0);
	}
	obj_templates_list(obj, obj->obj_type);
}

static void process_device_template(object_t *obj){
	symtab_t table = obj->symtab;
	device_t *dev = (device_t *)obj;
	symbol_t sym;
	int i;
	param_value_t tmp;

	param_value_t *val = gdml_zmalloc(sizeof(*val));	
	val->type = PARAM_TYPE_LIST;
	val->u.list.size = dev->bank_count;
	val->u.list.vector = gdml_zmalloc(sizeof(*val) * val->u.list.size);
	for(i = 0; i < dev->bank_count; i++) {
		tmp.type = PARAM_TYPE_REF;
		tmp.u.ref = dev->banks[i];
		val->u.list.vector[i] = tmp;
	}
	symbol_insert(table, "banks", PARAMETER_TYPE, val);
	val = (param_value_t *)gdml_zmalloc(sizeof(*val));	
	val->type = PARAM_TYPE_REF;
	val->u.ref = obj;
	symbol_insert(table, "obj", PARAMETER_TYPE, val);
	val = (param_value_t *)gdml_zmalloc(sizeof(*val));	
	val->type = PARAM_TYPE_REF;
	val->u.ref = obj;
	symbol_insert(table, "logobj", PARAMETER_TYPE, val);
	symbol_insert(table, "dev", OBJECT_TYPE, obj);
}

static void process_bank_template(object_t *obj){
	symtab_t table = obj->symtab;
	bank_t *bank = (bank_t *)obj;
	int i;
	param_value_t tmp;
	dml_register_t *reg;
	int undefined = 0;
	int defined = 0;

	for(i = 0; i < bank->reg_count; i++) {
		reg = (dml_register_t *)bank->regs[i];
		if(!reg->is_undefined) {
			defined++;
		} else {
			undefined++;
		}
	}
	param_value_t *val = gdml_zmalloc(sizeof(*val));
	val->type = PARAM_TYPE_LIST;
	val->u.list.size = defined;
	val->u.list.vector = gdml_zmalloc(sizeof(*val) * bank->reg_count);
	for(i = 0; i < defined; i++) {
		reg = (dml_register_t *)bank->regs[i];
		if(!reg->is_undefined) {
			tmp.type = PARAM_TYPE_REF;
			tmp.u.ref = bank->regs[i];
			val->u.list.vector[i] = tmp;
		}
	}
	symbol_insert(table, "mapped_registers", PARAMETER_TYPE, val);
	val = gdml_zmalloc(sizeof(*val));
	val->type = PARAM_TYPE_LIST;
	val->u.list.size = undefined;
	if(undefined > 0) {
		for(i = 0; i < undefined; i++) {
			reg = (dml_register_t *)bank->regs[i];
			if(reg->is_undefined) {
				tmp.type = PARAM_TYPE_REF;
				tmp.u.ref = bank->regs[i];
				val->u.list.vector[i] = tmp;
			}
		}		
	}

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
	val->type = PARAM_TYPE_LIST;
	val->u.list.size = reg->field_count;
	val->u.list.vector = gdml_zmalloc(sizeof(*val) * reg->field_count);
	for(i = 0; i < reg->field_count; i++) {
		tmp.type = PARAM_TYPE_REF;
		tmp.u.ref = reg->fields[i];
		val->u.list.vector[i] = tmp;
	}
	symbol_insert(table, "fields", PARAMETER_TYPE, val);
	val = gdml_zmalloc(sizeof(*val));
	val->type = PARAM_TYPE_STRING;
	val->u.string = strdup(obj->name);
	symbol_insert(table, "_regname", PARAMETER_TYPE, val);
	val = gdml_zmalloc(sizeof(*val));
	val->type = PARAM_TYPE_INT;
	val->u.integer = 0;
	symbol_insert(table, "hard_reset_value", PARAMETER_TYPE, val);

	val = gdml_zmalloc(sizeof(*val));
	val->type = PARAM_TYPE_INT;
	bank_t *bank = (bank_t *)reg->obj.parent;
	val->u.integer = bank->register_size * 8;
	symbol_insert(table, "bitsize", PARAMETER_TYPE, val);

	val = gdml_zmalloc(sizeof(*val));
	val->type = PARAM_TYPE_BOOL;
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
	val->type = PARAM_TYPE_BOOL;
	if(field->is_dummy) {
		val->u.boolean = 0;
	} else {
		val->u.boolean = 1;
	}
	symbol_insert(table, "explicit", PARAMETER_TYPE, val);
	val = gdml_zmalloc(sizeof(*val));
	val->type = PARAM_TYPE_INT;
	val->u.integer = field->low;
	symbol_insert(table, "lsb", PARAMETER_TYPE, val);
	val = gdml_zmalloc(sizeof(*val));
	val->type = PARAM_TYPE_INT;
	val->u.integer = field->high;
	symbol_insert(table, "msb", PARAMETER_TYPE, val);

	val = gdml_zmalloc(sizeof(*val));
	val->type = PARAM_TYPE_INT;
	val->u.integer = field->len;
	symbol_insert(table, "bitsize", PARAMETER_TYPE, val);
	val = gdml_zmalloc(sizeof(*val));
	val->type = PARAM_TYPE_INT;
	val->u.integer = 0;
	symbol_insert(table, "hard_reset_value", PARAMETER_TYPE, val);
}

static void free_rely_temp_list(struct template_list* list) {
	struct template_list* tmp = list;
	struct template_list* old = list;
	while (tmp) {
		tmp = tmp->next;
		//printf("Free: src_name: %s, name: %s\n", old->src_name, old->name);
		free((void*)(old->src_name));
		free((void*)(old->name));
		old = tmp;
	}

	return;
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

	//BE_DBG(TEMPLATE, "\ntemplate name %s, obj name %s--------------\n\n", name->name, obj->name);
	struct template_list* rely_temp_list = (struct template_list*)gdml_zmalloc(sizeof(struct template_list));
	rely_temp_list->src_name = strdup(obj->name);
	rely_temp_list->name = strdup(name->name);
	name->def = create_template_def(name->name, rely_temp_list);
	free_rely_temp_list(rely_temp_list);
}

static void process_basic_template(object_t *obj) {
	/*hardcode object auto parameter in object symtab*/
	symtab_t table = obj->symtab;
	param_value_t *val = gdml_zmalloc(sizeof(*val));
	val->type = PARAM_TYPE_REF;
	val->u.ref = obj->parent;
	symbol_insert(table, "parent", PARAMETER_TYPE, val);

	val = gdml_zmalloc(sizeof(*val));
	val->type = PARAM_TYPE_STRING;
	val->u.string = strdup(obj->name);
	symbol_insert(table, "name", PARAMETER_TYPE, val);

	val = gdml_zmalloc(sizeof(*val));
	val->type = PARAM_TYPE_STRING;
	val->u.string = strdup(obj->qname);
	symbol_insert(table, "qname", PARAMETER_TYPE, val);

	val = gdml_zmalloc(sizeof(*val));
	val->type = PARAM_TYPE_REF;
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
	[Obj_Type_Connect] = object_none_temp_fn,
};

static void process_default_template(object_t *obj) {
	process_basic_template(obj);
	specific_temp_fn[obj->encoding](obj);
	BE_DBG(TEMPLATE, "start ........\nobject Name %s symbols\n", obj->name);
	//print_all_symbol(obj->symtab);
	BE_DBG(TEMPLATE, "end .......");
}

void process_object_templates(object_t *obj){
	struct template_name *name;
	struct list_head *p;

	process_default_template(obj);
}

static void parse_rely_templates(object_t* obj) {
	assert(obj != NULL);
	struct template_name *name;
	struct list_head *p;
	list_for_each(p, &obj->templates) {
		name = list_entry(p,struct template_name,entry);
		if(name) {
			process_object_template(obj, name);
		}
	}

	return;
}

static void add_object_templates(object_t *obj, tree_t *t){
	tree_t *it = t;
	struct tree_ident *ident;
	struct template_name *temp;
	
	add_default_template(obj);
	parse_rely_templates(obj);
	//process_object_templates(obj);
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

object_t *find_index_obj(object_t *obj, int index) {
	object_t *ret = obj;

	while(ret) {
		if((ret->is_array) && (ret->depth == index)) {
			return ret;
		} else {
			ret = obj->parent;
		}
	}
	return NULL;
}

object_t* get_device_obj() {
	return DEV;
}
