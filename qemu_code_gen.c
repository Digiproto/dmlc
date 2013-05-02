
/* Copyright (C) 
*  2013  alloc.young@gmail.com
* This program is free software; you can redistribute it and/or
* * modify it under the terms of the GNU General Public License
* * as published by the Free Software Foundation; either version 2
* * of the License, or (at your option) any later version.
* * 
* * This program is distributed in the hope that it will be useful,
* * but WITHOUT ANY WARRANTY; without even the implied warranty of
* * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* * GNU General Public License for more details.
* * 
* * You should have received a copy of the GNU General Public License
* * along with this program; if not, write to the Free Software
* * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
* * 
* */

/**
 * * @file code_gen.c
 * * @brief The c source code generation
 * * @author alloc alloc.young@gmail.com
 * * @version 7849
 * * @date 2013-3-12
 * */


#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "types.h"
#include "ast.h"
#include "symbol.h"

static char tab[16][32] =
	{ {"\t"}, {"\t\t"}, {"\t\t\t"}, {"\t\t\t\t"}, {"\t\t\t\t\t"} };

static char *get_tab (int n)
{
	return tab[n - 1];
}

static int cal_shift (int n)
{
	int i = 0;
	int tmp;
	if (!(n & (n - 1))) {
		for (i = 0;; i++) {
			tmp = n & (1 << i);
			if (tmp)
				break;
		}
	}
	else {
		printf ("0x%x is not right\n", n);
	}
	return i;
}

static void gen_register_struct (device_attr_t * dev, register_attr_t * reg,
								 FILE * f)
{
	char *type = "uint32_t";
	fprintf (f, "\t\t%s %s;\n", type, reg->name);
}

static void gen_bank_struct (device_attr_t * dev, bank_attr_t * b, FILE * f)
{
	register_list_node_t *list = b->registers;
	register_attr_t *reg;
	fprintf (f, "\n\tstruct {\n");
	for (; list; list = list->next) {
		reg = list->reg;
		gen_register_struct (dev, reg, f);
	}

	fprintf (f, "\n\t} %s;\n", b->name);

}

static void gen_header_file (device_attr_t * dev, FILE * f)
{
	fprintf (f, "\n#ifndef %s_H_\
			\n#define %s_H_", dev->name, dev->name);
	fprintf (f, "typedef struct %s{\
			\n\tSysBusDevice busdev;\n", dev->name);
	bank_list_node_t *b_list;
	bank_attr_t *b;
	b_list = dev->banks;
	for (; b_list; b_list = b_list->next) {
		b = b_list->bank;
		fprintf (f, "\n\tMemoryRegion %_mr;\n", b->name);
	}
	b_list = dev->banks;
	for (; b_list; b_list = b_list->next) {
		b = b_list->bank;
		gen_bank_struct (dev, b, f);
	}

	fprintf (f, "\n}%s_t;\
			\n\n#endif\n", dev->name);

}

static void gen_device_props (device_attr_t * dev, FILE * f)
{
	fprintf (f, "\nstatic Property %s_props[] = {\
			\n\t", dev->name);
	fprintf (f, "\n\tDEFINE_PROP_END_OF_LIST()");
	fprintf (f, "\n};\n");
}

static void gen_interface (interface_attr_t * iface, FILE * f)
{
}

static void gen_device_vmstate (device_attr_t * dev, FILE * f)
{
	fprintf (f, "\nstatic const VMStateDescription %s_vmstate = { \
	\n\t.name = \"%s\",\
	\n\t.version_id = 1,\
	\n\t.minumum_version_id = 1,\
	\n\t.minumum_old_version_id = 1,\
	\n\t.fields = {\
	\n", dev->name, dev->name);

	fprintf (f, "\n\t\tVMSTATE_END_OF_LIST()\n");

	fprintf (f, "\n\t}\n");

	fprintf (f, "\n};\n");
}

static void gen_register_read_access (device_attr_t * dev,
									  register_attr_t * reg, FILE * f)
{

	fprintf (f, "\nstatic uint64_t %s_%s_read(%s_t *_dev,unsigned size){\
			\n\treturn _dev->%s.%s;\
			\n}\n", reg->bank->name, reg->name, dev->name, reg->bank->name, reg->name);
}

static void gen_register_write_access (device_attr_t * dev,
									   register_attr_t * reg, FILE * f)
{

	fprintf (f, "\nstatic void %s_%s_write(%s_t *_dev,uint64_t val,unsigned size){\
			\n\t_dev->%s.%s = val;\
			\n}\n", reg->bank->name, reg->name, dev->name, reg->bank->name,
			 reg->name);
}

static void gen_registers_access (device_attr_t * dev, bank_attr_t * b,
								  FILE * f)
{
	register_attr_t *reg;
	register_list_node_t *l = b->registers;
	for (; l; l = l->next) {
		reg = l->reg;
		gen_register_read_access (dev, reg, f);
		gen_register_write_access (dev, reg, f);
	}
}

static void gen_bank_read_access (device_attr_t * dev, bank_attr_t * b,
								  FILE * f)
{

	int shift = cal_shift (b->register_size);
	fprintf (f, "\nstatic uint64_t %s_read(void *p,hwaddr addr,unsigned size){\
			\n\t%_t *_dev = (%s_t *)p;\
			\n\tunsigned int index = addr >> (%d);\
			\n\tswitch(index){\
			\n", b->name, dev->name, dev->name, shift);
	register_attr_t *reg;
	register_list_node_t *l = b->registers;
	for (; l; l = l->next) {
		reg = l->reg;
		fprintf (f, "\n\t\tcase 0x%x:\n", (reg->offset >> shift));
		fprintf (f, "\n\t\t\treturn %s_%s_read(_dev,size);\n", b->name,
				 reg->name);
	}
	fprintf (f, "\n\t\tdefault:\
			\n\t\t\tprintf(\"fault addr 0x%%x\\n\",index);");

	fprintf (f, "\n\t}\n}\n");
}


static void gen_bank_write_access (device_attr_t * dev, bank_attr_t * b,
								   FILE * f)
{

	int shift = cal_shift (b->register_size);
	fprintf (f, "\nstatic void %s_write(void *p,hwaddr addr,uint64_t val,unsigned size){\
			\n\t%_t *_dev = (%s_t *)p;\
			\n\tunsigned int index = addr >> (%d);\
			\n\tswitch(index){\
			\n", b->name, dev->name, dev->name,
			 shift);
	register_attr_t *reg;
	register_list_node_t *l = b->registers;
	for (; l; l = l->next) {
		reg = l->reg;
		fprintf (f, "\n\t\tcase 0x%x:\n", (reg->offset >> shift));
		fprintf (f, "\n\t\t\t%s_%s_write(_dev,val,size);\n", b->name,
				 reg->name);
		fprintf (f, "\n\t\t\tbreak;\n");
	}
	fprintf (f, "\n\t\tdefault:\
			\n\t\t\tprintf(\"fault addr 0x%%x\\n\",index);");

	fprintf (f, "\n\t}\n}\n");
}


static void gen_bank_accesses (device_attr_t * dev, FILE * f)
{
	bank_attr_t *b;
	bank_list_node_t *l = dev->banks;

	for (; l; l = l->next) {
		b = l->bank;
		gen_registers_access (dev, b, f);
		gen_bank_read_access (dev, b, f);
		gen_bank_write_access (dev, b, f);
	}

}

#define LITTLE_ENDIAN_T 0

static void gen_memory_region (device_attr_t * dev, FILE * f)
{
	bank_attr_t *b;
	bank_list_node_t *l = dev->banks;
	gen_bank_accesses (dev, f);
	char *endian =
		(dev->endian ==
		 LITTLE_ENDIAN_T) ? "DEVICE_LITTLE_ENDIAN" : "DEVICE_BIG_ENDIAN";
	for (; l; l = l->next) {
		b = l->bank;
		fprintf (f, "static const MemoryRegionOps %s_mmio_ops = {\
				\n\t.read = %s_read,\
				\n\t.write = %s_write;\
				\n\t.endianness = %s;\
				\n\t.imlp = {\
				\n\t\t.min_access_size = 1;\
				\n\t\t.max_access_size = %d;\
				\n\t},\
				\n};\n", b->name, b->name, b->name, endian, b->register_size);
	}

}


static void gen_register_init (register_attr_t * reg, FILE * f)
{
}

static void gen_bank_init (bank_attr_t * b, FILE * f)
{

	fprintf (f, "\n\t{\
			\n");
	register_attr_t *reg;
	register_list_node_t *l = b->registers;

	for (; l; l = l->next) {
		reg = l->reg;
		gen_register_init (reg, f);
	}

	fprintf (f, "\n\t}\n");
}

static void gen_banks_init (device_attr_t * dev, FILE * f)
{

	bank_attr_t *b;
	bank_list_node_t *l = dev->banks;

	fprintf (f, "\nstatic void %s_regs_init(%s_t *_dev){\
				\n", dev->name, dev->name);
	/*inline bank init */
	for (; l; l = l->next) {
		b = l->bank;
		gen_bank_init (b, f);
	}

	fprintf (f, "\n}\n");
}

static void gen_banks_mr_init (device_attr_t * dev, FILE * f)
{

	gen_memory_region (dev, f);

	fprintf (f, "\nstatic void %s_regs_mr_init(%s_t *_dev){\
				\n", dev->name, dev->name);
	bank_attr_t *b;
	bank_list_node_t *l = dev->banks;
	for (; l; l = l->next) {
		b = l->bank;
		fprintf (f, "\n\tmemory_region_init_io(&_dev->%s_mr,%s_mmio_ops,_dev,0x%x);\
				\n", b->name, b->name,
				 b->size);
		fprintf (f, "\n\tsysbus_init_mmio(_dev,&_dev->%s_mr);\
				\n", b->name);
	}
	fprintf (f, "\n}\n");
}

static void gen_device_init (device_attr_t * dev, FILE * f)
{

	char *dev_name = dev->name;
	gen_banks_init (dev, f);
	gen_banks_mr_init (dev, f);
	fprintf (f, "\nstatic %s_init(SysBusDevice *dev){\
			\n\t%s_t *_dev = FROM_SYSBUS(%_t,dev);\
			\n\t%s_regs_init(_dev);\
			\n\t%s_regs_mr_init(_dev);\
			\n\treturn 0;\
			\n}\n", dev->name, dev_name, dev_name);

}

static void gen_register_reset (device_attr_t * dev, register_attr_t * reg,
								FILE * f, int n)
{

	char *tab = get_tab (n);
	fprintf (f, "\n%s{\n", tab);

	char *tab2 = get_tab (n + 1);
	fprintf (f, "%s_dev->%s.%s = 0;\n", tab2, reg->bank->name, reg->name);

	fprintf (f, "\n%s}\n", tab);

}

static void gen_bank_reset (device_attr_t * dev, bank_attr_t * bank, FILE * f,
							int n)
{

	char *tab = get_tab (n);
	fprintf (f, "\n%s{\n", tab);

	register_attr_t *reg;
	register_list_node_t *l = bank->registers;

	for (; l; l = l->next) {
		reg = l->reg;
		gen_register_reset (dev, reg, f, n + 1);
	}

	fprintf (f, "\n%s}\n", tab);

}

static void gen_device_reset (device_attr_t * dev, FILE * f)
{

	char *name = dev->name;
	fprintf (f, "\nstatic void %s_reset(DeviceState *dev){\
			\n\t%s_t *_dev = DO_UPCAST(%s_t,dev.busdev,dev);\
			\n", name, name, name);
	/*inline hard reset code */
	bank_attr_t *b;
	bank_list_node_t *l = dev->banks;
	for (; l; l = l->next) {
		b = l->bank;
		gen_bank_reset (dev, b, f, 2);
	}

	fprintf (f, "\n}\n");
}


static void gen_device_class_init (device_attr_t * dev, FILE * f)
{
	char *name = dev->name;

	gen_device_reset (dev, f);
	gen_device_init (dev, f);
	gen_device_props (dev, f);
	gen_device_vmstate (dev, f);

	fprintf (f, "\nstatic void %s_class_init(ObjectClass *klass,void *data){ \
			\n\tDeviceClass *dc = DEVICE_CLASS(klass); \
			\n\tSysBusDeviceClass *k = SYS_BUS_DEVICE_CLASS(dc);\
			\n\n\tk->init = %s_init; \
			\n\tdc->reset = %s_reset; \
			\n\tdc->desc = \"%s\"; \
			\n\tdc->props = %s_props; \
			\n\tdc->vmsd = %s_vmstate; \
			\n}\n", name, name, name, name, name, name);
}

static void gen_device_type_info (device_attr_t * dev, FILE * f)
{
	char *dev_name = dev->name;
	gen_device_class_init (dev, f);
	/*generate type info */
	fprintf (f, "\nstatic const TypeInfo %s_info = { \
	\n\t.name = \"%s\", \
	\n\t.parent = TYPE_SYS_BUS_DEVICE,\
	\n\t.instance_size = sizeof(%s_t),\
	\n\t.class_init = %s_class_init,\
	\n};\n", dev_name, dev_name, dev_name, dev_name);

	fprintf (f, "\nstatic void %s_register_types(void){ \
			\n\ttype_register_static(&%s_info);\
			\n}\n", dev_name, dev_name);

	fprintf (f, "\ntype_init(%s_register_types)\n", dev_name);
}

void gen_qemu_code (node_t * root, char *name)
{
	//symbol_t *sym = symbol_find ("DEVICE", DEVICE_TYPE);
	/* FIXME: there is some problem */
	symbol_t *sym;
	if (!sym) {
		printf ("cannot find device tree\n");
		exit (-1);
	}
	//device_attr_t *dev = (device_attr_t *) sym->attr.device;
	/* FIXME: there is some problem */
	device_attr_t *dev;
#define PATH_SIZE 256
	char tmp[PATH_SIZE];

	snprintf (tmp, PATH_SIZE, "%s/%s.h", name, dev->name);

	FILE *f = fopen (tmp, "w");
	if (f) {
		gen_header_file (dev, f);
		fclose (f);
	}
	else {
		fprintf (stderr, "cannot open file %s\n", tmp);
		exit (-1);
	}


	snprintf (tmp, PATH_SIZE, "%s/%s.c", name, dev->name);
	f = fopen (tmp, "w");
	if (f) {
		gen_device_type_info (dev, f);
		fclose (f);
	}
	else {
		fprintf (stderr, "cannot open file %s\n", tmp);
		exit (-1);
	}
}
