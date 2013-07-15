/*
 * gen_utility.c: 
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

#include "gen_utility.h"

static char tab[16][32] =
    { {"\0"}, {"\t"}, {"\t\t"}, {"\t\t\t"}, {"\t\t\t\t"}, {"\t\t\t\t\t"}, {"\t\t\t\t\t\t"}, {"\t\t\t\t\t\t\t"}, {"\t\t\t\t\t\t\t\t"} };

const char *simics_apis[] = {
	"SIM_mem_op_is_read",
	"SIM_mem_op_set_value",
	"SIM_mem_op_get_value",
	NULL
};

const char *simics_typedef[] = {
	"generic_transaction_t",
	NULL
};

static int tab_count;
static int local_index = 1;
static int exit_index  = 1;
static int label_index = 1;
static int tmp_index = 1;
static int throw_index = 1;

void enter_scope(void){
    D("{\n");
    tab_count++;
}

void exit_scope(void){
    tab_count--;
	POS;
    D("} ");
}

void tabcount_set(int num) {
	tab_count = num;
}

void tabcount_add(int num) {
	tab_count += num;
}
 
void tabcount_sub(int num) {
	tab_count -= num;
}
const char *get_tabstr(void){
	return tab[tab_count];
}

int get_local_index(void){
	    return local_index++;
}

int get_exit_index(void){
	    return exit_index++;
}

int get_throw_index(void) {
	    return throw_index++;
}

int get_label_index(void) {
	    return label_index++;
}

int get_tmp_index(void) {
	    return tmp_index++;
}

const char *get_symbol_alias(symbol_t sym) {
    int i;
    int ret;
    char tmp[TMP_SIZE];
    int len;
    void *p;

    if(!sym->alias_name) {
        i = get_local_index();
        snprintf(tmp, TMP_SIZE, "v%d_%s", i, sym->name);
        len = strlen(tmp);
        p = gdml_zmalloc(len + 1);
        p = memcpy(p, tmp, len);
        sym->alias_name = p;
    }
    return sym->alias_name;
}

void set_symbol_alias(symbol_t sym,int type,int index) {
    char tmp[TMP_SIZE];
    int len;
    void *p;

    if(!sym->alias_name) {
        int ret;
        if(type == 0){
            snprintf(tmp, TMP_SIZE, "v%s", sym->name);
            len = strlen(tmp);
            p = gdml_zmalloc(len + 1);
            p = strncat(p, tmp, TMP_SIZE);
            sym->alias_name = p;
        } else if (type == 1) {
            snprintf(tmp, TMP_SIZE, "*%s", sym->name);
            len = strlen(tmp);
            p = gdml_zmalloc(len + 1);
            p = strncat(p, tmp, TMP_SIZE);
            sym->alias_name = p;
        } else {
            snprintf(tmp, TMP_SIZE, "v%d_%s", index,  sym->name);
            len = strlen(tmp);
            p = gdml_zmalloc(len + 1);
            p = strncat(p, tmp, TMP_SIZE);
            sym->alias_name = p;
        }
    }
}

void set_symbol_ret_alias(symbol_t sym) {
    int index;
    int ret;
    char tmp[TMP_SIZE];
    int len;
    void *p;

    if(!sym->alias_name) {
        index = get_local_index();
        snprintf(tmp, TMP_SIZE, "v%d__ret_%s", index,  sym->name);
        len = strlen(tmp);
        p = gdml_zmalloc(len + 1);
        p = strncat(p, tmp, TMP_SIZE);
        sym->alias_name = p;
    }
}

/*name must dynamically alloced */
void set_symbol_alias_name(symbol_t sym, char *name) {
    if(!sym->alias_name) {
        sym->alias_name = name;
    }
}

void set_exit_symbol_alias(symbol_t sym){
    int index;
    int ret;
    char tmp[TMP_SIZE];
    int len;
    void *p;

    if(sym) {
        if(!sym->alias_name) {
            index = get_exit_index();
            snprintf(tmp, TMP_SIZE, "%s%d", "exit", index);
            len = strlen(tmp);
            p = gdml_zmalloc(len + 1);
            p = strncat(p, tmp, TMP_SIZE);
            sym->alias_name = p;
        }
    }
}

void set_throw_symbol_alias(symbol_t sym){
    int index;
    int ret;
    char tmp[TMP_SIZE];
    int len;
    void *p;

    if(sym) {
        if(!sym->alias_name) {
            index = get_throw_index();
            snprintf(tmp, TMP_SIZE, "%s%d", "throw", index);
            len = strlen(tmp);
            p = gdml_zmalloc(len + 1);
            p = strncat(p, tmp, TMP_SIZE);
            sym->alias_name = p;
        }
    }
}

void set_label_symbol_alias(symbol_t sym){
    int index;
    int ret;
    char tmp[TMP_SIZE];
    int len;
    void *p;

    if(sym) {
        if(!sym->alias_name) {
            index = get_label_index();
            snprintf(tmp, TMP_SIZE, "%s%d", "label", index);
            len = strlen(tmp);
            p = gdml_zmalloc(len + 1);
            p = strncat(p, tmp, TMP_SIZE);
            sym->alias_name = p;
        }
    }
}

char *to_upper(const char *src) {
    int len = strlen(src);
    char *dst = gdml_zmalloc(len + 1);
    int i;

    for(i = 0; i < len; i++) {
        dst[i] = toupper(src[i]);
    }
    dst[len] = '\0';
    return dst;
}

enum size_spec {
    ENUM_SZ_1,
    ENUM_SZ_2,
    ENUM_SZ_4,
    ENUM_SZ_8
}size_spec_t;

const char* sizestr[] = {
    [ENUM_SZ_1] = "uint8",
    [ENUM_SZ_2] = "uint16",
    [ENUM_SZ_4] = "uint32",
    [ENUM_SZ_8] = "uint64"
};

const char *size2str(int size) {
    switch (size) {
        case 1:
            return sizestr[ENUM_SZ_1];
        case 2:
            return sizestr[ENUM_SZ_2];
        case 4:
            return sizestr[ENUM_SZ_4];
        case 8:
            return sizestr[ENUM_SZ_8];
        default:
            return sizestr[ENUM_SZ_4];
    }
    return "";
}

int bits2byte(int bits) {
	if(bits %8 == 0) {
		return bits/8;
	} else {
		return bits/8 + 1;
	}
}

const char *bits2str(int bits) {
	int byte;

	byte = bits2byte(bits);
	return size2str(byte);
}

int adjust_size(int size) {
    if(size == 3) {
        return 4;
    }
    if((size > 4) && (size < 8)) {
        return 8;
    }
    return size;
}

int is_simics_api(const char *name) {
	const char *tmp;
	int i = 0;

	tmp = simics_apis[0];
	while(tmp) {
		if(!strcmp(tmp, name))
			return 1;
		i++;
		tmp = simics_apis[i];
	}
	return 0;
}
