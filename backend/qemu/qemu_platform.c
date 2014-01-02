/*
 * qemu_platform.c: 
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
#include <time.h>
#include "gen_utility.h"
#include "gen_object.h"
#include "platform.h"
#include "gen_connect.h"
#include "gen_implement.h"
#include "gen_attribute.h"
#include  "gen_port.h"
#include "gen_property.h"
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

    fprintf(f, "\n#include \"%s.h\"\n", name);
    fprintf(f, "\n#include \"%s_protos.c\"\n", name);
}

void pre_gen_code(device_t *dev, FILE *f) {
	gen_headerfile(dev, f);
}

static void gen_bank_read_access(object_t *obj, FILE *f) {
	const char *name = obj->name;
	const char *dev_name = DEV->name;
	const char *fmt = "0x%08x";
	int index = get_local_index();
	int index2 = get_local_index();
	int index3 = get_local_index();

	fprintf(f, "\nstatic uint64_t\n");
	fprintf(f, "%s_mmio_read(void *opaque, hwaddr addr, unsigned size) {\n", name);
	F_HEAD;
	fprintf(f, "\t%s_t *_dev = (%s_t *)opaque;\n", dev_name, dev_name);
	fprintf(f, "\tgeneric_transaction_t v%d_memop;\n", index);
	fprintf(f, "\tbool v%d_ret = 0;\n", index2);
	fprintf(f, "\tuint64_t v%d_val;\n", index3);
	fprintf(f, "\n");
	fprintf(f, "\tmemset(&v%d_memop, 0, sizeof(v%d_memop));\n", index, index);
	fprintf(f, "\tSIM_set_mem_op(&v%d_memop, %s);\n", index,"!SIM_Trn_Write");
	fprintf(f, "\tv%d_ret = _DML_M_%s__access(_dev, &v%d_memop, (physical_address_t)addr, (physical_address_t)size);\n",index2, name, index);
	fprintf(f, "\tif(!v%d_ret) {\n", index2);
	fprintf(f, "\t\tv%d_val = SIM_mem_op_get_value(&v%d_memop);\n", index3, index);
	fprintf(f, "\t} else {\n");
	fprintf(f, "\t\tSIM_log_err(&_dev->obj.qdev, 0, \"MMIO unkown read addr=%s\\n\", (unsigned int)addr);\n", fmt);
	fprintf(f, "\t\tv%d_val = 0;\n", index3);
	fprintf(f, "\t}\n");
	F_END;
	fprintf(f, "\treturn v%d_val;\n", index3);
	fprintf(f, "}\n");
}

static void gen_bank_write_access(object_t *obj, FILE *f) {
	const char *name = obj->name;
	const char *dev_name = DEV->name;
	int index = get_local_index();
	int index2 = get_local_index();

	fprintf(f, "\nstatic void\n");
	fprintf(f, "%s_mmio_write(void *opaque, hwaddr addr, uint64_t val, unsigned size) {\n", name);
	F_HEAD;
	fprintf(f, "\t%s_t *_dev = (%s_t *)opaque;\n", dev_name, dev_name);
	fprintf(f, "\tgeneric_transaction_t v%d_memop;\n", index);
	fprintf(f, "\tbool v%d_ret = 0;\n ", index2);
	fprintf(f, "\tUNUSED(v%d_ret);\n", index2);
	fprintf(f, "\n");
	fprintf(f, "\tmemset(&v%d_memop, 0, sizeof(v%d_memop));\n", index, index);
	fprintf(f, "\tSIM_set_mem_op(&v%d_memop, %s);\n", index, "SIM_Trans_Store");
	fprintf(f, "\tSIM_mem_op_set_value(&v%d_memop, val);\n", index);
	fprintf(f, "\tv%d_ret = _DML_M_%s__access(_dev, &v%d_memop, (physical_address_t)addr, (physical_address_t)size);\n", index2, name, index);
	F_END;
	fprintf(f, "}\n");
}	

static void gen_bank_access(object_t *obj, FILE *f) {
	gen_bank_read_access(obj, f);
	gen_bank_write_access(obj, f);
	add_object_method(obj, "access");
}

static void gen_bank_mr(object_t *obj, FILE *f) {
	bank_t *b = (bank_t *)obj;
	const char *name = obj->name;
	int min_size, max_size;
	min_size = max_size = b->register_size;
	const char *endianness = "DEVICE_LITTLE_ENDIAN";

	gen_bank_access(obj, f);
	fprintf(f, "\nstatic const MemoryRegionOps %s_mmio_ops = {\
				\n\t.read = %s_mmio_read,\
				\n\t.write = %s_mmio_write,\
				\n\t.endianness = %s,\
				\n\t.impl = { \
				\n\t\t.min_access_size = %d,\
				\n\t\t.max_access_size = %d,\
				\n\t},\
				\n};\n", name, name, name, endianness, min_size, max_size);
}

static void gen_banks_mr(device_t *dev, FILE *f) {
	int i = 0;
	object_t *obj;
	for(i = 0; i < dev->bank_count; i++) {
		obj = dev->banks[i];
		gen_bank_mr(obj, f);	
	}
}

static void gen_mmio_setup(device_t *dev, FILE *f) {
	const char *dev_name = dev->obj.name;
	int i = 0;
	object_t *obj;
	char *cap;

	gen_banks_mr(dev, f);
	fprintf(f, "\nstatic void\n");
	fprintf(f, "%s_mmio_setup(%s_t *_dev) {\n", dev_name, dev_name);
	F_HEAD;
	for(i = 0; i < dev->bank_count; i++) {
		obj = dev->banks[i];
		cap = to_upper(obj->name);
		fprintf(f, "\n\tmemory_region_init_io(&_dev->mr_%s, &%s_mmio_ops, _dev, \"%s\",\
			\n\t\t\t\t\t   %s_MR_SIZE);\n", obj->name, obj->name, obj->name, cap);
		fprintf(f, "\tsysbus_init_mmio(&_dev->obj, &_dev->mr_%s);\n", obj->name);
		free(cap);
	}
	F_END;
	fprintf(f, "}\n");
}

static void gen_notifier_func(device_t *dev, FILE *f) {
	const char *name = dev->obj.name;

	fprintf(f, "\nstatic void %s_notify(Notifier *n, void *data){\n", name);
	fprintf(f, "\t%s_t *_dev = container_of(n, %s_t, notifier);\n", name, name);
	fprintf(f, "\tbool ret;\n");
	fprintf(f, "\tUNUSED(ret);\n");
	fprintf(f, "\tret = _DML_M_post_init(_dev);\n");
	fprintf(f, "}\n");
}

static void gen_post_init(device_t *dev, FILE *f) {
	const char *name = dev->obj.name;

	gen_notifier_func(dev, f);
}

void gen_device_init(device_t *dev, FILE *f) {
	const char *dev_name = dev->obj.name;
	int index = get_local_index();

	add_object_method(&dev->obj, "init");
	gen_post_init(dev, f);
	add_object_method(&dev->obj, "post_init");
	gen_mmio_setup(dev, f);
	fprintf(f, "\nstatic int %s_init(SysBusDevice *dev) {\n", dev_name);
	F_HEAD;
	fprintf(f, "\t%s_t *_dev = DO_UPCAST(%s_t, obj, dev);\n", dev_name, dev_name);
	fprintf(f, "\tbool v%d_exec = 0;\n", index);
	fprintf(f, "\tUNUSED(v%d_exec);\n", index);
	fprintf(f, "\n\t%s_hard_reset(_dev);\n", dev_name);
	fprintf(f, "\tv%d_exec = _DML_M_init(_dev);\n", index);
	fprintf(f, "\t%s_mmio_setup(_dev);\n", dev_name);
	fprintf(f, "\t_dev->notifier.notify = %s_notify;\n", dev_name);
	fprintf(f, "\tSIM_add_device_post_init(&_dev->notifier);\n");
	F_END;
	fprintf(f, "\treturn 0;\n");
	fprintf(f, "}\n");
}

static void gen_device_props(device_t *dev, FILE *f)
{
	const char *dev_name = dev->obj.name;
    fprintf (f, "\nstatic Property %s_props[] = {\
            \n\t", dev_name);
    //gen_device_property(dev, f);
    fprintf (f, "\n\tDEFINE_PROP_END_OF_LIST()");
    fprintf (f, "\n};\n");
}

static void gen_device_vmstate(device_t *dev, FILE *f)
{
	const char *dev_name = dev->obj.name;
    fprintf (f, "\nstatic const VMStateDescription %s_vmstate = { \
    \n\t.name = \"%s\",\
    \n\t.version_id = 1,\
    \n\t.minimum_version_id = 1,\
    \n\t.minimum_version_id_old = 1,\
    \n\t.fields = (VMStateField[]){\
    \n", dev_name, dev_name);

    fprintf (f, "\n\t\tVMSTATE_END_OF_LIST()\n");

    fprintf (f, "\n\t}\n");

    fprintf (f, "\n};\n");
}

static void gen_dc_reset(device_t *dev, FILE *f) {
	const char *name = dev->obj.name;

    fprintf (f, "\nstatic void %s_reset(DeviceState *dev){", name);
	F_HEAD;
    fprintf (f, "\n\t%s_t *_dev = DO_UPCAST(%s_t, obj.qdev, dev);\
            \n", name, name);
	fprintf(f, "\t%s_hard_reset(_dev);\n", name);	
	F_END;
	fprintf(f, "}\n");
}

static void gen_device_connect(device_t *dev, FILE *f) {
	struct list_head *p;
	object_t *tmp;
	int i = 0;
	const char *name;

	gen_device_connect_code(dev, f);

	fprintf(f, "\nconst struct ConnectDescription %s_connects[] = {\n", dev->obj.name);
	list_for_each(p, &dev->connects) {
		tmp = list_entry(p, object_t, entry);
		if(tmp->is_array) {
			name = tmp->a_name;
		} else 	{
			name = tmp->name;
		}
		fprintf(f, "\t[%d] = (struct ConnectDescription) {\n", i);
		fprintf(f, "\t\t.name = \"%s\",\n", name);
		fprintf(f, "\t\t.set = %s_set,\n", name);
		fprintf(f, "\t\t.get = %s_get,\n", name);
		fprintf(f, "\t},\n");
		i++;
	}
	fprintf(f, "\t[%d] = {}\n", i);
	fprintf(f, "};\n");

}

static void gen_device_implement(device_t *dev, FILE *f) {
		gen_device_implement_desc(dev, f);
}

static void gen_device_attribute_code(device_t *dev, FILE *f) {
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
		} else 	{
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

void gen_platform_device_module(device_t *dev, const char *out) {
}

static void gen_device_class_init(device_t *dev, FILE *f)
{
    const char *name = dev->obj.name;
	
	gen_dc_reset(dev, f);
    gen_device_props(dev, f);
    gen_device_vmstate(dev, f);
	gen_device_connect(dev, f);
	gen_device_implement(dev, f);
	gen_device_port_description(dev, f);
	gen_device_attribute_code(dev, f);

    fprintf (f, "\nstatic void %s_class_init(ObjectClass *klass,void *data){", name);
	F_HEAD;
    fprintf (f, "\n\tDeviceClass *dc = DEVICE_CLASS(klass); \
            \n\tSysBusDeviceClass *k = SYS_BUS_DEVICE_CLASS(klass);\
            \n\n\tk->init = %s_init; \
            \n\tdc->reset = %s_reset; \
            \n\tdc->desc = \"%s\"; \
            \n\tdc->props = %s_props; \
            \n\tdc->vmsd = &%s_vmstate; \
			\n\tdc->connects = %s_connects;\
			\n\tdc->ifaces = %s_ifaces; \
			\n\tdc->ports  = %s_ports; \
			\n\tdc->attrs  = %s_attributes;", name, name, name, name, name, name, name, name, name);
		
		
	F_END;
    fprintf (f, "\n}\n");
}

void gen_device_type_info(device_t * dev, FILE *f)
{
    const char *dev_name = dev->obj.name;

    gen_device_class_init(dev, f);
    /*generate type info */
    fprintf (f, "\nstatic const TypeInfo %s_info = { \
    \n\t.name = \"%s\", \
    \n\t.parent = TYPE_SYS_BUS_DEVICE,\
    \n\t.instance_size = sizeof(%s_t),\
    \n\t.class_init = %s_class_init,\
    \n};\n", dev_name, dev_name, dev_name, dev_name);

    fprintf (f, "\nstatic void %s_register_types(void){", dev_name);
	F_HEAD;
    fprintf (f, "\n\ttype_register_static(&%s_info);", dev_name);
	F_END;
    fprintf (f, "\n}\n");

    fprintf (f, "\ntype_init(%s_register_types)\n", dev_name);
}

void post_gen_code(device_t *dev, FILE *f) {
}



