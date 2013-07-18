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

extern object_t *DEV;

static void gen_bank_read_access(bank_t *b,FILE *f){
	int register_size = b->register_size;
	int offset;
	int size;
	object_t *t;
	struct list_head *p;
	int exec_index = get_local_index();
	int param = get_local_index();
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
	fprintf(f,"\t\t\t\t\t*readvalue = 0;\n",param);
	
	fprintf(f,"\t\t\t\t\tswitch(v%d_offset / %d )\n",param,register_size);
	fprintf(f, "\t\t\t\t\t{\n");
	int i = 0;
	int ret_index;
	int local_index;
	offset = 0;
	size = 0;
	int bound = 0;
	list_for_each(p,&b->obj.childs){
		t = list_entry(p,object_t,entry);
		local_index = get_local_index();
		fprintf(f,"\t\t\t\t\t\tcase %d:\n",i++);
		fprintf(f,"\t\t\t\t\t\t\t{\n");
		fprintf(f,"\t\t\t\t\t\t\t\tuint8 v%d_lsb;\n",local_index);
		fprintf(f,"\t\t\t\t\t\t\t\tuint8 v%d_msb;\n",local_index);
		bound = offset + register_size;	
		fprintf(f,"\t\t\t\t\t\t\t\tv%d_size2 = v%d_size < %d - v%d_offset ? v%d_size : %d - v%d_offset;\n",param,param,bound,param,param,bound,param);
		fprintf(f,"\t\t\t\t\t\t\t\tv%d_lsb = (v%d_offset - %d) * 8;\n",local_index,param,offset);
		fprintf(f,"\t\t\t\t\t\t\t\tv%d_msb = v%d_lsb + v%d_size2 * 8;\n",local_index,local_index,param);
		offset += register_size;
		fprintf(f,"\t\t\t\t\t\t\t\t{\n");
		ret_index = get_local_index();
		fprintf(f,"\t\t\t\t\t\t\t\t\tuint64 v%d_ret_value  = 0;\n",ret_index);	
		fprintf(f,"\t\t\t\t\t\t\t\t\t{\n");
		add_object_method(t,"read_access");	
		fprintf(f,"\t\t\t\t\t\t\t\t\t\tv%d_exec = _DML_M_%s__read_access(_dev, memop, v%d_msb, v%d_lsb, &v%d_ret_value);\n",exec_index,t->qname,local_index,local_index,ret_index);	
		fprintf(f,"\t\t\t\t\t\t\t\t\t\tif(v%d_exec)\n",exec_index);		
		fprintf(f,"\t\t\t\t\t\t\t\t\t\t\treturn 1;\n",exec_index);
		
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

static void gen_bank_write_access(bank_t *b,FILE *f){
	int register_size = b->register_size;
	int offset;
	int size;
	object_t *t;
	struct list_head *p;
	int exec_index = get_local_index();
	int param = get_local_index();

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
	
	fprintf(f,"\t\t\t\t\tswitch(v%d_offset / %d )\n",param,register_size);
	fprintf(f, "\t\t\t\t\t{\n");
	int i = 0;
	int ret_index;
	int local_index;
	offset = 0;
	size = 0;
	int bound = 0;
	list_for_each(p,&b->obj.childs){
		t = list_entry(p,object_t,entry);
		local_index = get_local_index();
		fprintf(f,"\t\t\t\t\t\tcase %d:\n",i++);
		fprintf(f,"\t\t\t\t\t\t\t{\n");
		fprintf(f,"\t\t\t\t\t\t\t\tuint8 v%d_lsb;\n",local_index);
		fprintf(f,"\t\t\t\t\t\t\t\tuint8 v%d_msb;\n",local_index);
		bound = offset + register_size;	
		fprintf(f,"\t\t\t\t\t\t\t\tv%d_size2 = v%d_size < %d - v%d_offset ? v%d_size : %d - v%d_offset;\n",param,param,bound,param,param,bound,param);
		fprintf(f,"\t\t\t\t\t\t\t\tv%d_lsb = (v%d_offset - %d) * 8;\n",local_index,param,offset);
		fprintf(f,"\t\t\t\t\t\t\t\tv%d_msb = v%d_lsb + v%d_size2 * 8;\n",local_index,local_index,param);
		offset += register_size;
		fprintf(f,"\t\t\t\t\t\t\t\t{\n");
		ret_index = get_local_index();
		add_object_method(t,"write_access");
		fprintf(f,"\t\t\t\t\t\t\t\t\tv%d_exec = _DML_M_%s__write_access(_dev, memop, v%d_msb, v%d_lsb, writevalue);\n",exec_index,t->qname,local_index,local_index);	
		fprintf(f,"\t\t\t\t\t\t\t\t\tif(v%d_exec)\n",exec_index);		
		fprintf(f,"\t\t\t\t\t\t\t\t\t\treturn 1;\n",exec_index);	
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

static void  gen_hard_reset(device_t *dev, FILE *f) {
    const char *name = dev->obj.name;
    int index = get_local_index();

    add_object_method(&dev->obj, "hard_reset");
    fprintf(f, "\nvoid %s_hard_reset(%s_t *obj) {\n", name, name);
    fprintf(f, "\tbool v%d_exec;\n", index);
    fprintf(f, "\tUNUSED(v%d_exec);\n", index);
    fprintf(f, "\n");
    fprintf(f, "\tv%d_exec = _DML_M_hard_reset(obj);\n",index, name);
    fprintf(f, "}\n");
}

static void  gen_soft_reset(device_t *dev, FILE *f) {
    const char *name = dev->obj.name;
    int index = get_local_index();

    add_object_method(&dev->obj, "soft_reset");
    fprintf(f, "\nvoid %s_soft_reset(%s_t *obj) {\n", name, name);
    fprintf(f, "\tbool v%d_exec;\n", index);
    fprintf(f, "\tUNUSED(v%d_exec);\n", index);
    fprintf(f, "\n");
    fprintf(f, "\tv%d_exec = _DML_M_soft_reset(obj);\n", index, name);
    fprintf(f, "}\n");
}

static void gen_device_reset(device_t *dev, FILE *f) {
    gen_hard_reset(dev, f);
    gen_soft_reset(dev, f);
}

void gen_code_once_noplatform(device_t *dev, FILE *f){
	bank_t *b;
	struct list_head *p;

	gen_device_reset(dev,f);
	list_for_each(p,&dev->obj.childs){
		b = list_entry(p, bank_t, obj.entry);
		gen_bank_read_access(b, f);
		gen_bank_write_access(b, f);
	}	
}
