/*
 * skyeye_object_headfile.c: 
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
#include "platform.h"
#include "gen_struct.h"

void gen_object_struct(device_t *dev, FILE *f) {
#if 0
    const char *dev_name = dev->obj.name;
    time_t timep;
    char *cap_name = to_upper(dev_name);

    time(&timep);
    fprintf(f, "/* Generated by dmlc, not to edit it */\n");
    fprintf(f, "/* %s */\n", asctime(localtime(&timep)));

    fprintf(f, "#ifndef %s_STRUCT_H\n", cap_name);
    fprintf(f, "#define %s_STRUCT_H\n", cap_name);
	fprintf(f, "\n");
    fprintf(f, "#include \"skyeye_obj.h\"\n");
    fprintf(f, "#include \"memory_space.h\"\n");
    fprintf(f, "#include \"skyeye_log.h\"\n");
	fprintf(f, "#include \"simics/dev_iface.h\"\n");
    fprintf(f, "\ntypedef struct %s %s_t;\n", dev_name, dev_name);
    gen_device_macros(dev, f);
    fprintf(f, "\nstruct %s {\n", dev_name);
    fprintf(f, "\tconf_object_t obj;\n");
    gen_device_struct(dev, f);
    /*more here */
    fprintf(f,"};\n");
#ifdef DEVICE_TEST 
	fprintf(f, "\nint debug_function_pos;\n");
    fprintf(f, "\n#define FUNC_HEAD \\\n"
			   "\tdo{ \\\n"
			   "\t\tint i; \\\n"
			   "\t\tfor(i = 0; i < debug_function_pos; i++) {printf(\"|---\");} \\\n"
			   "\t\tprintf(\"[DML] Into: %%s, %%s, %%d\\n\", __FILE__, __FUNCTION__, __LINE__); \\\n"
			   "\t\tdebug_function_pos++; \\\n"
			   "\t}while(0)\n");
    fprintf(f, "\n#define FUNC_END \\\n"
			   "\tdo{ \\\n"
			   "\t\tdebug_function_pos--; \\\n"
			   "\t\tint i; \\\n"
			   "\t\tfor(i = 0; i < debug_function_pos; i++) {printf(\"|---\");} \\\n"
			   "\t\tprintf(\"[DML] Exit: %%s, %%s, %%d\\n\", __FILE__, __FUNCTION__, __LINE__); \\\n"
			   "\t}while(0)\n");
#endif
    fprintf(f, "\n#endif\n");
    free(cap_name);
#endif
}
