/*
 * skyeye_platform.c: 
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
#include "platform.h"
#include "gen_utility.h"
#include <time.h>
#include "gen_method_protos.h"
#include "gen_connect.h"
#include "gen_implement.h"
#include "skyeye_gen_attribute.h"
#include "gen_struct.h"

extern object_t *DEV;

static void gen_headerfile(device_t *dev, FILE *f) {
    const char *name = dev->obj.name;
    time_t timep;

    time(&timep);
    fprintf(f, "/* \n");
    fprintf(f, " * %s.c\n", name);
    fprintf(f, " * Generated by dmlc, not to edit it\n");
    fprintf(f, " * %s",asctime(localtime(&timep)));
    fprintf(f, " */\n");
	fprintf(f, "\n");
    fprintf(f, "#include  <string.h>\n", name);
    fprintf(f, "#include \"%s.h\"\n", name);
    fprintf(f, "#include \"%s_protos.c\"\n", name);
    fprintf(f, "#include \"skyeye_mm.h\"\n");
    fprintf(f, "#include \"skyeye_class.h\"\n", name);
	fprintf(f, "#include \"skyeye_iface.h\"\n");
	fprintf(f, "#include \"skyeye_attribute.h\"\n");
}

void pre_gen_code(device_t *dev, FILE *f) {
	gen_headerfile(dev, f);
}

static void gen_bank_read_access(object_t *obj, FILE *f) {
	const char *name = obj->name;
	const char *dev_name = DEV->name;

	fprintf(f, "\nstatic exception_t\n");
	fprintf(f, "%s_read(conf_object_t *obj, generic_address_t addr, void *buf, size_t count) {\n", name);
	F_HEAD;
	fprintf(f, "\t%s_t *_dev = (%s_t *)obj;\n", dev_name, dev_name);
	fprintf(f, "\tgeneric_transaction_t memop;\n");
	fprintf(f, "\tbool ret = 0;\n");
	fprintf(f, "\tUNUSED(ret);\n");
	fprintf(f, "\tuint64_t val;\n");
	fprintf(f, "\n");
	fprintf(f, "\tmemset(&memop, 0, sizeof(memop));\n");
	fprintf(f, "\tSIM_set_mem_op(&memop, !SIM_Trn_Write);\n");
	fprintf(f, "\tret = _DML_M_%s__access(_dev, &memop, (physical_address_t)addr, (physical_address_t)count);\n",  name);
	fprintf(f, "\tval = SIM_mem_op_get_value(&memop);\n");
	fprintf(f, "\tmemcpy(buf, &val, count);\n");
	F_END;
	fprintf(f, "\treturn No_exp;\n");
	fprintf(f, "}\n");
}

static void gen_bank_write_access(object_t *obj, FILE *f) {
	const char *name = obj->name;
	const char *dev_name = DEV->name;

	fprintf(f, "\nstatic exception_t\n");
	fprintf(f, "%s_write(conf_object_t *obj, generic_address_t addr, const void *buf, size_t count) {\n", name);
	F_HEAD;
	fprintf(f, "\t%s_t *_dev = (%s_t *)obj;\n", dev_name, dev_name);
	fprintf(f, "\tgeneric_transaction_t memop;\n");
	fprintf(f, "\tbool ret = 0;\n");
	fprintf(f, "\tUNUSED(ret);\n");
	fprintf(f, "\tuint64_t val = *(uint64 *)buf;\n");
	fprintf(f, "\n");
	fprintf(f, "\tmemset(&memop, 0, sizeof(memop));\n");
	fprintf(f, "\tSIM_set_mem_op(&memop, SIM_Trn_Write);\n");
	fprintf(f, "\tSIM_mem_op_set_value(&memop, val);\n");
	fprintf(f, "\tret = _DML_M_%s__access(_dev, &memop, (physical_address_t)addr, (physical_address_t)count);\n",  name);
	F_END;
	fprintf(f, "\treturn No_exp;\n");
	fprintf(f, "}\n");
}

static void gen_bank_access(object_t *obj, FILE *f) {
    gen_bank_read_access(obj, f);
    gen_bank_write_access(obj, f);
    add_object_method(obj, "access");
}

static void gen_banks_mr(device_t *dev, FILE *f) {
    int i = 0;
    object_t *obj;
    for(i = 0; i < dev->bank_count; i++) {
        obj = dev->banks[i];
        gen_bank_access(obj, f);
    }
}

static void gen_mmio_setup(device_t *dev, FILE *f) {
    const char *dev_name = dev->obj.name;
    int i = 0;
    object_t *obj;

    gen_banks_mr(dev, f);
    fprintf(f, "\nstatic void\n");
    fprintf(f, "%s_mmio_setup(%s_t *_dev, const char *objname) {\n", dev_name, dev_name);
    for(i = 0; i < dev->bank_count; i++) {
        obj = dev->banks[i];
		fprintf(f, "\t{\n");
		F_HEAD;
		fprintf(f, "\t\t_dev->mr_%s.conf_obj = &_dev->obj;\n", obj->name);	
		fprintf(f, "\t\t_dev->mr_%s.read = %s_read;\n", obj->name, obj->name);
		fprintf(f, "\t\t_dev->mr_%s.write = %s_write;\n", obj->name, obj->name);
		fprintf(f, "\t\tSKY_register_interface(&_dev->mr_%s, objname, MEMORY_SPACE_INTF_NAME);\n", obj->name);
		F_END;
		fprintf(f, "\t}\n");
    }
    fprintf(f, "}\n");
}


void gen_device_init(device_t *dev, FILE *f) {
    const char *dev_name = dev->obj.name;
    int index = get_local_index();

    add_object_method(&dev->obj, "init");
    gen_mmio_setup(dev, f);
    fprintf(f, "\nstatic conf_object_t  *%s_create(const char *name) {\n", dev_name);
	F_HEAD;
    fprintf(f, "\t%s_t *_dev = skyeye_mm_zero(sizeof(*_dev));\n", dev_name);
//	fprintf(f, "\tconf_object_register(&_dev->obj, name);\n");
	fprintf(f, "\t_dev->obj = new_conf_object(name, _dev);\n");
    fprintf(f, "\tbool v%d_exec = 0;\n", index);
    fprintf(f, "\tUNUSED(v%d_exec);\n", index);
    fprintf(f, "\n\t%s_hard_reset(_dev);\n", dev_name);
    fprintf(f, "\tv%d_exec = _DML_M_init(_dev);\n", index);
    //fprintf(f, "\t%s_mmio_setup(_dev, name);\n", dev_name);
	F_END;
    fprintf(f, "\treturn _dev->obj;\n");
    fprintf(f, "}\n");
}

static void gen_device_connect(device_t *dev, FILE *f) {
    struct list_head *p;
    object_t *tmp;
    int i = 0;

    gen_device_connect_code(dev, f);

    fprintf(f, "\nconst struct ConnectDescription %s_connects[] = {\n", dev->obj.name);
    list_for_each(p, &dev->connects) {
        tmp = list_entry(p, object_t, entry);
        fprintf(f, "\t[%d] = (struct ConnectDescription) {\n", i);
        fprintf(f, "\t\t.name = \"%s\",\n", tmp->name);
        fprintf(f, "\t\t.set = %s_set,\n", tmp->name);
        fprintf(f, "\t\t.get = %s_get,\n", tmp->name);
        fprintf(f, "\t},\n");
        i++;
    }
    fprintf(f, "\t[%d] = (struct ConnectDescription) {\n", i);
    fprintf(f, "\t\t.name = NULL,\n");
    fprintf(f, "\t\t.set = NULL,\n");
    fprintf(f, "\t\t.get = NULL,\n");
    fprintf(f, "\t}\n");
    fprintf(f, "};\n");

}

static void gen_device_module(device_t *dev, FILE *f) {
	const char *name = dev->obj.name;

	fprintf(f, "#include \"skyeye_module.h\"\n");
	fprintf(f, "\n");
	fprintf(f, "const char *skyeye_module = \"%s\";\n", name);
	fprintf(f, "\n");
	fprintf(f, "extern void init_%s(void);\n", name);
	fprintf(f, "\n");
	fprintf(f, "void module_init() {\n");
	fprintf(f, "\tinit_%s();\n", name);
	fprintf(f, "}\n");
	fprintf(f, "\n");
	fprintf(f, "void module_fini() {\n");
	fprintf(f, "}\n");
}

void gen_platform_device_module(device_t *dev, const char *out) {
	const char *name = dev->obj.name;
	char tmp[1024];
	FILE *f;

	snprintf(tmp, 1024, "%s/%s_module.c", out, name);
	f = fopen(tmp, "w");
	if(f) {
		gen_device_module(dev, f);
		fclose(f);
	} else {
		printf("failed to open file %s\n", tmp);
	}
}

static void gen_device_implement(device_t *dev, FILE *f) {
	gen_device_implement_desc(dev, f);
}

void gen_device_type_info(device_t *dev, FILE *f) {
	const char *name = dev->obj.name;

	//gen_device_struct(dev, f);
	gen_device_connect(dev, f);	
	gen_device_implement(dev, f);
	sky_gen_device_attribute_description(dev, f);
	fprintf(f, "\nvoid init_%s(void) {\n", name);
#ifdef DEVICE_TEST
	fprintf(f, "\tdebug_function_pos = 0;\n");
#endif
	F_HEAD;
	fprintf(f, "\tstatic skyeye_class_t class_data = {\n");
	fprintf(f, "\t\t.class_name = \"%s\",\n", name);	
	fprintf(f, "\t\t.class_desc = \"%s\",\n", name);	
	fprintf(f, "\t\t.new_instance = %s_create,\n", name);
	fprintf(f, "\t\t.free_instance = NULL,\n");
	fprintf(f, "\t\t.set_attr = NULL,\n");
	fprintf(f, "\t\t.get_attr = NULL,\n");
	fprintf(f, "\t\t.connects = %s_connects,\n", name);
	fprintf(f, "\t\t.ifaces = %s_ifaces,\n", name);
	fprintf(f, "\t};\n");
	fprintf(f, "\tconf_class_t* clss = SKY_register_class(class_data.class_name, &class_data);\n");

	/* gen interface register */
	#if 0
	list_empty
	if(dev->implements){
		fprintf(f, "\tint i = 0;");
		fprintf(f, "\tfor(i = 0; i < (sizeof(%s_ifaces) / sizeof (InterfaceDescription)); i++){\n", name);
		fprintf(f, "\t\tSKY_register_iface(clss, %s_ifaces[i].name, %s_ifaces[i].iface);\n", name, name);
		fprintf(f, "\t}\n");
        }
	#endif
	struct list_head *p;
        object_t *tmp;
        
        list_for_each(p, &dev->implements) {
                tmp = list_entry(p, object_t, entry);
                if(!strcmp(tmp->name, "io_memory")) {
                        continue;
                }
		fprintf(f, "\t\tSKY_register_iface(clss, %s_ifaces[i].name, %s_ifaces[i].iface);\n", tmp->name, tmp->name);
        }      

	//fprintf(f, "\tSKY_register_class(class_data.class_name, &class_data);\n");
	fprintf(f, "\tstatic const memory_space_intf io_memory = {\n");
	fprintf(f, "\t\t.read = %s_read,\n", "regs");
	fprintf(f, "\t\t.write = %s_write,\n", "regs");
	fprintf(f, "\t};\n");
	fprintf(f, "\tSKY_register_iface(clss, MEMORY_SPACE_INTF_NAME, &io_memory);\n");
	fprintf(f, "\t%s_register_attribute(clss);\n", name);
	F_END;
	fprintf(f, "}\n");
}

void post_gen_code(device_t *dev, FILE *f) {
}
