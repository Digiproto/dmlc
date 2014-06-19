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
extern FILE *out;
static int list_count = LIST_SZ;
static offset_info_t *loc;
static void gen_group_access2(object_t *obj, reg_array_t *list, int register_size, int is_read);
static reg_array_t *sort_register_array2(struct list_head *p);

/* just a try*/
#define MAGIC_NUM 100
typedef struct {
	int param;
	int exec;
	int found;
} label_info_t;
static const char *get_output_fmt(int num) {
	const char *fmt;

	if(num <= MAGIC_NUM) {
		fmt = "%d";
	} else {
		fmt = "0x%x";
	}
	return fmt;
} 

static int reg_cal_offset(dml_register_t *reg) {
	int depth, j;
	int offset;
	int is_array;	

	depth = reg->obj.depth;
	offset = reg->offset_info.offset;
	is_array = reg->obj.is_array;
	depth = is_array ? depth -1 : depth;
	if(depth >= 1) {
		j = 0;
		while(j < depth) {
			offset += reg->offset_info.interval[j] * loc->interval[j];
			j++;
		}		
	}
	return offset;
}

static int gen_reg_offset(dml_register_t *reg) {
	int depth, j;
	int offset;
	int is_array;	

	depth = reg->obj.depth;
	offset = reg->offset_info.offset;
	is_array = reg->obj.is_array;
	depth = is_array ? depth -1 : depth;
	D_c("start = 0x%x;\n", reg->offset_info.offset);
	if(depth > 1) {
		j = 0;
		while(j < depth) {
			D_c("start += 0x%x * _idx%d;\n", reg->offset_info.interval[j], j);
			j++;
		}	
	}
	D_c("end = start + 0x%x;\n", reg->offset_info.interval[depth -1]);
}
static void sort_case(struct list_head *head, reg_item_t *item, int register_size) {
	struct list_head *p;
	case_sort_t *sort = NULL;
	dml_register_t *reg;
	
	reg = (dml_register_t *)item->obj;	
	list_for_each(p, head) {
		sort = list_entry(p, case_sort_t, entry);	
		if(reg->offset_info.offset/register_size == sort->val) {
			list_add_tail(&item->case_entry, &sort->sublist);
			return;
		}
	}
	sort = (case_sort_t *)gdml_zmalloc(sizeof *sort);
	INIT_LIST_HEAD(&sort->entry);	
	INIT_LIST_HEAD(&sort->sublist);	
	sort->val = reg_cal_offset(reg)/register_size;
	list_add_tail(&sort->entry, head);
	list_add_tail(&item->case_entry, &sort->sublist);
}

static void gen_register_array_access(reg_array_t *list, label_info_t *info, int register_size, int is_read) {
	int i;
	struct list_head *p;
	dml_register_t *reg;	
	reg_item_t *item;
	int param;
	int exec_index;
	int found_index;
	reg_array_t *e;
	int len;
	const char *func;

	func = is_read ? "read_access":"write_access";
	param = info->param;
	exec_index = info->exec;
	found_index = info->found;
	for(i = 1; i < list->list_count; i++) {
		e = &list[i];
		if(!e->size) {
			break;
		}
		len = register_size * e->size;
		POS;
		enter_scope();
		list_for_each(p, &e->list) {
			item = list_entry(p, reg_item_t, entry);
			reg = (dml_register_t *)item->obj;
			int ret_index = get_local_index();
			int depth = item->obj->depth;
			add_object_method(item->obj, func);
			int tmp = reg->offset/len;
			int interval = reg->offset_info.interval[depth - 1];
			int start;
			int end;
			int j;
			int index;
			start = reg_cal_offset(reg);	
			end = start + (reg->array_size - 1)* interval;
			D_c("if(v%d_offset >= 0x%x && v%d_offset <= 0x%x)\n", param, start, param, end); // temp
			POS;
			enter_scope();
			D_c("int _idx0;\n");
			D_c("_idx0 = (v%d_offset - 0x%x) / %d;\n", param, start, interval);
			D_c("if(((_idx0 >= 0) && (_idx0 < %d)) && ((v%d_offset - _idx0* %d - 0x%x) == 0))\n",reg->array_size, param,  interval, start);
			POS;
			enter_scope();
			if(is_read) {
				ret_index = get_local_index();
				D_c("uint64 v%d_ret_value;\n", ret_index);
			}
			D_c("v%d_size2 = %d;\n", param, reg->size);
			D_c("v%d_exec = _DML_M_%s__%s(_dev, ", exec_index, item->obj->qname, func);
			if(depth > 1) {
				j = 0;
				while(j < depth -1) {
					D("%d, ", loc->interval[j]);
					j++;
				}		
			}	
			D("_idx0, memop, 31, 0, " );
			if(is_read) {
				D("&v%d_ret_value);\n", ret_index);
			} else {	
				D("writevalue);\n");
			}
			D_c("if(v%d_exec)", exec_index);
			enter_scope();
			D_c("return 1;\n");
			exit_scope();
			new_line();
			if(is_read) {
				D_c("*readvalue = v%d_ret_value;\n", ret_index);
			}
			D_c("goto found%d;\n", found_index);
			exit_scope();
			new_line();
			exit_scope();
			new_line();

		}
		exit_scope();
		new_line();
	}
}

static void gen_register_single_access(reg_array_t *list, label_info_t *info, int register_size, int is_read) {
	int i;
	struct list_head *p;
	dml_register_t *reg;	
	reg_item_t *item;
	int param;
	int exec_index;
	int found_index;
	const char *func;

	param = info->param;
	exec_index = info->exec;
	found_index = info->found;
	func = is_read ? "read_access":"write_access";
	D_c("switch(v%d_offset / %d)\n",param, register_size);
	POS;
	enter_scope();
	int offset = 0;
	int size = 0;
	int bound = 0;
	int depth = 0;
	int j;
	LIST_HEAD(sorted_list);	
	list_for_each(p, &list[0].list){
		item = list_entry(p, reg_item_t, entry);
		sort_case(&sorted_list, item, register_size);	
	}
	case_sort_t *tmpx;
	list_for_each(p, &sorted_list) {
		tmpx = list_entry(p, case_sort_t, entry);	
		struct list_head *p2;
		if(tmpx->val < MAGIC_NUM) {
			D_c("case %d:\n", tmpx->val);
		} else {
			D_c("case 0x%x:\n", tmpx->val);
		}
		list_for_each(p2, &tmpx->sublist) {
			item = list_entry(p2, reg_item_t, case_entry);
			object_t *t = item->obj;
			reg = (dml_register_t *)t;
			int local_index = get_local_index();
			int ret_index = get_local_index();
			offset = reg_cal_offset(reg);	
			bound = offset + reg->size;
			depth = t->depth;
			int tmp = reg->offset/register_size;
			POS;
			enter_scope();
			D_c("uint8 v%d_lsb;\n",local_index);
			D_c("uint8 v%d_msb;\n",local_index);
			D_c("v%d_size2 = v%d_size < %d - v%d_offset ? v%d_size : %d - v%d_offset;\n",param,param,bound,param,param,bound,param);
			D_c("v%d_lsb = (v%d_offset - %d) * 8;\n",local_index,param, offset);
			D_c("v%d_msb = v%d_lsb + v%d_size2 * 8;\n",local_index,local_index,param);
			POS;
			enter_scope();
			if(is_read) {
				D_c("uint64 v%d_ret_value  = 0;\n",ret_index);	
			}
			add_object_method(t, func);	
			D_c("v%d_exec = _DML_M_%s__%s(_dev, ",exec_index,t->qname, func); 
			if(depth >= 1) {
				j = 0;
				while(j < depth) {
					D("%d, ", loc->interval[j]);
					j++;
				}
			}
			D("memop, v%d_msb, v%d_lsb, ", local_index,local_index);
			if(is_read)
				D("&v%d_ret_value);\n", ret_index);	
			else 
				D("writevalue);\n");	
			D_c("if(v%d_exec)",exec_index);		
			enter_scope();
			D_c("return 1;\n");
			exit_scope();	
			D("\n");
			if(is_read)
				D_c("*readvalue = v%d_ret_value;\n",ret_index);	
			exit_scope();
			D("\n");
			D_c("goto found%d;\n", found_index);
			exit_scope();
			D("\n");
		}	
	}
	exit_scope();
	D("\n");
}

static void gen_register_access(reg_array_t *list, int register_size, int is_read) {
	int exec_index = get_local_index();
	int param = get_local_index();
	int exit_index = get_exit_index();
	int found_index = get_local_index();

	label_info_t info = {.param = param, .exec = exec_index, .found = found_index};
	POS;
	enter_scope();
	D_c("bool v%d_exec = 0;\n",exec_index);
	POS;
	enter_scope();
	POS;
	enter_scope();
	D_c("uint64 v%d_offset = offset;\n",param);	
	D_c("int64 v%d_size = size;\n",param);
	D_c("int64 v%d_size2;\n",param);
	if(is_read)
		D_c("*readvalue = 0;\n");
	gen_register_array_access(list, &info, register_size, is_read);
	gen_register_single_access(list, &info, register_size, is_read);	
	D_c("*success = 0;\n");
	D_c("goto exit%d;\n",exit_index);
	D_c("found%d:\n", found_index);
	D_c("*success = v%d_size == v%d_size2;\n", param, param);
	exit_scope();
	D("\n");
	//D_c("exit%d:;\n",exit_index);
	exit_scope();
	D("\n");
	exit_scope();
	D("\n");
	D_c("return 0;\n");
	D_c("exit%d:;\n",exit_index);
}

static void gen_group_access(object_t *obj, int register_size, int is_read) {
	struct list_head *p;
	group_t *gp;
	object_t *tmp;
	int i;	
	int start, end;
	reg_array_t *list;
	gp = (group_t *)obj;
	int j;
	int index;

	for(i = 0; i < obj->array_size; i++) {	
		j = 0;
		start = gp->offset_info.offset;
		while(j < obj->depth - 1) {
			start += loc->interval[j] * gp->loc->interval[j];  
			j++;
		}
		start += gp->offset_info.len * i;
		end = start + gp->offset_info.len; 
		loc->interval[obj->depth -1] = i;
		index = obj->depth -1;
		D_c("if(offset >=  0x%x && offset < 0x%x)", start, end);
		enter_scope();
		list_for_each(p, &gp->groups) {
			tmp = list_entry(p, object_t, entry);
			POS;
			enter_scope();
			gen_group_access(tmp, register_size, is_read);	
			exit_scope();
			D("\n");
		}
		if(!list_empty(&gp->registers)) {
			struct list_head *p;
			object_t *obj;
			list_for_each(p, &gp->registers) {
				obj = list_entry(p, object_t,entry );
			}
			list = sort_register_array2(&gp->registers);		
			gen_group_access2(obj, list, register_size, is_read);
		}
		exit_scope();
		D("\n");
	}
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
	struct list_head *p;
	object_t *tmp;
	out = f;

	fprintf(f,"\nstatic bool\n");
	fprintf(f,"_DML_M_%s__read_access(%s_t *_dev, generic_transaction_t *memop, physical_address_t offset, physical_address_t size, bool *success, uint64 *readvalue)\n",b->obj.qname,DEV->name);
	enter_scope();
	gen_register_access(list, register_size, 1);
	list_for_each(p, &b->groups) {
		tmp = list_entry(p, object_t, entry);
		group_t *gp = (group_t *)tmp;
		loc = &gp->offset_info;	
		gen_group_access(tmp, register_size, 1);	
	}
	exit_scope();
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
	struct list_head *p;
	object_t *tmp;
	out = f;

	fprintf(f,"\nstatic bool\n");
	fprintf(f,"_DML_M_%s__write_access(%s_t *_dev, generic_transaction_t *memop, physical_address_t offset, physical_address_t size, uint64 writevalue, bool *success)\n",b->obj.qname,DEV->name);
	enter_scope();
	gen_register_access(list, register_size, 0);
	list_for_each(p, &b->groups) {
		tmp = list_entry(p, object_t, entry);
		gen_group_access(tmp, register_size, 0);	
	}
	exit_scope();
}


static void gen_group_access2(object_t *obj, reg_array_t *list, int register_size, int is_read) {
	POS;
	enter_scope();
	gen_register_access(list, register_size, is_read);	
	exit_scope();
	D("\n");
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

	reg_array_t *list = gdml_zmalloc(sizeof(*list) * LIST_SZ);
	list->list_count = LIST_SZ;
	e = &list[0];
	for(; i < list->list_count; i++) {
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

	fprintf(stderr, "base %d, size %d, list %p\n", base, size, list);	
	for(; i < list->list_count; i++) {
		e = &list[i];
		if(e->size == size) {
			return i;
		}
	} 
	fprintf(stderr, "here list %p\n", list);
	for(i = 1; i < list->list_count; i++) {
		e = &list[i];
		if(!e->size) {
			e->size = size;
			return i;
		}
	}
	fprintf(stderr, "here2 list %p\n", list);
	if(i >= list->list_count) {
		list->list_count *= 2;
		list = realloc(list, list->list_count * sizeof(*e));
	}
	fprintf(stderr, "here3 list %p\n", list);
	for(; i < list->list_count; i++) {
		e = &list[i];
		e->size = 0;
		INIT_LIST_HEAD(&e->list);
	}
	i = list->list_count/2;
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

	if(i >= list->list_count) {
		BE_DBG(GENERAL, "index too large, expect index value smaller than %d\n", list_count);
	}
	item->obj = obj;
	INIT_LIST_HEAD(&item->entry);
	INIT_LIST_HEAD(&item->case_entry);
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
		if(reg->is_undefined || reg->is_unmapped) {
			continue;
		}
		fprintf(stderr, "i %d, reg %s, base 0x%x, list %p\n", i++, reg->obj.name, reg->offset, list);
		if(reg->is_array) {
			fprintf(stderr, "is array list %p\n", list);
			i = find_slot(list, reg->offset,  reg->array_size);		
			reg_list_insert(list, i, obj);
		} else {
			reg_list_insert(list, 0, obj);
		}
		fprintf(stderr, "end\n");
	}
	return list;
}

static reg_array_t *sort_register_array2(struct list_head *regs_list) {
	struct list_head *p;
	object_t *obj;
	dml_register_t *reg;
	reg_array_t *list = NULL;
	int i = 0;

	if(!regs_list)
		return;
	list = new_reg_list();
	list_for_each(p, regs_list) {
		obj = list_entry(p, object_t, entry);	
		reg = (dml_register_t *)obj;
		fprintf(stderr, "i %d, regxxx %s, base 0x%x, list %p\n", i++, reg->obj.name, reg->offset_info.offset, list);
		if(reg->is_undefined || reg->is_unmapped) {
			continue;
		}
		fprintf(stderr, "i %d, regxxx %s, base 0x%x, list %p\n", i++, reg->obj.name, reg->offset_info.offset, list);
		if(reg->is_array) {
			fprintf(stderr, "is array list %p\n", list);
			i = find_slot(list, reg->offset_info.offset,  reg->array_size);		
			reg_list_insert(list, i, obj);
		} else {
			reg_list_insert(list, 0, obj);
		}
		fprintf(stderr, "end\n");
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
		list = sort_register_array2(&b->obj.childs);
		gen_bank_read_access(b, list, f);
		gen_bank_write_access(b, list, f);
	}	
}
