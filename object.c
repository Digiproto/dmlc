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
#include "table_utility.h"
#include "node.h"
#include "decl.h"
static int g_register_size;
/**
 * @brief asprintf : combine the string into one based on the format
 *
 * @param sptr : the pointer for the last value
 * @param fmt : thr format of args
 * @param ... : ellipse parameters
 *
 * @return : the number of characters printed
 */
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

/**
 * @brief string_concat_with : combine two strings with connector
 *
 * @param s1 : the first string
 * @param s2 : the second string
 * @param connector : the connector to combine two strings
 *
 * @return : combined string
 */
static const char *string_concat_with(const char *s1,const char *s2,const char *connector ){
	int len = strlen(s1) + strlen(s2) + 1 + strlen(connector);
	char *d = gdml_zmalloc(len);
	strncpy(d,s1,strlen(s1));
	strcat(d,connector);
	strcat(d,s2);
	return d;
}

/**
 * @brief get_obj_qname : get object parent name
 *
 * @param obj : object struct
 *
 * @return : name of object parent
 */
const char *get_obj_qname(object_t *obj){
	return obj->qname;
}

/**
 * @brief get_obj_ref : get the object name of generating
 *
 * @param obj : the object to be generated
 *
 * @return : generating name of object
 */
const char *get_obj_ref(object_t *obj){
	return obj->dotname;
}

/**
 * @brief _get_arraydef : get the array definition struct of one object
 *
 * @param attr : the attribute of object
 *
 * @return : the arraydef struct of object
 */
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

/**
 * @brief process_object_names : process the object name for generating
 *
 * @param obj : the struct of object
 */
static void process_object_names(object_t *obj, arraydef_attr_t *array) {
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
	obj->is_array = (array != NULL);
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
		if(strcmp(obj->obj_type, "data")) {
		      asprintf(&name, "_idx%d", obj->depth);
		      node = create_string_node(name);
		      obj->i_node = obj->d_node = node;
		      /*rename object */
		      if(array->fix_array) {
			      index = "i";
		      } else {
			      index = array->ident;
		      }
		      asprintf(&name, "%s[$%s]", obj->name, index);
			obj->a_name = obj->name;
			obj->name = name;
			asprintf(&name, "_idx%d", depth - 1);
			param_value_t *val = gdml_zmalloc(sizeof(*val));
			val->type = PARAM_TYPE_STRING;
			val->u.string = name;	
			symbol_insert(obj->symtab, index, PARAMETER_TYPE, val);
			int size;
			if(!array->fix_array) {
				size = array->high - array->low + 1;
			} else {
				size = array->high;
			}
			obj->array_size = size;

		} else {
			obj->a_name = obj->name;
		}
	} else {
		node = create_string_node(obj->name);
		obj->i_node = obj->d_node = node;
	}
}

/**
 * @brief object_symbol_find : find object type symbol
 *
 * @param table : table to start finding symbol
 * @param name : name of symbol
 * @param type : type of symbol
 *
 * @return : pointer to symbol
 */
static symbol_t object_symbol_find(symtab_t table, const char *name, type_t type) {
	symbol_t sym = NULL;
	object_t *obj;

	obj = table->obj;
	TABLE_TRACE("IN object: try to search symbol %s, %s\n", name, obj->name);
	sym = _symbol_find(table->table, name, type);
	if(sym) {
		sym->owner = table->obj;
		TABLE_TRACE("symbol found %s in object table %p, table num %d\n", sym->name, table, table->table_num);
		return sym;
	} else if (table->sibling){
		TABLE_TRACE("let's goto his brother, %p, table_num %d, root table  %p , cb %p\n", table->sibling, table->sibling->table_num,  root_table, root_table->cb);
		sym = symbol_find(table->sibling, name, type);
		if(sym) {
			sym->owner = table->obj;
			TABLE_TRACE("symbol %s found in his brother \n", sym->name);
			return sym;
		}
	}
	TABLE_TRACE("not found, try it's parent\n");
	if(table->parent) {
		sym = symbol_find(table->parent, name, type);
		TABLE_TRACE("symbol %s found in parent symtable\n", name);
	}
	return sym;

}

/**
 * @brief object_symbol_find_notype : find object symbol without type
 *
 * @param table : table to start finding symbol
 * @param name : name of symbol
 *
 * @return : pointer to symbol
 */
static symbol_t object_symbol_find_notype(symtab_t table, const char *name) {
	symbol_t sym = NULL;
	object_t *obj;

	obj = table->obj;
	TABLE_TRACE("IN object(notype): try to search symbol %s, %s\n", name, obj->name);
	sym = _symbol_find_notype(table->table, name);
	if(sym) {
		sym->owner = table->obj;
		TABLE_TRACE("IN object(notype): symbol found %s in object table %p, table num %d, type %d\n", sym->name, table, table->table_num, sym->type);
		return sym;
	} else if (table->sibling){
		TABLE_TRACE("IN object(notype): let's goto his brother, %p, table_num %d, root table  %p, root num %d\n", table->sibling, table->sibling->table_num,  root_table, root_table->table_num);
		sym = symbol_find_notype(table->sibling, name);
		if(sym) {
			sym->owner = table->obj;
			TABLE_TRACE("IN object(notype): symbol %s found in his brother \n", sym->name);
			return sym;
		}
	}
	TABLE_TRACE("IN object(notype): not found, try it's parent\n");
	if(table->parent) {
		sym = symbol_find_notype(table->parent, name);
		if(sym) {
			TABLE_TRACE("IN object(notype): symbol %s found in parent symtable\n", name);
		}
	}
	//BE_DBG(OBJ_SYM, "symbol %s found in parent symtable\n", sym->name);
	return sym;

}

/**
 * @brief get_object_encoding : get the object type from object type name
 *
 * @param name : name of object type
 *
 * @return : object type
 */
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

extern int inc_current_table_num();
static void process_object_relationship(object_t *obj);
static void add_object_templates(object_t *obj);


/**
 * @brief init_object : initialize the object struct
 *
 * @param obj : struct of object
 * @param parent : parent of object
 * @param name : nmae of object
 * @param type : type of object
 * @param node : syntax tree node about object
 * @param table : table of object
 */
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
	if(!table && strcmp(type, "data")) {
		obj->symtab->sibling = symtab_create(TMP_TYPE);
		if (node) {
			object_attr_t* attr = node->common.attr;
			attr->common.table = obj->symtab->sibling;
		}
	} else {
		obj->symtab->sibling = table;
	}
	if(strcmp(type, "data")) {
		obj->symtab->sibling->parent  = NULL;
	}
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
	add_object_templates(obj);
}

/**
 * @brief process_object_relationship : relize the relationship between object and its parent
 *
 * @param obj : struct of object
 */
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
		} else if(!strcmp(obj->obj_type, "register") && !strcmp(parent->obj_type, "group")){
			list = &((group_t *)parent)->registers;
		} else {
			list = &parent->childs;
		}
		list_add_tail(&obj->entry, list);
		symbol_insert(parent->symtab, obj->name, OBJECT_TYPE, obj);
	}
}

static void process_default_template(object_t *obj);
static void create_none_object(object_t *obj, symbol_t sym) {
	printf("who are you %s something error happened in object name %s\n", obj->name, sym->name);
}

static void create_objs(object_t *obj, type_t type);
static void merge_table(symtab_t dst, symtab_t src) {

	symbol_t tmp;
	tmp = src->list;
	while(tmp) {
		symbol_insert(dst, tmp->name, tmp->type, tmp->attr);
		tmp = tmp->lnext;
	}
	struct template_table *templ;
	templ = src->template_table;
	while(templ ) {
		merge_table(dst, templ->table);	
		templ = templ->next;
	}
}

static void merge_node(tree_t *dst_node, tree_t *src_node) {
	symbol_t tmp;
	symtab_t dst_table;
	symtab_t src_table;
	object_attr_t *attr1, *attr2;
	
	attr1 = dst_node->common.attr;
	attr2 = src_node->common.attr;	
	dst_table = attr1->common.table;
	src_table = attr2->common.table;
	tmp = src_table->list;
	while(tmp) {
		symbol_insert(dst_table, tmp->name, tmp->type, tmp->attr);
		tmp = tmp->lnext;
	}
	obj_spec_t *spec = src_node->reg.spec;
	while(spec) {
		tree_t *tmp;
		tmp = spec->node;
		tree_t *block = tmp->spec.block;
		if(!block) { 
			spec = spec->next;
			continue;
		}
		tree_t *s;
		s = block->block.statement;
		while(s) {
			if(s->common.type == IDENT_TYPE) {
				const char *name = s->ident.str;		
				add_template_to_table(src_table, name, 0);
			}
			s = s->common.sibling;
		}
		spec = spec->next;
	}
	struct template_table *templ;
	templ = src_table->template_table;
	while(templ) {
		merge_table(dst_table, templ->table);
		templ = templ->next;
	}
}

static void merge_object(object_t *obj, symbol_t sym) {
	symbol_list_t *list = NULL;
	symtab_t table = obj->parent->symtab->sibling;
	symbol_list_t *head;

	head = list = symbol_list_find_full(table, sym->name, REGISTER_TYPE);	
	struct object_common *attr;
	while(list) {
		if(list->sym == sym) {
			list = list->next;
			continue;
		}
		attr = list->sym->attr;	
		parse_register(attr->node, attr->table);
		list = list->next;
	}
	list = head;
	while(list) {
		if(list->sym == sym) {
			list = list->next;
			continue;
		}
		attr = list->sym->attr;
		symtab_t sub_tab;
		sub_tab = attr->table;
		if(sub_tab) {
			if(!obj->symtab->sibling) {
				obj->symtab->sibling = symtab_create(TMP_TYPE);
			}
			merge_table(obj->symtab->sibling, sub_tab);
			struct template_table *temp = sub_tab->template_table;
			/*
			while(temp)
			{
				fprintf(stderr, "merge template %s, table %p\n", temp->template_name, temp->table);
	
				if(!strcmp(temp->template_name, "pci_status_register")) { 
					print_all_symbol(temp->table);
				}
				//merge_table(obj->symtab->sibling, temp->table);		
				if(!strcmp(temp->template_name, "pci_status_register")) {
				exit(-1);
				}
				temp = temp->next;
			}*/
		}
		list = list->next;
	}	
}

static void merge_symbol(symbol_t dst, symbol_t src) {
	struct object_common *attr1, *attr2;

	attr1 = dst->attr;
	attr2 = src->attr;
	
	if(!attr1->table) {
		attr1->table = symtab_create(TMP_TYPE);
	}
	parse_register(attr2->node, attr2->table);
	if(attr2->table) {
		print_all_symbol(attr1->table);
		
		print_all_symbol(attr2->table);
		
		merge_node(attr1->node, attr2->node);
		//Add_object_node();
	}
}

/**
 * @brief create_dummy_field : create a dummy field
 *
 * @param obj : the parent object of filed
 *
 * @return : filed object
 */
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

/**
 * @brief create_field_object : create field object
 *
 * @param obj : the parent object of field
 * @param sym : symbol of field
 */
static void create_field_object(object_t *obj, symbol_t sym){
	field_t *fld = gdml_zmalloc(sizeof(*fld));
	field_attr_t *field_attr = (field_attr_t *)sym->attr;
	tree_t *node = field_attr->common.node;
	symtab_t table = field_attr->common.table;
	parse_field_attr(node, obj->symtab);
	/* parse the elements that in filed table*/
	parse_field(node, table);
	init_object(&fld->obj, obj, sym->name, "field", node, table);
	create_objs(&fld->obj, DATA_TYPE);
}

int in_group = 0;
/**
 * @brief create_register_object : create register object
 *
 * @param obj : the parent object of register
 * @param sym : symbol of register
 */
static void create_register_object(object_t *obj, symbol_t sym){
	symtab_t table = obj->symtab;
	symbol_t sym2;
	//if((sym2 = _symbol_find(table->table, sym->name, OBJECT_TYPE)) != NULL) {
		/* merge two object */
	//	merge_object((object_t *)sym2->attr, sym);
	//} else 
	{

		dml_register_t *reg = (dml_register_t *)gdml_zmalloc(sizeof(*reg));
		register_attr_t *reg_attr = (register_attr_t *)(sym->attr);
		symtab_t table = reg_attr->common.table;

		print_all_symbol(obj->symtab->sibling);
		if(!strcmp(obj->obj_type, "group")) {
			//in_group = 1;	
		}
		parse_register_attr(reg_attr->common.node, obj->symtab);
		in_group = 0;
		parse_register(reg_attr->common.node, table);

		init_object(&reg->obj, obj, sym->name, "register", reg_attr->common.node, table);
		//merge_object(&reg->obj, sym);
		create_objs(&reg->obj, FIELD_TYPE);
		create_objs(&reg->obj, DATA_TYPE);
		create_objs(&reg->obj, EVENT_TYPE);
		if(!strcmp(sym->name, "status")) {
			tree_t *node = reg_attr->common.node;
		}
		merge_object(&reg->obj, sym);
	}
	//print_all_symbol(reg->obj.symtab);
}

/**
 * @brief create_iface_object : create the interface object
 *
 * @param obj : the parent object of interface
 * @param sym : symbol of interface
 */
static void create_iface_object(object_t *obj, symbol_t sym) {
	interface_t *iface = (interface_t *)gdml_zmalloc(sizeof(*iface));
	interface_attr_t *attr = (interface_attr_t *)sym->attr;
	symtab_t table = attr->common.table;
	init_object(&iface->obj, obj, sym->name, "interface", attr->common.node, table);
}

/**
 * @brief create_connect_object : create connect object
 *
 * @param obj : the object of connect
 * @param sym : symbol of connect
 */
static void create_connect_object(object_t *obj, symbol_t sym) {
	connect_t *con = (connect_t *)gdml_zmalloc(sizeof(*con));
	connect_attr_t *attr = (connect_attr_t *)(sym->attr);
	symtab_t table = attr->common.table;
	parse_connect_attr(attr->common.node, obj->symtab);
	/* parse elements and calculate expressions that
	 * in  connect table*/
	parse_connect(attr->common.node, table);
	init_object(&con->obj, obj, sym->name, "connect", attr->common.node, table);
	create_objs(&con->obj, INTERFACE_TYPE);
	create_objs(&con->obj, DATA_TYPE);
	create_objs(&con->obj, EVENT_TYPE);
}

/**
 * @brief create_attribute_object : create the attribute object
 *
 * @param obj : object of attribute
 * @param sym : the symbol of attribute
 */
static void create_attribute_object(object_t *obj, symbol_t sym) {
	attribute_t *att = (attribute_t *)gdml_zmalloc(sizeof(*att));
	attribute_attr_t *attr = (attribute_attr_t *)(sym->attr);
	symtab_t table = attr->common.table;
	parse_attribute_attr(attr->common.node, obj->symtab);
	/* parse the elements that in the attribute table*/
	parse_attribute(attr->common.node, table);

	init_object(&att->obj, obj, sym->name, "attribute", attr->common.node, table);
	//parse_attribute_attr(att->obj.node, att->obj.symtab->parent);
	/* parse the elements that in the attribute table*/
	//parse_attribute(att->obj.node, att->obj.symtab->sibling);
		
	create_objs(&att->obj, DATA_TYPE);
	create_objs(&att->obj, EVENT_TYPE);
}

/**
 * @brief create_implement_object : create implement object
 *
 * @param obj : the parent object of implement
 * @param sym : symbol of implement
 */
static void create_implement_object(object_t *obj, symbol_t sym){
	implement_t *impl = (implement_t *)gdml_zmalloc(sizeof (*impl));
	implement_attr_t *attr = (implement_attr_t *)sym->attr;
	symtab_t table = attr->common.table;
	init_object(&impl->obj, obj, sym->name, "implement", attr->common.node, table);
}

/**
 * @brief create_data_object : create data object
 *
 * @param obj : the parent object of data
 * @param sym : symbol of data
 */
static void create_data_object(object_t *obj, symbol_t sym) {
	data_t *data = (data_t *)gdml_zmalloc(sizeof (*data));
	cdecl_t *type = (cdecl_t *)sym->attr;
	init_object(&data->obj, obj, sym->name, "data", type->node, NULL );
	if(type->common.bty) {
		//fprintf(stderr, "type %d\n", type->common.bty->common.categ);
	}
}

/**
 * @brief create_port_object : create port object
 *
 * @param obj : the parent object of port, often device
 * @param sym : symbol of port
 */
static void create_port_object(object_t *obj, symbol_t sym) {
	if(!obj || strcmp(obj->obj_type, "device")) {
		BE_DBG(OBJ, "port should in top level\n");
		printf("port should in top level\n");
	}
	dml_port_t *port = (dml_port_t *)gdml_zmalloc(sizeof(*port));
	port_attr_t *attr = (port_attr_t *)sym->attr;
	symtab_t table = attr->common.table;
	init_object(&port->obj, obj, sym->name, "port", attr->common.node, table);
	INIT_LIST_HEAD(&port->connects);
	INIT_LIST_HEAD(&port->implements);
	INIT_LIST_HEAD(&port->attributes);
	create_objs(&port->obj, IMPLEMENT_TYPE);
	create_objs(&port->obj, DATA_TYPE);
	create_objs(&port->obj, EVENT_TYPE);
	create_objs(&port->obj, CONNECT_TYPE);
	create_objs(&port->obj, ATTRIBUTE_TYPE);
}

/**
 * @brief create_event_object : create event object
 *
 * @param obj : the parent object of event
 * @param sym : symbol of event
 */
static void create_event_object(object_t *obj, symbol_t sym) {
	if(!obj || strcmp(obj->obj_type, "device")) {
		BE_DBG(OBJ, "event should in top level\n");
	}
	event_t *event = (event_t *)gdml_zmalloc(sizeof(*event));
	event_attr_t *attr = (event_attr_t *)sym->attr;
	symtab_t table = attr->common.table;
	parse_event(attr->common.node, table);
	init_object(&event->obj, obj, sym->name, "event", attr->common.node, table);
	create_objs(&event->obj, DATA_TYPE);
}

/**
 * @brief create_group_object : create group object
 *
 * @param obj : the parent object of group
 * @param sym : symbol of group
 */
static void create_group_object(object_t *obj, symbol_t sym) {
	group_t *gp = (group_t *)gdml_zmalloc(sizeof(*gp));
	group_attr_t *attr = (group_attr_t *)sym->attr;
	symtab_t table;

	table = attr->common.table;
	gp->obj.is_abstract = 1;
	parse_group_attr(attr->common.node, table);
	parse_group(attr->common.node, table);
	INIT_LIST_HEAD(&gp->groups);
	INIT_LIST_HEAD(&gp->attributes);
	INIT_LIST_HEAD(&gp->registers);
	init_object(&gp->obj, obj, sym->name, "group", attr->common.node, table);
	create_objs(&gp->obj, REGISTER_TYPE);
	create_objs(&gp->obj, GROUP_TYPE);
	create_objs(&gp->obj, DATA_TYPE);
	create_objs(&gp->obj, EVENT_TYPE);
	create_objs(&gp->obj, ATTRIBUTE_TYPE);
	print_all_symbol(gp->obj.symtab);
}

object_t *default_bank;
/**
 * @brief create_bank_object : create bank object
 *
 * @param obj : the parent object of bank
 * @param sym : the symbol of bank
 */
static void create_bank_object(object_t *obj, symbol_t sym){
	bank_t *bank = gdml_zmalloc(sizeof(*bank));
	INIT_LIST_HEAD(&bank->groups);
	bank_attr_t *b = (bank_attr_t *)(sym->attr);
	symtab_t table = b->common.table;

	if(b->common.node)
		parse_bank(b->common.node, table);
	if(!strcmp(sym->name, "no_name_bank"))
		default_bank = &bank->obj;
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
	//fprintf(stderr, "bankxxx name %s, file %s, line %d\n", sym->name, b->common.node->common.location.file->name, b->common.node->common.location.first_line);
}

/**
 * @brief : array to store the pointer of function for creating object
 *
 * @param obj : parent object
 * @param sym : symbol of child object
 */
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

/**
 * @brief type2obj_type : switch symbol type to object type
 *
 * @param type : the type of object symbol
 *
 * @return  : type of object
 */
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
		case GROUP_TYPE:
			ret = Obj_Type_Group;
			break;
		default:
			ret = Obj_Type_None;
	}
	return ret;
}

/**
 * @brief fake_bank_list : create a fake bank list when one device have on bank
 *
 * @return : list of fake bank
 */
symbol_list_t* fake_bank_list() {
	bank_attr_t* attr = (bank_attr_t*)gdml_zmalloc(sizeof(bank_attr_t));
	attr->common.obj_type = BANK_TYPE;
	attr->common.name = strdup("__fake_bank");
	attr->common.table = symtab_create(BANK_TYPE);
	symbol_t sym = (symbol_t)gdml_zmalloc(sizeof(symbol_t));
	sym->name = strdup("__fake_bank");
	sym->attr = attr;
	symbol_list_t* list = (symbol_list_t*)gdml_zmalloc(sizeof(symbol_list_t));
	list->sym = sym;
	return list;
}

typedef struct node_created {
	symbol_t sym;
	struct list_head entry;
} node_name_t;


static symbol_t symbol_created(struct list_head *p, symbol_t sym) {
	struct list_head *tmp;
	node_name_t *node;

	list_for_each(tmp, p) {
		node = list_entry(tmp, node_name_t, entry);
		if(!strcmp(sym->name, node->sym->name)) {
			return node->sym;
		}
	}
	return NULL;
}

static void add_to_created(struct list_head *head, symbol_t sym) {
	struct list_head *p;
	node_name_t *node;

	node = gdml_zmalloc(sizeof(*node));
	node->sym = sym;
	INIT_LIST_HEAD(&node->entry);
	list_add_tail(&node->entry, head);
}

/**
 * @brief create_objs : create child object with type
 *
 * @param obj : parent object
 * @param type : type of child object
 */
static void create_objs(object_t *obj, type_t type) {
	symtab_t table = obj->symtab->sibling;
	if (table)
		check_undef_template(table);
	symbol_list_t *list = NULL;
	symbol_list_t *head = list;
	object_type_t obj_type;
	LIST_HEAD(created);
	struct list_head *p;
	symbol_t sym2;

	list = symbol_list_find_type(table, type);
	obj_type = type2obj_type(type);
	if (type == BANK_TYPE && list == NULL) {
		list = fake_bank_list();
	}
	print_all_symbol(table);
	symbol_list_t *list2 = list;
	while(list) {
		if((sym2 = symbol_created(&created, list->sym)) != NULL) {
			merge_symbol(sym2, list->sym);
		} else {
			add_to_created(&created, list->sym);
		}
        	list = list->next;
	}
	/*
	list = list2;
	while(list) {
		create_func[obj_type](obj, list->sym);
		list = list->next;
	}*/
	list_for_each(p, &created) {
		node_name_t *tmp = list_entry(p, node_name_t, entry);
		create_func[obj_type](obj, tmp->sym);
	}
    	symbol_list_free(head);
}


typedef struct create_obj_args {
	object_t *obj;
	object_type_t type;
} obj_arg_t;

static void create_objx(symbol_t sym, void *data) {
	obj_arg_t *arg = (obj_arg_t *)data;
	object_t *obj = arg->obj;;
	object_type_t obj_type = arg->type;

	create_func[obj_type](obj, sym);
}

/*
static void create_objs(object_t *obj, type_t type) {
	symtab_t table = obj->symtab->sibling;
	object_type_t obj_type;

	obj_type = type2obj_type(type);
	obj_arg_t args = {.obj = obj, .type = obj_type};
	table_visit(table, type, create_objx, (void *)&args);
}
*/

typedef struct create_node_args {
	device_t *dev;
	type_t type;
} node_args_t;

typedef enum toplevel_type {
	TOPLEVEL_NONE,
	TOPLEVEL_HEADER,
	TOPLEVEL_FOOTER,
	TOPLEVEL_LOGGROUP,
	TOPLEVEL_STRUCT,
	TOPLEVEL_EXTERN,
	TOPLEVEL_TYPEDEF,
	TOPLEVEL_EXTERN_TYPEDEF,
	TOPLEVEL_CDECL_TYPE
} toplevel_type_t;

static void toplevel_create_none(device_t *dev, symbol_t sym) {
	printf("toplevel: please use your self implementation\n");
	return;
}

static void toplevel_create_header(device_t *dev, symbol_t sym) {
	node_entry_t *tmp;

	tmp = (node_entry_t *)create_node_entry(sym);
	list_add_tail(&tmp->entry, &dev->headers);
	return;
}

static void toplevel_create_footer(device_t *dev, symbol_t sym) {
	node_entry_t *tmp;

	tmp = create_node_entry(sym);
	list_add_tail(&tmp->entry, &dev->footers);
}


static void toplevel_create_loggroup(device_t *dev, symbol_t sym) {
	node_entry_t *tmp;

	tmp	= create_node_entry(sym);
	list_add_tail(&tmp->entry, &dev->loggroups);
}

static void toplevel_create_struct(device_t *dev, symbol_t sym) {
	node_entry_t *tmp;

	tmp = create_node_entry(sym);
	list_add_tail(&tmp->entry, &dev->struct_defs);
}

static void toplevel_create_extern(device_t *dev, symbol_t sym) {
	node_entry_t *tmp;

	tmp = create_node_entry(sym);
	list_add_tail(&tmp->entry, &dev->externs);
}

static void toplevel_create_typedef(device_t *dev, symbol_t sym) {
	node_entry_t *tmp;
	tree_t *node;
	cdecl_t *type;

	type = sym->attr;
	node = type->node;
	tmp = create_node_entry(sym);
	if(!type->common.is_extern) {
		list_add_tail(&tmp->entry, &dev->typedefs);
	} else {
		list_add_tail(&tmp->entry, &dev->extern_typedefs);
	}
}

static void toplevel_create_extern_typedef(device_t *dev, symbol_t sym) {
	node_entry_t *tmp;

	tmp = create_node_entry(sym);
	list_add_tail(&tmp->entry, &dev->extern_typedefs);
}

static void toplevel_create_cdecl(device_t *dev, symbol_t sym) {
	node_entry_t *tmp;
	tree_t *node;

	tmp = create_node_entry(sym);
}

static void (*toplevel_func[])(device_t *dev, symbol_t sym) = {
	[TOPLEVEL_NONE] = toplevel_create_none,
	[TOPLEVEL_HEADER] = toplevel_create_header,
	[TOPLEVEL_FOOTER] = toplevel_create_footer,
	[TOPLEVEL_LOGGROUP] = toplevel_create_loggroup,
	[TOPLEVEL_STRUCT] = toplevel_create_struct,
	[TOPLEVEL_EXTERN] = toplevel_create_extern,
	[TOPLEVEL_TYPEDEF] = toplevel_create_typedef,
	[TOPLEVEL_EXTERN_TYPEDEF] = toplevel_create_extern_typedef,
	[TOPLEVEL_CDECL_TYPE] = toplevel_create_cdecl
};

static toplevel_type_t get_toplevel(type_t type) {
	toplevel_type_t top_type;

	switch(type) {
		case HEADER_TYPE:
			top_type = TOPLEVEL_HEADER;
			break;
		case FOOTER_TYPE:
			top_type = TOPLEVEL_FOOTER;
		   	break;
		case STRUCT_TYPE:
		    top_type = TOPLEVEL_STRUCT;
		   	break;
		case TYPEDEF_T:
		   	top_type = TOPLEVEL_TYPEDEF;
		   	break;
		case LOGGROUP_TYPE:
		   	top_type = TOPLEVEL_LOGGROUP;
		   	break;
			/* a complex case */
		default:
		   	top_type = TOPLEVEL_NONE;
	}
	return top_type;
}


static void create_misc_node(symbol_t sym, void *data) {
	node_args_t *args = (node_args_t *)data;
	toplevel_type_t toplevel;

	toplevel = get_toplevel(args->type);
	toplevel_func[toplevel](args->dev, sym);
}

static void create_dev_node_type(device_t *dev, type_t type) {
	symtab_t table = root_table;

	node_args_t args = {.dev = dev, .type = type};
	table_visit(table, type, create_misc_node, (void *)&args);
}

static void create_device_misc(device_t *dev) {
	create_dev_node_type(dev, HEADER_TYPE);
	create_dev_node_type(dev, FOOTER_TYPE);
	create_dev_node_type(dev, STRUCT_TYPE);
	create_dev_node_type(dev, TYPEDEF_T);
}

extern object_t *device;
/* @brief create_device_object : create the object of device and its member
 *
 * @param sym : symbol of device
 *
 * @return : pointer to device object
 */
object_t *create_device_object(symbol_t sym){
	int i = 0;
	struct list_head *p;
	object_t *obj;

	device_t *dev =  (device_t *)gdml_zmalloc(sizeof(*dev));
	if(DEV)
		BE_DBG(OBJ, "device exists\n");
	device_attr_t *dev_attr = (device_attr_t *)sym->attr;
	BE_DBG(OBJ, "device %s found\n",sym->name);
	param_value_t *val = (param_value_t *)gdml_zmalloc(sizeof(*val));
	val->type = PARAM_TYPE_STRING;
	val->u.string = sym->name;
	symbol_insert(root_table, "name", PARAMETER_TYPE, val);
	parse_device(dev_attr->common.node, root_table);	
	init_object(&dev->obj, NULL, sym->name, "device", dev_attr->common.node, root_table);
	p = &dev->constants;
	for(i = 0; i < 12; i++, p++) {
		INIT_LIST_HEAD(p);
	}
	obj = &dev->obj;
	DEV = obj;
	device = &dev->obj;
	create_objs(obj, BANK_TYPE);
	create_objs(obj, CONNECT_TYPE );
	create_objs(obj, ATTRIBUTE_TYPE);
	create_objs(obj, IMPLEMENT_TYPE);
	create_objs(obj, PORT_TYPE);
	create_objs(obj, EVENT_TYPE);
	create_objs(obj, DATA_TYPE);
	return obj;
}

/**
 * @brief create_device_tree : create the structure about one device,
 * include its bank, register, field, attribute, connect and so on.
 *
 * @param root : the root of syntax tree
 *
 * @return : pointer to device object struct
 */
device_t *create_device_tree(tree_t *root){
	object_t *obj;

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

/**
 * @brief create_template_name : switch template symbol into template struct of object
 *
 * @param obj : the object with template
 * @param name : name of template
 */
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

static void data_realize(object_t *obj);
static void group_realize(object_t *obj);

extern void set_current_obj(object_t* obj);
/**
 * @brief field_realize : realize the content of field
 *
 * @param obj : the object of field
 */
static void field_realize(object_t *obj) {
	field_t *fld = (field_t *)obj;
	tree_t *bitrange;
	tree_t *expr;
	tree_t *templates = (obj->node) ? obj->node->field.templates : NULL;

	set_current_obj(obj);
	//add_object_templates(obj);
	if(fld->is_dummy) {
		process_object_names(obj, NULL);
		process_object_templates(obj);
		return;
	}

	/* parse default parameters about registers
	 * such as: size, offset, array*/
	//parse_field_attr(obj->node, obj->symtab->parent);
	/* parse the elements that in filed table*/
	//parse_field(obj->node, obj->symtab);
	bitrange = obj->node->field.bitrange;
	if (bitrange) {
		expr = bitrange->array.expr;
		fld->is_fixed = bitrange->array.is_fix;
		fld->high = expr->int_cst.value;
		if(fld->is_fixed) {
			fld->len = 1;
			fld->low = fld->high;
		} else {
			expr = bitrange->array.expr_end;
			fld->low = expr->int_cst.value;
			fld->len = fld->high - fld->low + 1;
		}
	}
	process_object_names(obj, NULL);
	process_object_templates(obj);
	return;
}

/**
 * @brief get_binopnode_constant : get the value from constant node
 *
 * @param t : syntax tree node
 * @param op : the operator of calculate
 * @param result : result of value
 *
 * @return : 0 = left node is constant value
 *			1 = right node is constant value
 */
static int get_binopnode_constant(tree_t *t, type_t op, int *result, symtab_t table) {
	tree_t *left, *right;
	int ret = -1;
	expr_t *lexpr;
	expr_t *rexpr;
	int tmp;

	if(!t || (t->common.type == UNDEFINED_TYPE)
		|| ((t->common.type != INTEGER_TYPE) && (t->common.type != BINARY_TYPE))) {
		*result = -1;
		ret = 2;
		return ret;
	}
	left = t->binary.left;
	right = t->binary.right;
	if(t->common.type == INTEGER_TYPE) {
		*result = t->int_cst.value;
		ret = 2;
		return ret;
	}
	lexpr = check_expr(left, table);
	rexpr = check_expr(right, table);	
	if(lexpr->is_const && lexpr->type->common.categ == INT_T) {
			tmp = lexpr->val->int_v.value;
			*result = tmp;	
			ret = 0;
			return ret;
	}
	if(rexpr->is_const && rexpr->type->common.categ == INT_T) {
			tmp = rexpr->val->int_v.value;
			*result = tmp;	
			ret = 1;
			return ret;
	}
	if(left->common.type == INTEGER_TYPE && right->common.type == INTEGER_TYPE && t->binary.type == op) {
		if(op == ADD_TYPE){
			int tmp = left->int_cst.value + right->int_cst.value;
			*result = tmp;
			ret = 2;
			return ret;
		}
	}
	if(t->common.type != BINARY_TYPE || t->binary.type != op) {
		*result = -1;
		ret = 2;
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

/**
 * @brief get_reg_offset : get the offset of register
 *
 * @param t : the parameter spec of offset
 * @param interval : the value of offset
 *
 * @return : offset of register
 */
static int get_reg_offset(paramspec_t *t, int *interval, symtab_t table) {
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
		ret = get_binopnode_constant(node, ADD_TYPE, &offset, table);
		if(ret < 0) {
			offset = -2;
		} else if(!ret) {
			tmp = node->binary.right;
		} else if(ret == 2) {
			return offset;
		} else {
			tmp = node->binary.left;
		}
		ret = get_binopnode_constant(tmp, MUL_TYPE, interval, table);
		if(ret < 0) {
			BE_DBG(GENERAL, "wrong register offset format, node type %d, expected format %d(*)\n", node->common.type, MUL_TYPE);
		}
	}
	return offset;
}

static int my_index;
static int get_offset_info(tree_t *expr, offset_info_t *info, symtab_t table) {
	tree_t *left, *right;
	expr_t *lexpr;
	expr_t *rexpr;
	expr_t *tmp;

	if(expr->common.type == BINARY_TYPE && expr->binary.type == ADD_TYPE) {
		left = expr->binary.left;	
		right = expr->binary.right;
		/*
		lexpr = check_expr(left, table);	
		rexpr = check_expr(right, table);
		if(lexpr->is_const && lexpr->type->common.categ == INT_T) {
			info->offset = lexpr->val->int_v.value;				
			fprintf(stderr, "offset 0x%x\n", info->offset);
		} else {
			get_offset_info(left, info, table);
		} */ 
		if(left->common.type == INTEGER_TYPE) {
			info->offset = left->int_cst.value;
		} else {
			get_offset_info(left, info, table);
		}
		if(right->common.type == BINARY_TYPE && right->binary.type == MUL_TYPE) {
			tree_t *sub_left = right->binary.left;	
			tree_t *sub_right = right->binary.right;	
			int tmp;
			if(sub_left->common.type == INTEGER_TYPE) {
				tmp = sub_left->int_cst.value;	
			} else if(sub_right->common.type == INTEGER_TYPE) {
				tmp = sub_right->int_cst.value;	
			}
			info->interval[my_index++] = tmp;	
		} else if(right->common.type == QUOTE_TYPE){
			info->interval[my_index++] = 1;	
		} else {
			fprintf(stderr, "file: %s, line %d error offset format, expect multiply, got %d\n",
										right->common.location.file->name,
										right->common.location.first_line,
										right->common.type);
			exit(-1);
		}
	} else if(expr->common.type == BINARY_TYPE && expr->binary.type == MUL_TYPE) {
			tree_t *sub_left = expr->binary.left;	
			tree_t *sub_right = expr->binary.right;	
			int tmp;
			if(sub_left->common.type == INTEGER_TYPE) {
				tmp = sub_left->int_cst.value;	
			} else if(sub_right->common.type == INTEGER_TYPE) {
				tmp = sub_right->int_cst.value;	
			}
			//if(my_index < MAX_DEPTH) {
				info->interval[my_index++] = tmp;	
			//}
	} else {
		fprintf(stderr, "file %s, line %d\n", expr->common.location.file->name, expr->common.location.first_line);
		fprintf(stderr, "error offset format, got %d, binary %d, expr %p\n", expr->common.type, expr->binary.type, expr);
		exit(-1);
	}
	return 0;
}

static int get_register_offset2(object_t *reg, tree_t *offset_expr, symtab_t table, offset_info_t *info) {
	int interval = -1;
	tree_t *expr;
	tree_t *left;
	tree_t *right;
	expr_t *lexpr;
	expr_t *rexpr;
	int tmp;
	int ret;
	int multi = 0; 
	tree_t *complex_expr;
	expr = offset_expr;				
	if(!expr)
		return -1;
	ret = get_offset(expr, table);
	if(ret >= 0) {
		info->offset = ret;
		return;
	} else if(ret == -2) {
		return -1;
	}
	expr = offset_expr;					
	if(expr->common.type == EXPR_BRACK_TYPE) {
		expr = expr->expr_brack.expr_in_brack;
	}
	if(offset_expr->common.type == BINARY_TYPE && offset_expr->binary.type == MUL_TYPE) {
		if(offset_expr->binary.right->common.type == INTEGER_TYPE) {
			interval = offset_expr->binary.right->int_cst.value; 	
			if(offset_expr->binary.left->common.type == EXPR_BRACK_TYPE){
				expr = 	offset_expr->binary.left->expr_brack.expr_in_brack;
				multi = 1;
			} else {
				info->offset = 0;	
				info->interval[0] = interval; 
				return ;
			}
		} else if(offset_expr->binary.left->common.type == INTEGER_TYPE) {
			interval = offset_expr->binary.left->int_cst.value; 	
			if(offset_expr->binary.right->common.type == EXPR_BRACK_TYPE){
				expr = 	offset_expr->binary.right->expr_brack.expr_in_brack;
				multi = 1;
			} else {
				info->offset = 0;	
				info->interval[0] = interval; 
				return ;
			}
		}
	}
	if(expr->common.type == BINARY_TYPE && expr->binary.type == ADD_TYPE)	
	{
		left = expr->binary.left;
		right = expr->binary.right;
	} else {
		fprintf(stderr, "expect binary op , got %d, ADD_TYPE %d, file %s, line %d\n", expr->common.type, ADD_TYPE,
												expr->common.location.file->name,
												expr->common.location.first_line);
		exit(-1);
	}
	//complex_expr = expr;
	/*
	if(left->common.type == INTEGER_TYPE) {
		tmp = left->int_cst.value;
		if(interval != -1) {
			info->offset = tmp * interval;	
		} else {
			info->offset = tmp;
		}	
		goto check_multiply;
	} else {
		tmp = left->int_cst.value;
		if(interval != -1) {
			info->offset = tmp * interval;	
		} else {
			info->offset = tmp;
		}	
		goto check_multiply;	
	} */
	lexpr = check_expr(left, table);
	rexpr = check_expr(right, table);	
	if(lexpr->is_const && lexpr->type->common.categ == INT_T) {
			tmp = lexpr->val->int_v.value;
			if(interval != -1)
				info->offset = tmp * interval;
			else 
				info->offset = tmp;
			expr = right;
			goto check_multiply;
	} else if(rexpr->is_const && rexpr->type->common.categ == INT_T) {
			tmp = rexpr->val->int_v.value;
			if(interval != -1)
				info->offset = tmp * interval;
			else 
				info->offset = tmp;
			expr = left;
			goto check_multiply;
	} 
	my_index = 0;
	ret = get_offset_info(expr, info, table);
	if(ret == -1) {
		goto check_multiply;	
	}
	if(interval != -1) {
		info->offset *= interval;
		for(tmp = 0; tmp < MAX_DEPTH; tmp++){
			if(info->interval[tmp] > 0) {
				info->interval[tmp] *= interval;	
			}
		}
	}
	return;
	check_multiply:
		if(expr->common.type == BINARY_TYPE && expr->binary.type == MUL_TYPE) {
			if( !multi  && (expr->binary.left->common.type == EXPR_BRACK_TYPE || expr->binary.right->common.type == EXPR_BRACK_TYPE)) {
				if(expr->binary.left->common.type == INTEGER_TYPE) {
					tmp = expr->binary.left->int_cst.value;	
					interval = tmp;
					expr = expr->binary.right->expr_brack.expr_in_brack;
				} else if(expr->binary.right->common.type == INTEGER_TYPE) {
					tmp = expr->binary.right->int_cst.value;	
					interval = tmp;
					expr = expr->binary.left->expr_brack.expr_in_brack;
				}		
					
			}
			ret = get_offset_info(expr, info, table);
			if(interval != -1){	
				for(tmp = 0; tmp < MAX_DEPTH; tmp++){
					if(info->interval[tmp] > 0) {
						info->interval[tmp] *= interval;	
					}
				}
			}
		}
	return;
				
}
static void event_realize(object_t *obj);

static int register_unmapped(object_t *obj) {
	symtab_t table = obj->symtab->sibling;
	struct template_table *temp;
	
	temp = table->template_table;
	while(temp) {
		if(!strcmp(temp->template_name, "unmapped")) {
			return 1;
		}
		temp = temp->next;
	}	
	return 0;	
}

static void print_offset_info(offset_info_t *info) {
#if 0
	if(info) {
		int i;
		for(i = 0; i < MAX_DEPTH; i++) {
			fprintf(stderr, "interval %d 0x%x\n", i, info->interval[i]);
		}
	}		
#endif
}

/**
 * @brief register_realize : realize the content of register
 *
 * @param obj : the object of register
 */
static void register_realize(object_t *obj) {
	dml_register_t *reg  = (dml_register_t *)obj;
	register_attr_t *reg_attr;
	int i = 0;
	int register_size = g_register_size;
	struct list_head *p;
	object_t *tmp;
	arraydef_attr_t *arraydef = NULL;
	symbol_t sym;
	parameter_attr_t *parameter_attr;
	const char *index;
	int depth;

	//set_current_obj(obj);
	/* calculate the number of filed */
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

	//add_object_templates(obj);
	/* parse the attribute about registers
	 * such as: size, offset, array*/
	//parse_register_attr(reg->obj.node, obj->symtab->parent);
	/* parse the elements that in the register table */
	//parse_register(obj->node, obj->symtab->sibling);
	reg_attr = reg->obj.node->common.attr;
	reg->is_array = reg_attr->is_array;
	reg->size = reg_attr->size;
	/*take default value, if not specified*/
	if(!reg->size || reg->size == -1) {
		reg->size = register_size;
	}
	process_object_names(obj, reg_attr->arraydef);	
	reg->offset = reg_attr->offset;
	reg->array_size = obj->array_size;
	reg->is_undefined = 0;
	reg->is_unmapped = register_unmapped(obj);
	if(reg->offset == -2) 
		reg->is_undefined = 1;
	if(reg->offset >= 0) 
		reg->offset_info.offset = reg->offset;
	if(reg->offset == -1) {
		int ret = -1;
		param_value_t valuexxx;
		memset(&valuexxx, 0, sizeof(valuexxx));
		paramspec_t spec = {.expr_node = reg->obj.node->reg.offset, .value = &valuexxx};
		reg->offset = get_reg_offset(&spec, &ret, obj->symtab);
		my_index = 0;
		ret = get_register_offset2(obj, spec.expr_node, obj->symtab, &reg->offset_info);
		print_offset_info(&reg->offset_info);
		reg->offset = -3;
		if(ret != -1) {
				reg->interval = ret;
		} else {
				reg->interval = 4;
		}	
		if(ret == -1) {
			/*search for parameter offset */
			sym = symbol_find(obj->symtab, "offset", PARAMETER_TYPE);
			if(!sym) {
				reg->is_undefined = 1;
			} else {
				parameter_attr  = (parameter_attr_t *)sym->attr;
				reg->offset = get_reg_offset(parameter_attr->param_spec, &reg->interval, obj->symtab->parent);
				ret = get_register_offset2(obj, parameter_attr->param_spec->expr_node, obj->symtab, &reg->offset_info);
				if(ret == -1) {
					reg->is_undefined = 1;
				}
				if(reg->interval == -1) {
					BE_DBG(GENERAL, "register array interval not right\n");
					exit(-1);
				}
				BE_DBG(GENERAL, "reg offset 0x%x\n", reg->offset);
			}
		}
	}
	/*
	int ret = -1;
	param_value_t valuexxx = {.isconst = 0};
	memset(&valuexxx, 0, sizeof(valuexxx));
	paramspec_t spec = {.expr_node = reg->obj.node->reg.offset, .value = &valuexxx};
	reg->offset = get_reg_offset(&spec, &ret);
	if(ret != -1) {
		reg->interval = ret;
	} else {
		reg->interval = 4;
	}*/
	list_for_each(p, &obj->childs) {
		tmp = list_entry(p, object_t, entry);
		field_realize(tmp);
	}
	list_for_each(p, &obj->events) {
		tmp = list_entry(p, object_t, entry);
		event_realize(tmp);
	}
	list_for_each(p, &obj->data) {
		tmp = list_entry(p, object_t, entry);
		data_realize(tmp);
	}
	process_object_templates(obj);
}

static int  cal_group_offset(object_t *obj) {
	group_t *group = (group_t *)obj;		
	int offset = 0;

	offset = group->offset_info.offset + group->offset_info.len * obj->array_size;	
	printf("group offset_info len 0x%x, array_size 0x%x\n",  group->offset_info.len,  obj->array_size);
	return offset;
}

/**
 * @brief bank_calculate_register_offset : calculate the size about bank based on register offset
 *
 * @param obj : the object of bank
 */
static void bank_calculate_register_offset(object_t *obj) {
	bank_t *bank = (bank_t *)obj;
	dml_register_t *reg;
	register_attr_t *reg_attr;
	int i = 0;
	int offset = 0;
	int register_size = bank->register_size;
	int size;
	struct list_head *p;
	object_t *tmp;
	int group_offset = 0;
	int offset_tmp;

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
			size = reg->array_size * reg->offset_info.interval[0];
		} else {
			size = reg->size;
		}
		offset += size;
	}
	list_for_each(p, &bank->groups) {
		tmp = list_entry(p, object_t, entry);	
		 offset_tmp = cal_group_offset(tmp);	
		if(offset_tmp > group_offset) {
			group_offset = offset_tmp;
		}
	}
	if(group_offset > offset)
		offset = group_offset;
	
	//printf("offset 0x%x, bank reg count %d\n", bank->reg_count);
	/*take the register total size as bank size in a conservative way. Maybe some alignment should make */
	bank->size = offset;
}

static void attribute_realize(object_t *obj);
static void implement_realize(object_t* obj);

/**
 * @brief bank_realize : realize the content of bank
 *
 * @param obj : the object struct about bank
 */
static void bank_realize(object_t *obj) {
	bank_t *bank = (bank_t *)obj;
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

	set_current_obj(obj);
	/* calculate the number of register */
	list_for_each(p, &obj->childs) {
		i++;
	}
	tmp = (object_t *)gdml_zmalloc(sizeof(tmp) * i);
	bank->regs = (object_t **)tmp;
	bank->reg_count = i;
	i = 0;
	/* insert the register object into array from list */
	list_for_each(p, &obj->childs) {
		tmp = list_entry(p, object_t, entry);
		bank->regs[i] = tmp;
		i++;
	}

	//add_object_templates(obj);
	if (obj->node == NULL) return;
	bank_attr_t *attr = obj->node->common.attr;
	/* parse the symbols, parameters and check expressions
	 * that in the bank table */
	//parse_bank(obj->node, obj->symtab->sibling);

	BE_DBG(OBJ, "object name %s\n", obj->name);
	sym = symbol_find(obj->symtab, "register_size", PARAMETER_TYPE);
	if(sym) {
		param = (parameter_attr_t *)sym->attr;
		spec = param->param_spec;
		value = spec->value;
		if(!value->is_const) {
			reg_size = 4;
		} else {
			reg_size = value->u.integer;
		}
	} else {
		reg_size = 4;
	}
	g_register_size = reg_size;
	process_object_names(obj, NULL);

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
	list_for_each(p, &bank->groups) {
		tmp = list_entry(p, object_t, entry);
		group_realize(tmp);
	}
	list_for_each(p, &obj->events) {
		tmp = list_entry(p, object_t, entry);
		event_realize(tmp);
	}
	list_for_each(p, &obj->data) {
		tmp = list_entry(p, object_t, entry);
		data_realize(tmp);
	}
	/*sort the register in user specifid order*/
	bank_calculate_register_offset(obj);
	process_object_templates(obj);
}

static void interface_realize(object_t *obj) {
	process_object_names(obj, NULL);		
}

/**
 * @brief connect_realize : realize the content of connect
 *
 * @param obj : the object of connect
 */
static void connect_realize(object_t *obj) {
	arraydef_attr_t *array_def;
	tree_t *node;
	connect_attr_t *attr;

	//add_object_templates(obj);
	/* parse the connect arraydef expression */
	//parse_connect_attr(obj->node, obj->symtab->parent);
	/* parse elements and calculate expressions that
	 * in  connect table*/
	//parse_connect(obj->node, obj->symtab->sibling);

	node = obj->node;
	attr = node->common.attr;
	process_object_names(obj, attr->arraydef);	
	struct list_head *p;
	object_t *tmp;
	list_for_each(p, &obj->events) {
		tmp = list_entry(p, object_t, entry);
		event_realize(tmp);
	}
	list_for_each(p, &obj->data) {
		tmp = list_entry(p, object_t, entry);
		data_realize(tmp);
	}
	list_for_each(p, &obj->childs) {
		tmp = list_entry(p, object_t, entry);
		interface_realize(tmp);
	}
	process_object_templates(obj);
}

static void data_realize(object_t *obj) {
	cdecl_t *cdecl;
	
	cdecl = obj->node->common.cdecl;
	process_object_names(obj, NULL);
	obj->is_array = (cdecl->common.categ == ARRAY_T);	
#if 0
	if(obj->is_array ) {
		fprintf(stderr, "dataxxx name %s , data type %s\n", obj->name, obj->obj_type);
	}
#endif
}

static offset_info_t *loc;
static void get_group_loc(object_t *obj) {
	struct list_head *p;
	object_t *tmp;
	group_t *gp;
	
	gp = (group_t *)obj;
	if(list_empty(&gp->registers)) {
		p = gp->groups.next;		
		tmp = list_entry(p, object_t, entry);
		get_group_loc(tmp);
	} else {
		p = gp->registers.next;		
		tmp = list_entry(p, object_t, entry);
		dml_register_t *reg = (dml_register_t *)tmp;	
		loc = &reg->offset_info;
	}
}


static void get_group_map_info(object_t *obj) {
	group_t *gp;
	
	gp = (group_t *)obj;	
	get_group_loc(obj);	
	if(loc) {
		gp->offset_info.len = loc->interval[obj->depth - 1];	
		gp->offset_info.offset = loc->offset;
		gp->loc = loc;
	}
}

static void group_realize(object_t *obj) {
	struct list_head *p;
	object_t *tmp;
	group_t *gp;
	group_attr_t *attr;

	gp = (group_t *)obj;
	attr = obj->node->common.attr;
	process_object_names(obj, attr->arraydef);
	list_for_each(p, &gp->registers) {
		tmp = list_entry(p, object_t, entry);
		register_realize(tmp);
	}
	list_for_each(p, &gp->groups) {
		tmp = list_entry(p, object_t, entry);
		group_realize(tmp);
	}
	list_for_each(p, &gp->attributes) {
		tmp = list_entry(p, object_t, entry);
		attribute_realize(tmp);
	}
	list_for_each(p, &obj->events) {
		tmp = list_entry(p, object_t, entry);
		event_realize(tmp);
	}
	list_for_each(p, &obj->data) {
		tmp = list_entry(p, object_t, entry);
		data_realize(tmp);
	}
	get_group_map_info(obj);
}

/**
 * @brief get_allocate_type : get the allocate type with quotes of attribute
 *
 * @param alloc_type : the string of allocate type
 * @param attr : the attribute of attribute
 *
 * @return : string of allocate type
 */
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

/**
 * @brief get_configuration : get the configuration of attribute
 *
 * @param str : the string of configuration with quotes
 *
 * @return : string of configuration
 */
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

/**
 * @brief get_type : get the type of attribute
 *
 * @param str : string of type with quote
 * @param attr : the attribute of attribute
 *
 * @return : string of type
 */
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
	} else if(!strcmp(str, "\"[ii]\"")){
		type = "ii";
		attr->ty = INT_T;
	} else if(!strcmp(str, "\"[iiiiiii]\"")) {
		type = "iiiiiii";
		attr->ty = INT_T;
	} else if(!strcmp(str, "\"[iiiii]\"")) {
		type = "iiiii";
		attr->ty = INT_T;
	}
	return type;
}

#define get_param_spec(attr, spec) \
	do { \
		attr = (parameter_attr_t *)sym->attr; \
		spec = attr->param_spec; \
	} while(0)

/**
 * @brief attribute_realize : realize the content of attribute
 *
 * @param obj : the object of attribute
 */
static void attribute_realize(object_t *obj) {
	tree_t *node;
	symbol_t sym;
	const char* alloc_type;
	parameter_attr_t* attr;
	paramspec_t* spec;
	attribute_t *attr_obj = (attribute_t *)obj;

	//set_current_obj(obj);

	attribute_attr_t* attribute_attr = obj->node->common.attr;
	attribute_attr->attr_obj = attr_obj;
	//add_object_templates(obj);
	/*
	parse_attribute_attr(obj->node, obj->symtab->parent);
	attribute_attr_t* attribute_attr = obj->node->common.attr;
	attribute_attr->attr_obj = attr_obj;
	parse_attribute(obj->node, obj->symtab->sibling);
	*/
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
	sym = defined_symbol(obj->symtab->sibling, "type", PARAMETER_TYPE);
	if (sym) {
		get_param_spec(attr, spec);
		if (spec->value->u.string != NULL) {
			attr_obj->type = get_type(spec->value->u.string, attr_obj);
		} else {
			attr_obj->type = strdup("i");
			attr_obj->ty = INT_T;
		}
	}
	process_object_names(obj, attribute_attr->arraydef);
	struct list_head *p;
	object_t *tmp;
	list_for_each(p, &obj->events) {
		tmp = list_entry(p, object_t, entry);
		event_realize(tmp);
	}
	list_for_each(p, &obj->data) {
		tmp = list_entry(p, object_t, entry);
		data_realize(tmp);
	}
	return;
}

/**
 * @brief implement_realize : realize the content of implement
 *
 * @param obj : the object of implement
 */
static void implement_realize(object_t* obj) {
	set_current_obj(obj);
	parse_implement(obj->node, obj->symtab->sibling);
	process_object_names(obj, NULL);
	return;
}

/**
 * @brief port_realize : realize the content of port
 *
 * @param obj : the object of port
 */
static void port_realize(object_t *obj) {
	struct list_head *p;
	object_t *tmp;
	int i = 0;
	dml_port_t *port = (dml_port_t *)obj;
	port_attr_t *attr;

	set_current_obj(obj);
	//add_object_templates(obj);
	list_for_each(p, &port->implements) {
		i++;
	}
	port = (dml_port_t *)obj;
	port->num = i;
	port->impls = (object_t **)gdml_zmalloc(sizeof(*port->impls) * i);
	parse_port(obj->node, obj->symtab->sibling);
	attr = obj->node->common.attr;
	process_object_names(obj, attr->arraydef);
	parse_port(obj->node, obj->symtab->sibling);
	i = 0;
	list_for_each(p, &port->implements) {
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
	list_for_each(p, &port->implements) {
		tmp = list_entry(p, object_t, entry);
		implement_realize(tmp);
	}
}

/**
 * @brief event_realize :  realize the content of event
 *
 * @param obj : the object of event
 */
static void event_realize(object_t *obj) {
	set_current_obj(obj);
	//parse_event(obj->node, obj->symtab->sibling);
	//add_object_templates(obj);
	struct list_head *p;
	object_t *tmp;
				
	process_object_names(obj, NULL);
	list_for_each(p, &obj->data) {
		tmp = list_entry(p, object_t, entry);
		data_realize(tmp);
	}
}

/**
 * @brief device_realize : realize the content of device and its child object
 *
 * @param dev : struct of device
 */
void device_realize(device_t *dev) {
	struct list_head *p;
	object_t *tmp;
	int i = 0;

	/* calculate the number of bank */
	list_for_each(p, &dev->obj.childs) {
		tmp = list_entry(p, object_t, entry);
		if ((!strcmp(tmp->obj_type, "bank")) && (!strcmp(tmp->name, "__fake_bank"))) {
			continue;
		}
		i++;
	}
	dev->banks = (object_t **)gdml_zmalloc(sizeof(tmp) * i);
	i = 0;
	/* insert the bank object into device array from list */
	list_for_each(p, &dev->obj.childs) {
		tmp = list_entry(p, object_t, entry);
		if ((!strcmp(tmp->obj_type, "bank")) && (!strcmp(tmp->name, "__fake_bank"))) {
			continue;
		}
		dev->banks[i] = tmp;
		i++;
	}
	dev->bank_count = i;
	dev->obj.depth = 0;
	set_current_obj((object_t*)dev);

	/* add default templates "template device" into device,
	 * and insert symbols into table of templates */
	//add_object_templates(&dev->obj);
	/* In parsing, we only insert the object and method symbol
	 * into table, and create the table, but the constant, parameters
	 * and expression are not handled, we should insert the constant,
	 * and parameters into table, and calculate expressions,
	 * as something can be detemined after the object instantiated*/
	//parse_device(dev->obj.node, dev->obj.symtab->sibling);

	process_object_names(&dev->obj, NULL);
	list_for_each(p, &dev->obj.childs) {
		tmp = list_entry(p, object_t, entry);
		bank_realize(tmp);
	}
	list_for_each(p, &dev->attributes) {
		tmp = list_entry(p, object_t, entry);
		attribute_realize(tmp);
	}
	list_for_each(p, &dev->connects) {
		tmp = list_entry(p, object_t, entry);
		connect_realize(tmp);
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
	list_for_each(p, &dev->obj.data) {
		tmp = list_entry(p, object_t, entry);
		data_realize(tmp);
	}
	process_object_templates(&dev->obj);
	create_device_misc(dev);
}

void print_data(object_t *data, int tab_count) {
#if 0
    BE_DBG(OBJ, "data name %s\n",data->name );
    fprintf(stderr, "%sdata name %s\n", tab[tab_count], data->name);
#endif
}

/**
 * @brief print_object : print the object information
 *
 * @param obj : object will be printed
 * @param tab_count : tab count
 */
void print_object(object_t *obj, int tab_count) {
	const char *pos = (const char *)tab[tab_count];
	BE_DBG(OBJ, "%sobject type %s, name %s, symtab %p, sibling %p, symtab parent %p\n", pos, obj->obj_type, obj->name, obj->symtab, obj->symtab->sibling, obj->symtab->parent);

	//printf("%sobject type %s, name %s, symtab %p, sibling %p, symtab parent %p\n", pos, obj->obj_type, obj->name, obj->symtab, obj->symtab->sibling, obj->symtab->parent);

	struct list_head *p;
	object_t *tmp;

	list_for_each(p, &obj->childs) {
		tmp = list_entry(p, object_t, entry);
		print_object(tmp, tab_count + 1);
	}
   	list_for_each(p, &obj->data) {
	   	tmp = list_entry(p, object_t, entry);
	   	print_data(tmp, tab_count + 1);
    }

}

/**
 * @brief print_device_tree : print the object of device and other object that in device
 *
 * @param dev : the struct of device object
 */
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

/**
 * @brief print_rely_temp : print the name of rely templates
 *
 * @param list: list of rely templates
 */
static void print_rely_temp(struct template_list* list) {
	struct template_list* tmp = list;
	int i = 0;
	while (tmp) {
		printf("temp_list[%d]: %s, pre: %s\n", i++, tmp->name, tmp->src_name);
		tmp = tmp->next;
	}
}

/**
 * @brief add_temp_in_list : add new template into template list
 *
 * @param list : list of template
 * @param new : new template will be added into list
 *
 * @return  1 - template has in list
 *			0 - template add into list
 */
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
			return 1;
		}
		pre = tmp;
		tmp = tmp->next;
	}
	pre->next = new;

	return 0;
}

extern struct template_def *create_template_def(const char* name, struct template_list* rely_list);

/**
 * @brief add_rely_templates : add relied templates into rely list
 *
 * @param def : the origianl template
 * @param templates :  the reled teplates
 * @param rely_list : the list of storing relied templates
 */
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

/**
 * @brief create_template_def : process the block of template and its rely templates
 *
 * @param name : name of template
 * @param list : list of relied templates of one template
 *
 * @return : pointer to template struct
 */
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
				statement->common.type == CDECL_TYPE || statement->common.type == DATA_TYPE) {
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

#define not_port_data_obj(obj) \
		strcmp(obj->obj_type, "port") && \
		strcmp(obj->obj_type, "data") \

/**
 * @brief obj_templates_list :  add all templates of object into list
 *
 * @param obj : the object struct
 * @param name : the name of object type
 */
static void obj_templates_list(object_t* obj, const char* name) {
	assert(obj != NULL);
	assert(name != NULL);
	struct template_table* templates = NULL;
	symtab_t table = obj->symtab->sibling;

	if (not_port_data_obj(obj)) {
		create_template_name(obj, obj->obj_type);
	} else {
		return;
	}
	if (table) {
		templates = table->template_table;
		while (templates) {
			create_template_name(obj, templates->template_name);
			templates = templates->next;
		}
	}

	return;
}

/**
 * @brief add_default_template : add default template into object
 *
 * @param obj : the object struct
 */
static void add_default_template(object_t *obj){
	symtab_t table;

	table = obj->symtab->sibling;
	if(table) {
		BE_DBG(OBJ, "add default template %s\n", obj->obj_type);
		/* the port and data object do not have default template */
		if (not_port_data_obj(obj))
			add_template_to_table(table, obj->obj_type, 1);
	}
	obj_templates_list(obj, obj->obj_type);
}

/**
 * @brief process_device_template : process the default parameters of device
 *
 * @param obj : the object of device
 */
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

/**
 * @brief process_bank_template : process the default parameters about bank
 *
 * @param obj : the bank object
 */
static void process_bank_template(object_t *obj){
	symtab_t table = obj->symtab;
	bank_t *bank = (bank_t *)obj;
	int i;
	param_value_t tmp;
	dml_register_t *reg;
	int undefined = 0;
	int defined = 0;
	int j;

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
	for(i = 0, j = 0; i < bank->reg_count; i++) {
		reg = (dml_register_t *)bank->regs[i];
		if(!reg->is_undefined ) {
			tmp.type = PARAM_TYPE_REF;
			tmp.u.ref = bank->regs[i];
			val->u.list.vector[j] = tmp;
			j++;
		}
	}
	symbol_insert(table, "mapped_registers", PARAMETER_TYPE, val);
	val = gdml_zmalloc(sizeof(*val));
	val->type = PARAM_TYPE_LIST;
	val->u.list.size = 0;
	/*
	if(undefined > 0) {
		val->u.list.vector = gdml_zmalloc(sizeof(*val) * undefined);	
		for(i = 0, j = 0; i < bank->reg_count; i++) {
			reg = (dml_register_t *)bank->regs[i];
			if(reg->is_undefined) {
				tmp.type = PARAM_TYPE_REF;
				tmp.u.ref = bank->regs[i];
				val->u.list.vector[j++] = tmp;
			}
		}
	}*/

	symbol_insert(table, "unmapped_registers", PARAMETER_TYPE, val);
	val = gdml_zmalloc(sizeof(*val));
	val->type = PARAM_TYPE_LIST;
	val->u.list.size = 0;
	symbol_insert(table, "numbered_registers", PARAMETER_TYPE, val);

}

/**
 * @brief process_register_template : process the default parameters about register
 *
 * @param obj : the object of register
 */
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
	if(!symbol_defined(table->sibling, "hard_reset_value")) {
		val = gdml_zmalloc(sizeof(*val));
		val->type = PARAM_TYPE_INT;
		val->u.integer = 0;
		symbol_insert(table, "hard_reset_value", PARAMETER_TYPE, val);
	}

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

/**
 * @brief process_field_template : process the default parameters about field
 *
 * @param obj : the object of field
 */
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

static void process_connect_template(object_t *obj) {

	symtab_t table = obj->symtab;
	param_value_t *val;

	val = (param_value_t *)gdml_zmalloc(sizeof(*val));
	val->type = PARAM_TYPE_REF;
	val->u.ref = obj;
	symbol_insert(table, "obj", PARAMETER_TYPE, val);
}
/**
 * @brief free_rely_temp_list : free the list of relied templates
 *
 * @param list : head of reliled templates
 */
static void free_rely_temp_list(struct template_list* list) {
	struct template_list* tmp = list;
	struct template_list* old = list;
	while (tmp) {
		tmp = tmp->next;
		free((void*)(old->src_name));
		free((void*)(old->name));
		old = tmp;
	}

	return;
}

/*process parameters including auto parameters */

/**
 * @brief process_object_template : process all templates of object
 *
 * @param obj : the object struct
 * @param name : the name of template
 */
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

/**
 * @brief process_basic_template : process some common parameters about all object
 *
 * @param obj : the object struct
 */
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

static void object_none_temp_fn(object_t *obj) {
	BE_DBG(TEMPLATE, "some error happend\n");
}

static void (*specific_temp_fn[])(object_t *obj) = {
	[Obj_Type_None] = object_none_temp_fn,
	[Obj_Type_Device] = process_device_template,
	[Obj_Type_Bank] = process_bank_template,
	[Obj_Type_Register] = process_register_template,
	[Obj_Type_Field] = process_field_template,
	[Obj_Type_Connect] = process_connect_template,
};

/**
 * @brief process_default_template : process the default template of object
 * as the default template is added into object in system
 *
 * @param obj : the object struct
 */
static void process_default_template(object_t *obj) {
	process_basic_template(obj);
	specific_temp_fn[obj->encoding](obj);
	BE_DBG(TEMPLATE, "start ........\nobject Name %s symbols\n", obj->name);
	//print_all_symbol(obj->symtab);
	BE_DBG(TEMPLATE, "end .......");
}

/**
 * @brief process_object_templates : process the teplates of object
 *
 * @param obj : the object struct
 */
void process_object_templates(object_t *obj){
	struct template_name *name;
	struct list_head *p;

	process_default_template(obj);
}

/**
 * @brief parse_rely_templates : entry to parse all relied templates of one object
 *
 * @param obj : the object struct
 */
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

/**
 * @brief add_object_templates : add default template into object and
 * parse the block of templates in object
 *
 * @param obj : the object struct
 */
static void add_object_templates(object_t *obj){
	add_default_template(obj);
	parse_rely_templates(obj);
}

#include <unistd.h>
int method_to_generate = 0;
extern symbol_t get_symbol_from_templates(symtab_t table, const char* name, int type);

/**
 * @brief add_object_method : add method into object method list
 *
 * @param obj : object that contains method
 * @param name : name of method
 */
void add_object_method(object_t *obj,const char *name){
	struct method_name *m;
	struct list_head *p;
	symbol_t sym;
	method_attr_t *attr;

	if(!strcmp(obj->obj_type,"bank") && (!strcmp(name,"read_access") || !strcmp(name,"write_access"))){
		return;
	}
	if ((!strcmp(obj->obj_type, "bank")) && (!strcmp(obj->name, "__fake_bank"))) {
		return;
	}

	list_for_each(p,&obj->method_generated){
		m = list_entry(p,struct method_name,entry);
		if(!strcmp(m->name,name)) {
			return;
		}
	}

	list_for_each(p,&obj->methods){
		m = list_entry(p,struct method_name,entry);
		if(!strcmp(m->name,name)) {
			return;
		}
	}
	m = gdml_zmalloc(sizeof(*m));
	m->name = strdup(name);
	INIT_LIST_HEAD(&m->entry);
	BE_DBG(OBJ, "add object %s, name, %s\n", obj->name, name);
	
	sym = object_symbol_find(obj->symtab, name, METHOD_TYPE);
	//sym = (sym == NULL) ? get_symbol_from_templates(obj->symtab->sibling, name, METHOD_TYPE) : sym;
	if(!sym) {
		fprintf(stderr,"object %s has no method %s\n",obj->name, name);
		exit(-1);
	}
	else {
		attr = sym->attr;
		m->method = attr->common.node;
	}
	method_to_generate++;
	list_add_tail(&m->entry, &obj->methods);
}

/**
 * @brief object_method_generated :  check one method is generate or not
 *
 * @param obj : object that contains method
 * @param m : the method
 *
 * @return : 1 - method is generated
 *			0 - method is not generated
 */
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

/**
 * @brief add_object_generated_method :  add the generated method
 *
 * @param obj : object that have generated its method
 */
void add_object_generated_method(object_t *obj){
	struct list_head *p;
	int i = 0;

	list_for_each(p,&obj->methods){
		i++;
	}

	method_to_generate -= i;
	list_splice_init(&obj->methods, &obj->method_generated);
}

/**
 * @brief find_index_obj : find object with index
 *
 * @param obj : object to be fined
 * @param index : index of object
 *
 * @return :  object with index
 */
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

/**
 * @brief get_device_obj : get device object
 *
 * @return : object of device
 */
object_t* get_device_obj() {
	return DEV;
}
