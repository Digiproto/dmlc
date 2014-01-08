/*
 * import.c:
 *
 * Copyright (C) 2013 Oubang Shen <shenoubang@gmail.com>
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

#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <libgen.h>
#include "import.h"
#include "ast.h"
#include "info_output.h"

#ifndef GDML_LIBRARY_DIR
#ifdef _WIN32
#define GDML_LIBRARY_DIR "..\\include\\gdml\\1.0"
#else
#define GDML_LIBRARY_DIR "../include/gdml/1.0"
#endif
#endif

/**
 * @brief : pre-import files of simics
 */
const char *import_file_list[] = {
	"dml-builtins.dml",
	"simics/C.dml",
	"simics/base/attr-value.dml",
	"simics/base/cbdata.dml",
	"simics/base/conf-object.dml",
	"simics/base/hap-producer.dml",
	"simics/base/memory.dml",
	"simics/base/time.dml",
	"simics/base/bigtime.dml",
	"simics/base/clock.dml",
	"simics/base/duration.dml",
	"simics/base/local-time.dml",
	"simics/base/memory-transaction.dml",
	"simics/base/sim-exception.dml",
	"simics/base/types.dml",
	"simics/base/callbacks.dml",
	"simics/base/configuration.dml",
	"simics/base/event.dml",
	"simics/base/log.dml",
	"simics/base/new-code-block.dml",
	"simics/base/sobject.dml",
	"simics/base/version.dml",
	"simics/processor/event.dml",
	"simics/processor/generic-spr.dml",
	"simics/processor/processor-platform.dml",
	"simics/processor/time.dml",
	"simics/processor/types.dml",
	"simics/obsolete/4_0.dml",
	"simics/obsolete/4_0-old-event-types.dml",
	"simics/obsolete/4_2.dml",
	"simics/obsolete/4_4.dml",
	"simics/obsolete/limited-dml-simics-api.dml",
	"simics/util/int128.dml",
	"simics/util/alloc.dml",
	"simics/util/bitcount.dml",
	"simics/util/dbuffer.dml",
	"simics/util/frags.dml",
	"simics/util/hashtab.dml",
	"simics/util/inet.dml",
	"simics/util/interval-set.dml",
	"simics/util/os.dml",
	"simics/util/prof-data.dml",
	"simics/util/str-vec.dml",
	"simics/util/vect.dml",
	"simics/util/arith.dml",
	"simics/util/data-structs.dml",
	"simics/util/fphex.dml",
	"simics/util/genrand.dml",
	"simics/util/help-macros.dml",
	"simics/util/log.dml",
	"simics/util/os-time.dml",
	"simics/util/strbuf.dml",
	"simics/util/swabber.dml",
	"simics/util/vtprintf.dml",
	"simics/devs/apic.dml",
	"simics/devs/ethernet.dml",
	"simics/devs/generic-message.dml",
	"simics/devs/io-memory.dml",
	"simics/devs/microwire.dml",
	"simics/devs/ram.dml",
	"simics/devs/signal.dml",
	"simics/devs/arinc-429.dml",
	"simics/devs/eth-probe.dml",
	"simics/devs/i2c.dml",
	"simics/devs/liblink.dml",
	"simics/devs/mii.dml",
	"simics/devs/rapidio.dml",
	"simics/devs/usb.dml",
	"simics/devs/coreint-async.dml",
	"simics/devs/firewire.dml",
	"simics/devs/ieee_802_3.dml",
	"simics/devs/map-demap.dml",
	"simics/devs/mil-std-1553.dml",
	"simics/devs/sata-interface.dml",
	"simics/devs/vectored-interrupt.dml",
	"simics/devs/coreint.dml",
	"simics/devs/frequency.dml",
	"simics/devs/interrupt-ack.dml",
	"simics/devs/memory-space-async.dml",
	"simics/devs/nand-flash.dml",
	"simics/devs/serial-device.dml",
	"simics/devs/datagram-link.dml",
	"simics/devs/gbic.dml",
	"simics/devs/interrupt.dml",
	"simics/devs/memory-space.dml",
	"simics/devs/pci.dml",
	"simics/devs/serial-peripheral-interface.dml",
	"simics/model-iface/abs-pointer.dml",
	"simics/model-iface/cdrom.dml",
	"simics/model-iface/decoder.dml",
	"simics/model-iface/gfx-console.dml",
	"simics/model-iface/mmc.dml",
	"simics/model-iface/save-state.dml",
	"simics/model-iface/vga-text-interface.dml",
	"simics/model-iface/arch-register-info.dml",
	"simics/model-iface/components.dml",
	"simics/model-iface/exec-trace.dml",
	"simics/model-iface/image.dml",
	"simics/model-iface/sim-keys.dml",
	"simics/model-iface/attribute-info.dml",
	"simics/model-iface/cpu-group.dml",
	"simics/model-iface/execute.dml",
	"simics/model-iface/int-register.dml",
	"simics/model-iface/state.dml",
	"simics/model-iface/cycle.dml",
	"simics/model-iface/frontend-server.dml",
	"simics/model-iface/kbd-interface.dml",
	"simics/model-iface/restricted-access.dml",
	"simics/model-iface/temporal-state.dml",
	"simics/simulator-iface/checkpoint.dml",
	"simics/simulator-iface/recorder.dml",
	"simics/simulator/oec-control.dml",
	"simics/simulator/output-pr-err.dml",
	"simics/simulator/sim-get-class.dml",
	"omit.dml",
	NULL
};

/**
 * @brief push_file_stack : store files name into stack
 *
 * @param top: the top about stack
 * @param name : dml source file name
 *
 * @return : the topest pointer about stack
 */
struct file_stack* push_file_stack(struct file_stack* top, const char* name)
{
	assert(name != NULL);

	/* charge file is in stack, if in, the file is compiled, no
	 * need to parsed it */
	if(top != NULL) {
		/* file stack is not empty. */
		struct file_stack* tmp = top->file_history;
		/* stop import, if the file have been imported before. */
		while(tmp != NULL) {
			if(strcmp(tmp->name, name) == 0) {
				return NULL;
			}
			tmp = tmp->next;
		}
	}

	struct file_stack* new_stack = gdml_zmalloc(sizeof(struct file_stack));
	new_stack->name = strdup(name);
	new_stack->next = top;
	struct file_stack* new_history = gdml_zmalloc(sizeof(struct file_stack));
	new_history->name = new_stack->name;
	new_history->next = NULL;

	if(top != NULL) {
		new_stack->file_history = top->file_history;
		/* insert new file name into file history list end.*/
		struct file_stack* tmp = top->file_history;
		while(tmp->next != NULL) {
			tmp = tmp->next;
		}
		tmp->next = new_history;
	}else{
		new_stack->file_history = new_history;
	}
	return new_stack;
}

/**
 * @brief pop_file_stack : pop top file, but not free it as
 * the error information will use file information
 *
 * @param top : the stach top
 *
 * @return : the topest point of stack after pop file
 */
struct file_stack* pop_file_stack(struct file_stack* top)
{
	assert(top != NULL);
	/* node may be used as error information, so it can't be free. */
	return top->next;
}

int list_get_len(char **dir_list)
{
	int i;
	/* list = [dir0, dir1, ...., dirn, NULL] len = n + 1*/
	if(dir_list) {
		for(i = 0; dir_list[i]; i++);
		return i;
	}
	return 0;
}

/**
 * @brief list_add_dir : add the import file path into two dimensional array
 *
 * @param dir_list : address of two dimensional array
 * @param dir : new import file patch
 *
 * @return : address of two dimensional array
 */
char** list_add_dir(char **dir_list, const char *dir)
{
	char **tmp;
	int len = list_get_len(dir_list);

	tmp = (char**) realloc(dir_list, sizeof(char*) * (len + 2));
	assert(tmp);
	tmp[len] = strdup(dir);
	tmp[len + 1] = NULL;
	return tmp;
}

/**
 * @brief link_dir_filename : merge the filename with file path
 *
 * @param buf : buffer to store new merge file with path
 * @param n :  length of file and path
 * @param dir : the directory of file
 * @param filename : file name
 *
 * @return : buffer about merge file with path
 */
int link_dir_filename(char* buf, size_t n, const char* dir, const char* filename)
{
	int rt;
#ifdef _WIN32
	rt = snprintf(buf, n, "%s\\%s", dir, filename);
#else
	rt = snprintf(buf, n, "%s/%s", dir, filename);
#endif
	if(rt >= n) {
		warning("dir \"%s\" and file name \"%s\" is too long.", dir, filename);
		return -1;
	}
	return 0;
}

/**
 * @brief get_library_dir : get the directory of exe file
 *
 * @param dir_r : exe file of user inputing
 *
 * @return : exe file with absolute path
 */
static const char* get_library_dir(const char *dir_r)
{
	int len;
	char tmp[DIR_MAX_LEN] = {0};

	/* if exe is on windows, dir is like "d:\mingw\msys\1.0\usr\bin\dmlc" */
#ifdef _WIN32
	/* exe is on windows */
	char *p = strrchr(dir_r, '\\');
	assert(p);
	len = p - dir_r + 1;
	memcpy(tmp, dir_r, len);
	tmp[len] = '\0';
#else
	/* exe is on linux */
	size_t rt = readlink("/proc/self/exe", tmp, DIR_MAX_LEN);
	if(rt < 0) {
		perror("readlink");
		exit(-1);
	}
	char *p = strrchr(tmp, '/');
	assert(p);
	len = p - tmp + 1;
	tmp[len] = '\0';
#endif

	/* set library by linking bin path */
	assert(len + strlen(GDML_LIBRARY_DIR) < DIR_MAX_LEN);
	char* tp = strcat(tmp, GDML_LIBRARY_DIR);
	assert(tp != NULL);
	gdml_library_dir =  strdup(tp);
	return gdml_library_dir;
}

/**
 * @brief get_file_dir : get the directory of exe file
 *
 * @param filename : name of exe file
 *
 * @return : the directory of exe file
 */
static const char* get_file_dir(const char* filename) {
	assert(filename != NULL);

	char tmp[DIR_MAX_LEN];
	int n = strlen(filename);
	if(n >= DIR_MAX_LEN) {
		error("filename is to long. (>= %d)", DIR_MAX_LEN);
	}
	strncpy(tmp, filename, n + 1);
	char* dir = dirname(tmp);

	return strdup(dir);
}

/**
 * @brief set_import_dir : set directory for finding default import file
 *
 * @param execname : the exe file name
 * @param fullname : the source file name
 * @param extradirs : the import directory of user add with '-I'
 */
void set_import_dir(const char* execname, const char* fullname, char** extradirs)
{
	const char  *library_dir = get_library_dir(execname);
	char       **extra_library_dirs = extradirs;
	const char  *file_dir = get_file_dir(fullname);
	int i, j, len = 0;

	if(extradirs) {
		len = list_get_len(extradirs);
		extra_library_dirs = gdml_zmalloc(sizeof(char*) * (len + 1));
		for(i = 0; i < len; i++) {
			/* format extra library directory. */
			char* tmp_str = strdup(extradirs[i]);
			char* org_str = extradirs[i];
			j = strlen(org_str);
			do{
				j--;
				if(org_str[j] != '\\' && org_str[j] != '/')
					break;
			}while(j >= 0);
			if(j < 0) {
				error("extra library directory %s is wrong.", org_str);
			}
			tmp_str[j + 1] = '\0';
			extra_library_dirs[i] = tmp_str;
		}
	}

	/* NULL is end. */
	if(extra_library_dirs) {
		import_dir_list = gdml_zmalloc((len + 2 + 1) * sizeof(char*));
		import_dir_list[0] = file_dir;
		for(i = 0; i < len; i++) {
			import_dir_list[1 + i] = extra_library_dirs[i];
		}
		import_dir_list[1 + len] = library_dir;
		import_dir_list[1 + len + 1] = NULL;
	}else{
		import_dir_list = gdml_zmalloc((2 + 1) * sizeof(char*));
		import_dir_list[0] = file_dir;
		import_dir_list[1] = library_dir;
		import_dir_list[2] = NULL;
	}

	int dirlen = strlen(gdml_library_dir);
	/* add the full patch about import file */
	for(i = 0; import_file_list[i] != NULL; i++) {
		len = dirlen + strlen(import_file_list[i]);
		char* file_path = (char*)gdml_zmalloc(len + 2);
		int rt = link_dir_filename(file_path, len + 2, gdml_library_dir, import_file_list[i]);
		assert(rt == 0);
		import_file_list[i] = file_path;
	}
}
