/*
 * gen_common.c: 
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

/*hardcoded for bank read/write accesses*/
#include "gen_once_noplatform.h"
#include "gen_implement.h"
#include "gen_port.h"
#include "gen_event.h"

extern object_t *DEV;
static int list_count = LIST_SZ;
/* just a try*/
#define MAGIC_NUM 100

static const char *get_output_fmt(int num) {
	const char *fmt;

	if(num <= MAGIC_NUM) {
		fmt = "%d";
	} else {
		fmt = "0x%x";
	}
	return fmt;
} 

/**
 * @brief gen_bank_read_access : generated the code of bank read access method
 *
 * @param b : the object of bank
 * @param list : the list of classified register array
 * @param f : c file to be generated
 */
static void gen_bank_read_access(bank_t *b, reg_array_t *list, FILE *f){
	int register_size = b->register_size;
	int offset;
	int size;
	object_t *t;
	struct list_head *p;
	int exec_index = get_local_index();
	int param = get_local_index();
	int i;
	int ret_index;
	int local_index;
	dml_register_t *reg;
	reg_array_t *e;
	reg_item_t *item;
	int len;
	int tmp;

	fprintf(f,"\nstatic bool\n");
	fprintf(f,"_DML_M_%s__read_access(%s_t *_dev, generic_transaction_t *memop, physical_address_t offset, physical_address_t size, bool *success, uint64 *readvalue)\n",b->obj.qname,DEV->name);
	fprintf(f, "{\n");
	fprintf(f,"\t{\n");
	fprintf(f,"\t\t{\n");
	fprintf(f,"\t\t\tbool v%d_exec = 0;\n",exec_index);
	fprintf(f,"\t\t\t{\n");
	fprintf(f,"\t\t\t\t{\n");	
	fprintf(f,"\t\t\t\t\tuint64 v%d_offset = offset;\n",param);	
	fprintf(f,"\t\t\t\t\tint64 v%d_size = size;\n",param);
	fprintf(f,"\t\t\t\t\tint64 v%d_size2;\n",param);
	fprintf(f,"\t\t\t\t\t*readvalue = 0;\n");
	
	/*genenrate register array access function*/
	for(i = 1; i < list_count; i++) {
		e = &list[i];
		if(!e->size) {
			break;
		}
		len = register_size * e->size;
		//fprintf(f,"\t\t\t\t\tswitch(v%d_offset / %d )\n",param, len);
		fprintf(f, "\t\t\t\t\t{\n");
		list_for_each(p, &e->list) {
			item = list_entry(p, reg_item_t, entry);
			reg = (dml_register_t *)item->obj;
			ret_index = get_local_index();
			add_object_method(item->obj, "read_access");
			tmp = reg->offset/len;
			//if(tmp < MAGIC_NUM) {
			//	fprintf(f, "\t\t\t\t\t\tcase %d:\n", tmp);
			//} else {
			//	fprintf(f, "\t\t\t\t\t\tcase 0x%x:\n", tmp);
			//}
			fprintf(f,"\t\t\t\t\t\t\tif(v%d_offset >= 0x%x && v%d_offset < 0x%x)\n", param, reg->offset, param, reg->offset + reg->array_size * reg->interval); // temp
			fprintf(f,"\t\t\t\t\t\t\t{\n");
			fprintf(f,"\t\t\t\t\t\t\t\tint _idx0;\n");
			fprintf(f,"\t\t\t\t\t\t\t\t_idx0 = (v%d_offset - 0x%x) / %d;\n", param, reg->offset, reg->interval);
			fprintf(f,"\t\t\t\t\t\t\t\tif(((_idx0 >= 0) && (_idx0 < %d)) && ((v%d_offset - _idx0 * %d - 0x%x) == 0))\n", reg->array_size, param, reg->interval, reg->offset);
			fprintf(f,"\t\t\t\t\t\t\t\t{\n");
			fprintf(f,"\t\t\t\t\t\t\t\t\tuint64 v%d_ret_value;\n", ret_index);
			fprintf(f,"\t\t\t\t\t\t\t\t\tv%d_size2 = %d;\n", param, reg->size);
			fprintf(f,"\t\t\t\t\t\t\t\t\tv%d_exec = _DML_M_%s__read_access(_dev, _idx0, memop, 31, 0, &v%d_ret_value);\n", exec_index, item->obj->qname, ret_index);
			//fprintf(f,"\t\t\t\t\t\t\t\t\tprintf(\"-- %s: read %s[%%d] -> value 0x%%lx\\n\", _idx0, v%d_ret_value);\n", DEV->name, item->obj->a_name, ret_index);
			fprintf(f,"\t\t\t\t\t\t\t\t\tif(v%d_exec){\n", exec_index);
			fprintf(f,"\t\t\t\t\t\t\t\t\t\treturn 1;\n");
			fprintf(f,"\t\t\t\t\t\t\t\t\t}\n");
			fprintf(f,"\t\t\t\t\t\t\t\t\t*readvalue = v%d_ret_value;\n", ret_index);
			fprintf(f,"\t\t\t\t\t\t\t\t\tgoto found;\n");
			fprintf(f,"\t\t\t\t\t\t\t\t}\n");
			//fprintf(f,"\t\t\t\t\t\t\t\tbreak;\n");
			fprintf(f,"\t\t\t\t\t\t\t}\n");

		}
		fprintf(f, "\t\t\t\t\t}\n");
	}
    /*genenrate single register access*/
	fprintf(f,"\t\t\t\t\tswitch(v%d_offset / %d)\n",param, register_size);
	fprintf(f, "\t\t\t\t\t{\n");
	offset = 0;
	size = 0;
	int bound = 0;
	list_for_each(p, &list[0].list){
		item = list_entry(p, reg_item_t, entry);
		t = item->obj;
		reg = (dml_register_t *)t;
		local_index = get_local_index();
		offset = reg->offset;
		bound = reg->offset + reg->size;
		tmp = reg->offset/register_size;
		if(tmp < MAGIC_NUM) {
			fprintf(f,"\t\t\t\t\t\tcase %d:\n", tmp);
		} else {
			fprintf(f,"\t\t\t\t\t\tcase 0x%x:\n", tmp);
		}
		fprintf(f,"\t\t\t\t\t\t\t{\n");
		fprintf(f,"\t\t\t\t\t\t\t\tuint8 v%d_lsb;\n",local_index);
		fprintf(f,"\t\t\t\t\t\t\t\tuint8 v%d_msb;\n",local_index);
		fprintf(f,"\t\t\t\t\t\t\t\tv%d_size2 = v%d_size < %d - v%d_offset ? v%d_size : %d - v%d_offset;\n",param,param,bound,param,param,bound,param);
		fprintf(f,"\t\t\t\t\t\t\t\tv%d_lsb = (v%d_offset - %d) * 8;\n",local_index,param,offset);
		fprintf(f,"\t\t\t\t\t\t\t\tv%d_msb = v%d_lsb + v%d_size2 * 8;\n",local_index,local_index,param);
		fprintf(f,"\t\t\t\t\t\t\t\t{\n");
		ret_index = get_local_index();
		fprintf(f,"\t\t\t\t\t\t\t\t\tuint64 v%d_ret_value  = 0;\n",ret_index);	
		fprintf(f,"\t\t\t\t\t\t\t\t\t{\n");
		add_object_method(t,"read_access");	
		fprintf(f,"\t\t\t\t\t\t\t\t\t\tv%d_exec = _DML_M_%s__read_access(_dev, memop, v%d_msb, v%d_lsb, &v%d_ret_value);\n",exec_index,t->qname,local_index,local_index,ret_index);	
		//fprintf(f,"\t\t\t\t\t\t\t\t\t\tprintf(\"-- %s: read %s -> value 0x%%lx\\n\", v%d_ret_value);\n", DEV->name, item->obj->a_name, ret_index);
		fprintf(f,"\t\t\t\t\t\t\t\t\t\tif(v%d_exec)\n",exec_index);		
		fprintf(f,"\t\t\t\t\t\t\t\t\t\t\treturn 1;\n");
		
		fprintf(f,"\t\t\t\t\t\t\t\t\t}\n");
		
		fprintf(f,"\t\t\t\t\t\t\t\t\t*readvalue = v%d_ret_value;\n",ret_index);	

		fprintf(f,"\t\t\t\t\t\t\t\t}\n");

		fprintf(f,"\t\t\t\t\t\t\t\tgoto found;\n");

		fprintf(f,"\t\t\t\t\t\t\t}\n");
	}
	int exit_index = get_exit_index();	
	fprintf(f,"\t\t\t\t\t}\n");
	fprintf(f,"\t\t\t\t\t*success = 0;\n");
	fprintf(f,"\t\t\t\t\tgoto exit%d;\n",exit_index);
	fprintf(f,"\t\t\t\tfound:\n");
	fprintf(f,"\t\t\t\t\t*success = v%d_size == v%d_size2;\n", param, param);
	fprintf(f,"\t\t\t\t}\n");
	fprintf(f,"\t\t\texit%d:;\n",exit_index);
	fprintf(f,"\t\t\t}\n");
	fprintf(f,"\t\t}\n");
	fprintf(f,"\t\treturn 0;\n");
	fprintf(f,"\t}\n");
	fprintf(f,"}\n");
}		

/**
 * @brief gen_bank_write_access : generate the code of bank write access method
 *
 * @param b : the object of bank
 * @param list : the classified register array with list format
 * @param f : c file to be generated
 */
static void gen_bank_write_access(bank_t *b, reg_array_t *list, FILE *f){
	int register_size = b->register_size;
	int offset;
	int size;
	object_t *t;
	struct list_head *p;
	int exec_index = get_local_index();
	int param = get_local_index();
	reg_array_t *e;
	reg_item_t *item;
	int len;
	int tmp;
	dml_register_t *reg;
	int i;
	int ret_index;
	int local_index;

	fprintf(f,"\nstatic bool \n");
	fprintf(f,"_DML_M_%s__write_access(%s_t *_dev, generic_transaction_t *memop, physical_address_t offset, physical_address_t size, uint64 writevalue, bool *success)\n",b->obj.qname,DEV->name);
	fprintf(f, "{\n");
	fprintf(f,"\t{\n");
	fprintf(f,"\t\t{\n");
	fprintf(f,"\t\t\tbool v%d_exec = 0;\n",exec_index);
	fprintf(f,"\t\t\t{\n");
	fprintf(f,"\t\t\t\t{\n");	
	fprintf(f,"\t\t\t\t\tuint64 v%d_offset = offset;\n",param);	
	fprintf(f,"\t\t\t\t\tint64 v%d_size = size;\n",param);
	fprintf(f,"\t\t\t\t\tint64 v%d_size2;\n",param);

	/*genenrate register array access function*/
	for(i = 1; i < list_count; i++) {
		e = &list[i];
		if(!e->size) {
			break;
		}
		len = register_size * e->size;
		//fprintf(f,"\t\t\t\t\tswitch(v%d_offset / %d )\n",param, len);
		fprintf(f, "\t\t\t\t\t{\n");
		list_for_each(p, &e->list) {
			item = list_entry(p, reg_item_t, entry);
			reg = (dml_register_t *)item->obj;
			ret_index = get_local_index();
			add_object_method(item->obj, "write_access");
			tmp = reg->offset/len;
			//if(tmp < MAGIC_NUM) {
			//	fprintf(f, "\t\t\t\t\t\tcase %d:\n", tmp);
			//} else {
			//	fprintf(f, "\t\t\t\t\t\tcase 0x%x:\n", tmp);
			//}
			fprintf(f,"\t\t\t\t\t\t\tif(v%d_offset >= 0x%x && v%d_offset < 0x%x)\n", param, reg->offset, param, reg->offset + reg->array_size * reg->interval); // temp
			fprintf(f,"\t\t\t\t\t\t\t{\n");
			fprintf(f,"\t\t\t\t\t\t\t\tint _idx0;\n");
			fprintf(f,"\t\t\t\t\t\t\t\t_idx0 = (v%d_offset - 0x%x) / %d;\n", param, reg->offset, reg->interval);
			fprintf(f,"\t\t\t\t\t\t\t\tif(((_idx0 >= 0) && (_idx0 < %d)) && ((v%d_offset - _idx0 * %d - 0x%x) == 0))\n", reg->array_size, param, reg->interval, reg->offset);
			fprintf(f,"\t\t\t\t\t\t\t\t{\n");
			fprintf(f,"\t\t\t\t\t\t\t\tv%d_size2 = %d;\n", param, reg->size);
			//fprintf(f,"\t\t\t\t\t\t\t\t\tprintf(\"-- %s: write %s[%%d] <- value 0x%%lx\\n\", _idx0, writevalue);\n", DEV->name, item->obj->a_name);
			fprintf(f,"\t\t\t\t\t\t\t\t\tv%d_exec = _DML_M_%s__write_access(_dev, _idx0, memop, 31, 0, writevalue);\n", exec_index, item->obj->qname);
			fprintf(f,"\t\t\t\t\t\t\t\t\tif(v%d_exec){\n", exec_index);
			fprintf(f,"\t\t\t\t\t\t\t\t\t\treturn 1;\n");
			fprintf(f,"\t\t\t\t\t\t\t\t\t}\n");
			fprintf(f,"\t\t\t\t\t\t\t\t\tgoto found;\n");
			fprintf(f,"\t\t\t\t\t\t\t\t}\n");
			//fprintf(f,"\t\t\t\t\t\t\t\tbreak;\n");
			fprintf(f,"\t\t\t\t\t\t\t}\n");

		}
		fprintf(f, "\t\t\t\t\t}\n");
	}
	
	fprintf(f,"\t\t\t\t\tswitch(v%d_offset / %d )\n",param,register_size);
	fprintf(f, "\t\t\t\t\t{\n");
	offset = 0;
	size = 0;
	int bound = 0;
	list_for_each(p, &list[0].list){
		item = list_entry(p, reg_item_t, entry);
		t = item->obj;
		reg = (dml_register_t *)t;
		local_index = get_local_index();
		offset = reg->offset;
		bound = offset + reg->size;
		tmp = reg->offset/reg->size;
		if(tmp < MAGIC_NUM) {
			fprintf(f, "\t\t\t\t\t\tcase %d:\n", tmp);
		} else {
			fprintf(f, "\t\t\t\t\t\tcase 0x%x:\n", tmp);
		}
		fprintf(f,"\t\t\t\t\t\t\t{\n");
		fprintf(f,"\t\t\t\t\t\t\t\tuint8 v%d_lsb;\n",local_index);
		fprintf(f,"\t\t\t\t\t\t\t\tuint8 v%d_msb;\n",local_index);
		fprintf(f,"\t\t\t\t\t\t\t\tv%d_size2 = v%d_size < %d - v%d_offset ? v%d_size : %d - v%d_offset;\n",param,param,bound,param,param,bound,param);
		fprintf(f,"\t\t\t\t\t\t\t\tv%d_lsb = (v%d_offset - %d) * 8;\n",local_index,param,offset);
		fprintf(f,"\t\t\t\t\t\t\t\tv%d_msb = v%d_lsb + v%d_size2 * 8;\n",local_index,local_index,param);
		offset += register_size;
		fprintf(f,"\t\t\t\t\t\t\t\t{\n");
		ret_index = get_local_index();
		add_object_method(t,"write_access");
		//fprintf(f,"\t\t\t\t\t\t\t\t\tprintf(\"-- %s: write %s <- value 0x%%lx\\n\", writevalue);\n", DEV->name, item->obj->a_name);
		fprintf(f,"\t\t\t\t\t\t\t\t\tv%d_exec = _DML_M_%s__write_access(_dev, memop, v%d_msb, v%d_lsb, writevalue);\n",exec_index,t->qname,local_index,local_index);	
		fprintf(f,"\t\t\t\t\t\t\t\t\tif(v%d_exec)\n",exec_index);		
		fprintf(f,"\t\t\t\t\t\t\t\t\t\treturn 1;\n");	
		fprintf(f,"\t\t\t\t\t\t\t\t}\n");
		fprintf(f,"\t\t\t\t\t\t\t\tgoto found;\n");
		fprintf(f,"\t\t\t\t\t\t\t}\n");
	}
	int exit_index = get_exit_index();	
	fprintf(f,"\t\t\t\t\t}\n");
	fprintf(f,"\t\t\t\t\t*success = 0;\n");
	fprintf(f,"\t\t\t\t\tgoto exit%d;\n",exit_index);
	fprintf(f,"\t\t\t\tfound:\n");
	fprintf(f,"\t\t\t\t\t*success = v%d_size == v%d_size2;\n", param, param);
	fprintf(f,"\t\t\t\t}\n");
	fprintf(f,"\t\t\texit%d:;\n",exit_index);
	fprintf(f,"\t\t\t}\n");
	fprintf(f,"\t\t}\n");
	fprintf(f,"\t\treturn 0;\n");
	fprintf(f,"\t}\n");
	fprintf(f,"}\n");
}

/**
 * @brief gen_hard_reset : generate the hard reset function of device
 *
 * @param dev : the object of device
 * @param f : c file to be generated
 */
static void  gen_hard_reset(device_t *dev, FILE *f) {
    const char *name = dev->obj.name;
    int index = get_local_index();

    add_object_method(&dev->obj, "hard_reset");
    fprintf(f, "\nvoid %s_hard_reset(%s_t *obj) {\n", name, name);
    fprintf(f, "\tbool v%d_exec;\n", index);
    fprintf(f, "\tUNUSED(v%d_exec);\n", index);
    fprintf(f, "\n");
    fprintf(f, "\tv%d_exec = _DML_M_hard_reset(obj);\n",index);
    fprintf(f, "}\n");
}

/**
 * @brief gen_soft_reset : generate the soft reset function of device
 *
 * @param dev : the object of device
 * @param f : c file to be generated
 */
static void  gen_soft_reset(device_t *dev, FILE *f) {
    const char *name = dev->obj.name;
    int index = get_local_index();

    add_object_method(&dev->obj, "soft_reset");
    fprintf(f, "\nvoid %s_soft_reset(%s_t *obj) {\n", name, name);
    fprintf(f, "\tbool v%d_exec;\n", index);
    fprintf(f, "\tUNUSED(v%d_exec);\n", index);
    fprintf(f, "\n");
    fprintf(f, "\tv%d_exec = _DML_M_soft_reset(obj);\n", index);
    fprintf(f, "}\n");
}

/**
 * @brief gen_device_reset : generate the reset function of device
 *
 * @param dev : the object of device
 * @param f : c file to be generated
 */
static void gen_device_reset(device_t *dev, FILE *f) {
    gen_hard_reset(dev, f);
    gen_soft_reset(dev, f);
}

/**
 * @brief new_reg_list : creat a list to store registers for sorting
 *
 * @return : the list of register
 */
static reg_array_t *new_reg_list(void) {
	int i = 0;
	reg_array_t *e;

	reg_array_t *list = gdml_zmalloc(sizeof(*list) * list_count);
	e = &list[0];
	for(; i < list_count; i++) {
		INIT_LIST_HEAD(&e->list);
		e++;
	}
	return list;
}

/**
 * @brief reg_array_free : free the list of register
 *
 * @param list : head of register list
 */
static void reg_array_free(reg_array_t *list) {
	reg_array_t *e;
	reg_item_t *item;
	int i;
	struct list_head *head;
	struct list_head *p;

	for(i = 0; i < list_count; i++) {
		e = &list[i];
		if(!e->size) {
			break;
		}
		head = &e->list;
		p = head->next;
		while(p != head) {
			item = list_entry(p, reg_item_t, entry);
			p = p->next;
			free(item);
		}
	}
	free(list);
	list_count = LIST_SZ;
}

/**
 * @brief find_slot : get the index of array that classify register
 *
 * @param list : the classify register list
 * @param base : the base of array register
 * @param size : the size of array register
 *
 * @return : the index of array register in the classify register array
 */
static int find_slot(reg_array_t *list, int base,  int size) {
	int i = 1;
	reg_array_t *e;

	for(; i < list_count; i++) {
		e = &list[i];
		if(e->size == size) {
			return i;
		}
	} 
	for(i = 1; i < list_count; i++) {
		e = &list[i];
		if(!e->size) {
			e->size = size;
			return i;
		}
	}
	if(i >= list_count) {
		list_count *= 2;
		list = realloc(list, list_count * sizeof(*e));
	}
	for(; i < list_count; i++) {
		e = &list[i];
		e->size = 0;
		INIT_LIST_HEAD(&e->list);
	}
	i = list_count/2;
	e = &list[i];	
	e->size = size;
	e->base = base;
	return i;
}

/**
 * @brief reg_list_insert : insert the register into classify register array
 *
 * @param list : the classify register array with list format
 * @param i : the index of classify register array
 * @param obj : the object of register
 */
static void reg_list_insert(reg_array_t *list, int i, object_t *obj) {
	reg_item_t *item = gdml_zmalloc(sizeof(*item));

	if(i >= list_count) {
		BE_DBG(GENERAL, "index too large, expect index value smaller than %d\n", list_count);
	}
	item->obj = obj;
	INIT_LIST_HEAD(&item->entry);
	list_add_tail(&item->entry, &list[i].list);
}

/**
 * @brief sort_register_array : classify the register into one without array and one with array
 *
 * @param b: the object of bank
 *
 * @return : the classified array about register
 */
static reg_array_t *sort_register_array(bank_t *b) {
	struct list_head *p;
	object_t *obj;
	dml_register_t *reg;
	reg_array_t *list = NULL;
	int i = 0;

	list = new_reg_list();
	list_for_each(p, &b->obj.childs) {
		obj = list_entry(p, object_t, entry);	
		reg = (dml_register_t *)obj;
		if(reg->is_undefined) {
			continue;
		}
		if(reg->is_array) {
			i = find_slot(list, reg->offset,  reg->array_size);		
			reg_list_insert(list, i, obj);
		} else {
			reg_list_insert(list, 0, obj);
		}
	}
	return list;
}

/**
 * @brief gen_code_once_noplatform : genenrate the code without relationship with platform
 *
 * @param dev : the object of device
 * @param f : c file to be generated
 */
void gen_code_once_noplatform(device_t *dev, FILE *f){
	bank_t *b;
	struct list_head *p;
	reg_array_t *list;

	gen_device_reset(dev,f);
	gen_device_implement_code(dev, f);
	gen_device_port_code(dev, f);
	gen_device_event_code(dev, f);
	list_for_each(p,&dev->obj.childs){
		b = list_entry(p, bank_t, obj.entry);
		list = sort_register_array(b);
		gen_bank_read_access(b, list, f);
		gen_bank_write_access(b, list, f);
	}	
}
