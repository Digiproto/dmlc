/*
 * gen_utility.h: 
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

#ifndef __GEN_UTILITY_H__ 
#define __GEN_UTILITY_H__  

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "ast.h"
#include "gen_debug.h"

#define TMP_SIZE 512
extern FILE *out;
int get_local_index(void);
int get_exit_index(void);
int get_throw_index(void);
int get_label_index(void);
int get_tmp_index(void);

const char *get_symbol_alias(symbol_t sym);
void set_symbol_alias(symbol_t sym,int type,int index);
void set_symbol_ret_alias(symbol_t sym);
void set_symbol_alias_name(symbol_t sym, char *name);
void set_exit_symbol_alias(symbol_t sym);
void set_throw_symbol_alias(symbol_t sym);
void set_label_symbol_alias(symbol_t sym);

void enter_scope(void);
void exit_scope(void);
const char *get_tabstr(void);
void tabcount_set(int num);
void tabcount_add(int num);
void tabcount_sub(int num);
#define D(fmt,...) fprintf(out, fmt,  ## __VA_ARGS__)
#define POS fprintf(out, "%s", get_tabstr())

//#define DEVICE_TEST
#ifdef DEVICE_TEST
#define D_HEAD do{D("\n\tFUNC_HEAD;\n");}while(0)
#define D_END  do{D("\n\tFUNC_END;\n");}while(0)
#define F_HEAD do{fprintf(f, "\n\tFUNC_HEAD;\n");}while(0)
#define F_END  do{fprintf(f, "\n\tFUNC_END;\n");}while(0)
#else // else DEVICE_TEST
#define D_HEAD do{}while(0)
#define D_END  do{}while(0)
#define F_HEAD do{}while(0)
#define F_END  do{}while(0)
#endif // endif DEVICE_TEST

static void new_line(void) {
    D("\n");
}

static void translate(tree_t *node) {
    if(node->common.translate) {
        node->common.translate(node);
    }
}

char *to_upper(const char *src);
const char *size2str(int size);
const char *bits2str(int bits);
int adjust_size(int size);
int is_simics_api(const char *name);
static void gen_src_loc(YYLTYPE *l) {
    if(l) {
        POS;
        D("#line %d \"%s\"", l->first_line, l->file->name);
    }
    new_line();
}
#endif /* __GEN_UTILITY_H__ */

