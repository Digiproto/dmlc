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
#include "platform_code_gen.h"
#include "gen_method_protos.h"
#include "gen_object.h"
extern void gen_device_footer(device_t *, FILE *);
/**
 * @brief gen_cfile : generate the c file about device
 *
 * @param dev : the object of device
 * @param out: the output file for device
 */
void gen_cfile(device_t *dev, const char *out) {
	const char *dev_name = dev->obj.name;
	char tmp[1024];
	FILE *f;
	
	snprintf(tmp, 1024, "%s/%s.c", out, dev_name);
	f = fopen(tmp, "w");
	if(f) {
		setbuf(f, NULL);
		pre_gen_code(dev, f);
		gen_device_init(dev, f);
		/*generate common code for all platforms */
		gen_dml_code(dev, f);
		gen_device_footer(dev, f);
		gen_device_type_info(dev, f);
		post_gen_code(dev, f);
		fclose(f);
	} else {
		printf("can not  open file %s\n", tmp);
	}
#if backend == 3 
	gen_platform_device_info(dev, out);
#endif
	snprintf(tmp, 1024, "%s/%s_protos.c", out, dev_name);
	f = fopen(tmp, "w");
	if(f) {
		setbuf(f, NULL);
		gen_device_method_protos(dev, f);
		fclose(f);
	} else {
		printf("can not open file %s\n", tmp);
	}
	gen_platform_device_module(dev, out);
}
