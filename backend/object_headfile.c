/*
 * qemu_object_headfile.c: 
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
#include <string.h>
#include "object_headfile.h"
#include "gen_struct.h"
#include "gen_utility.h"

const char *headfile_dir = "simics";
static void gen_object_headfile(device_t *dev, FILE *f) {
    const char *dev_name = dev->obj.name;
    char *captical_name;
    time_t timep;

    captical_name = to_upper(dev_name);

    time(&timep);
    fprintf(f, "/* Generated by dmlc, not to edit it */\n");
    fprintf(f, "/* %s */\n", asctime(localtime(&timep)));
    fprintf(f, "\n");
    fprintf(f, "#ifndef %s_H\n", captical_name);
    fprintf(f, "#define %s_H\n", captical_name);
    fprintf(f, "\n");
#if backend != 3
    fprintf(f, "#include \"%s/base_types.h\"\n", headfile_dir);
    fprintf(f, "#include \"%s/generic_transaction.h\"\n", headfile_dir);
	fprintf(f, "#include \"%s/dev_iface.h\"", headfile_dir);
#else 
    fprintf(f, "#include <%s/base_types.h>\n", headfile_dir);
	fprintf(f, "#include <%s/core/object_model.h>\n", headfile_dir);
	fprintf(f, "#include <%s/core/object_class.h>\n", headfile_dir);
    fprintf(f, "#include <%s/generic_transaction.h>\n", headfile_dir);
	fprintf(f, "#include <%s/bank_access.h>\n", headfile_dir);
	fprintf(f, "#include <%s/dev_iface.h>\n", headfile_dir);
#endif
    fprintf(f, "#include \"%s_struct.h\"\n", dev_name);
    fprintf(f, "\n");
    gen_device_loggroup(dev, f);
    fprintf(f, "\n");
    fprintf(f, "void %s_hard_reset(%s_t *_obj);\n", dev_name, dev_name);
    fprintf(f, "void %s_soft_reset(%s_t *_obj);\n", dev_name, dev_name);
    fprintf(f, "\n#endif\n");
    free(captical_name);
}

void gen_headerfiles(device_t *dev, const char *out_dir) {
	FILE *f;
	char tmp[1024];	
	const char *dev_name = dev->obj.name;
	
	snprintf(tmp, 1024, "%s/%s.h", out_dir, dev_name);

	f = fopen(tmp, "w");
	if(f) {
		gen_object_headfile(dev, f);
		fclose(f);
	} else {
		printf("cannot open %s\n", tmp);
	}

	snprintf(tmp, 1024, "%s/%s_struct.h", out_dir, dev_name);
	f = fopen(tmp, "w");
	if(f) {
		gen_object_struct(dev, f);
		fclose(f);
	} else {
		printf("cannot open %s\n", tmp);
	}	
}

