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

object_t *OBJ;
object_t *DEV;

object_t OBJECT = {.name = "obj",.obj_type = "object"};

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
	if(OBJ && (!strcmp(obj->obj_type,"register") || !strcmp(obj->obj_type,"field"))){
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

static void init_object(object_t *obj, const char *name, const char *type, tree_t *node) {	
	int i;
	obj->name = name;
	obj->parent = OBJ;
	obj->obj_type = type;
	obj->node = node;
	struct list_head *p = &obj->childs;
	for(i = 0; i < 7;i++,p++){
		INIT_LIST_HEAD(p);
	}
	if(OBJ){
		list_add_tail(&obj->entry,&OBJ->childs);
	}		
	process_object_names(obj);
}

object_t *create_device_object(tree_t *t){
	device_t *dev =  (device_t *)gdml_zmalloc(sizeof(*dev));
	if(OBJ)
		printf("object exist\n");
	init_object(&dev->obj,t->common.name,"device",t);
	int i ;
	struct list_head *p = &dev->obj.childs;
	for(i = 0; i < 7;i++,p++){
		INIT_LIST_HEAD(p);
	}
	return &dev->obj;
}

static void create_field_object(tree_t *t){
	if(!OBJ || strcmp(OBJ->obj_type,"field"))
		printf("field %s not in right place\n",t->common.name);
	field_t *fld = gdml_zmalloc(sizeof(*fld));
	init_object(&fld->obj,t->common.name,"field",t);
}

static void create_field_obj(symtab_t symtab){
	symbol_t sym = symbol_find(symtab,"FIELD",FIELD_TYPE);
	object_t *obj = OBJ;
	while(sym){
		create_field_object((tree_t *)sym->attr);
		OBJ = obj;
	};
}

static void create_register_object(tree_t *t){
	if(!OBJ || strcmp(OBJ->obj_type,"register"))
		printf("register %s not in right place\n",t->common.name);
	dml_register_t *reg = (dml_register_t *)gdml_zmalloc(sizeof(*reg));
	init_object(&reg->obj,t->common.name,"register",t);
	OBJ = &reg->obj;
	create_field_obj(reg->obj.symtab);
}

static void create_reg_obj(struct symtab *symtab){
	symbol_t sym = symbol_find(symtab,"REGISTER",REGISTER_TYPE);
	object_t *obj = OBJ;
	while(sym){
		create_register_object((tree_t *)sym->attr);
		OBJ = obj;
		sym = sym->lnext;
	};
}

static void create_bank_object(tree_t *t){
	if(!OBJ || strcmp(OBJ->obj_type,"bank"))
		printf("bank %s not in right place\n",t->common.name);
	bank_t *bank = gdml_zmalloc(sizeof(*bank));
	init_object(&bank->obj,t->common.name,"bank",t);		
	OBJ = &bank->obj;
	create_reg_obj(bank->obj.symtab);
}

static void create_bank_obj(struct symtab *symtab){
	symbol_t sym = symbol_find(symtab,"BANK",BANK_TYPE);
	object_t *obj = OBJ;
	while(sym){
		create_bank_object((tree_t *)(sym->attr));
		OBJ = obj;
		sym = sym->lnext;
	};
}
extern symtab_t root_table;

device_t *create_device_tree(tree_t  *root){
	/*need a type search*/
	print_all_symbol(root_table);
	symbol_list_t *dev_list = symbol_list_find(root_table, DEVICE_TYPE);
	if(!dev_list || dev_list->next) {
		printf("device not correct\n");
		exit(-1);
	}
	symbol_t sym;
	OBJ = create_device_object((tree_t *)sym->attr);
	DEV = OBJ;
	//create_bank_obj((struct tree_device *)(sym->attr)->symtab);
	return (device_t *)DEV;
}

int in_template = 0;
static LIST_HEAD(templates); 

static void add_template(template_def_t  *def){
	struct list_head *p;
	struct template_def *tp;
	list_for_each(p,&templates){
		tp = list_entry(p,struct template_def,entry);
		if(!strcmp(tp->name, def->name)){
			printf("template %s redifinition,previous definition is here",def->name);
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
			printf("canot define template in template definition\n");
		if(strcmp(OBJ->obj_type,"device"))
			printf("template definition must at top level\n");
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


static void add_device_default_template(object_t *obj){
	struct template_name *tn = gdml_zmalloc(sizeof(*tn));
	struct template_def *def;

	def = lookup_template("device");
	if(!def)
		printf("no default device template\n");
	tn->name = "device";
	tn->def = def;
	INIT_LIST_HEAD(&tn->entry);
	list_add_tail(&obj->templates,&tn->entry);
}

static void add_bank_default_template(object_t *obj){
	struct template_name *tn = gdml_zmalloc(sizeof(*tn));
	struct template_def *def;
	def = lookup_template("bank");
	if(!def)
		printf("no default bank template\n");
	tn->name = "bank";
	tn->def = def;
	INIT_LIST_HEAD(&tn->entry);
	list_add_tail(&obj->templates,&tn->entry);
}

static void add_register_default_template(object_t *obj){
	struct template_name *tn = gdml_zmalloc(sizeof(*tn));
	struct template_def *def;
	def = lookup_template("register");
	if(!def)
		printf("no default register template\n");
	tn->name = "register";
	tn->def = def;
	INIT_LIST_HEAD(&tn->entry);
	list_add_tail(&obj->templates,&tn->entry);
}

static void add_field_default_template(object_t *obj){
	struct template_name *tn = gdml_zmalloc(sizeof(*tn));
	struct template_def *def;
	def = lookup_template("field");
	if(!def)
		printf("no default field template\n");
	tn->name = "field";
	tn->def = def;
	INIT_LIST_HEAD(&tn->entry);
	list_add_tail(&obj->templates,&tn->entry);
}

static void add_default_template(object_t *obj){
	if(!strcmp(obj->obj_type,"device")){
		add_device_default_template(obj);
	}else if(!strcmp(obj->obj_type,"bank")){
		add_bank_default_template(obj);
	}else if(!strcmp(obj->obj_type,"register")){
		add_register_default_template(obj);
	}else if(!strcmp(obj->obj_type,"field")){
		add_field_default_template(obj);
	}else{
		//do nothing
	}
}

static void process_auto_parameter(object_t *obj,struct template_def *def,symbol_t *sym){

}

static void process_device_template(object_t *obj,struct template_def *def){
	/*process parameters including auto parameters */
	/*foreach parameters in this template solv it;for auto parameters need special hanlding*/
}

static void process_object_template(object_t *obj,struct template_def *def){
	/*process parameters including auto parameters */
}

static void process_object_templates(object_t *obj){
	struct template_name *name;
	struct list_head *p;
	list_for_each(p,&obj->templates){
		name = list_entry(p,struct template_name,entry);
		if(name)
			process_object_template(obj,name->def);
	}
}

void add_object_templates(object_t *obj,tree_t *t){
	tree_t *it = t;
	struct tree_ident *ident;
	struct template_def *def;
	struct template_name *temp;
	add_default_template(obj);
	while(it){
		ident = (struct tree_ident *)it;
		def = lookup_template(ident->str);
		if(!def)
			printf("template %s,not defined\n",ident->str);
		temp = gdml_zmalloc(sizeof(*temp));
		temp->name = ident->str;
		temp->def = def;
		INIT_LIST_HEAD(&temp->entry);
		list_add_tail(&obj->templates,&temp->entry); 
		it = it->common.sibling;
	}
}


int method_to_generate = 0;
void add_object_method(object_t *obj,const char *name){
	struct method_name *m;
	struct list_head *p;
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
	//m->method = (tree_t)symbol_find(obj->symtab,name,METHOD_TYPE);
	if(!m->method)
		printf("object %s no method %s\n",obj->name,name);
	method_to_generate++;
	list_add_tail(&obj->methods,&m->entry);
}

int object_method_generated(object_t *obj, struct method_name *m){
	struct list_head *p;
	struct method_name *t;

	list_for_each(p,&obj->method_generated){
		t = list_entry(p,struct method_name,entry);
		if(!strcmp(t->name,m->name))
			return 1;
	}
	return 0;
}

void add_object_generated_method(object_t *obj,struct method_name *m){
	struct list_head *p;
	struct method_name *t;
	list_for_each(p,&obj->method_generated){
		t = list_entry(p,struct method_name,entry);
		if(!strcmp(t->name,m->name))
			return;
	}
	method_to_generate--;
	list_add_tail(&obj->method_generated,&m->entry);
}
