/*
 * symbol-common.h:
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

#ifndef __SYMBOL_COMMON_H__
#define __SYMBOL_COMMON_H__

#include "types.h"
#include "pre_parse_dml.h"

#define MAX_SYMBOLS 10000
#define SYMBOL_DEBUG

typedef struct symtab *symtab_t;

/* store identifier name, type and pointer to attribute_node.  */
typedef struct symbol {
    char          *name;  /* identifier name.  */
	char *alias_name;
	void *owner;
    type_t         type;  /* identifier type.  */
    struct symbol *next;  /* the other symbol with the same hash value */
	struct symbol *lnext; /* the symbol list for output. */
    union {
        void    *attr;   /* identifier declaration attribute.  */
        symtab_t belong; /* for undefined symbol list.  */
    };
}*symbol_t;

struct template_table {
	char* template_name;
	struct symtab *table;
	struct template_table *next;
};

typedef struct undef_var {
	//const char* var_name;
	void* node;
	struct undef_var* next;
}undef_var_t;

typedef struct undef_template {
        const char* name;
        struct undef_template* next;
} undef_template_t;

typedef int (*match_func_t) (symbol_t sym, void *arg);
typedef symbol_t (*symbol_find_fn_t)(symtab_t tab, const char *name, type_t type);
typedef symbol_t (*symbol_find_notype_fn_t)(symtab_t tab, const char *name);
/* a hash table for storing symbols. it have a pointer to a brother,
 * have a pointer to a child.  */
struct symtab {
    struct symtab *parent;
    struct symtab *sibling;
    struct symtab *child;
	struct template_table* template_table;
	undef_template_t* undef_temp;
	symbol_t list;
    symbol_t table[MAX_SYMBOLS];
	undef_var_t* undef_list;
	int table_num;
	int pass_num;
	type_t type;
	int no_check;
	int is_parsed;
	void *obj;
	symbol_find_fn_t cb;
	symbol_find_notype_fn_t notype_cb;
};

typedef struct symbol_list {
	struct symbol_list *next;
	symbol_t sym;
} symbol_list_t;

typedef struct pre_parse_symbol {
	char* name;
	type_t type;
	struct pre_parse_symbol* next;
}pre_parse_symbol_t;

typedef struct search_arg {
	const char *name;
	type_t type;
} search_arg_t;

/* find and insert symbol from the symbol table.  */
symbol_t default_symbol_find(symtab_t symtab, const char* name, type_t type);
symbol_t symbol_find(symtab_t symtab, const char* name, type_t type);
symbol_t _symbol_find(symbol_t *table, const char* name, type_t type);
symbol_t _symbol_find_notype(symbol_t* symbol_table, const char* name);
symbol_t symbol_find_from_templates(struct template_table *table, const char *name, type_t type);
symbol_t symbol_find_curr(symtab_t symtab, const char* name, type_t type);
symbol_t symbol_find_notype(symtab_t symtab, const char* name);
symbol_t symbol_find_curr_notype(symtab_t symtab, const char* name);
symbol_list_t *symbol_list_find_type(symtab_t tab, type_t type);
symbol_list_t *symbol_list_find_name(symtab_t tab, const char *name);
symbol_list_t *symbol_list_find_full(symtab_t tab, const char *name, type_t type);
symbol_list_t *local_list_find_type(symtab_t tab, type_t type);
void symbol_list_free(symbol_list_t *list);
int symbol_find_type_curr(symtab_t symtab, type_t type, symbol_t **result);
int symbol_insert(symtab_t symtab, const char* name, type_t type, void* attr);
void symbol_set_value(symbol_t sym, void *attr);
void symbol_set_type(symbol_t sym, type_t type);
void set_root_table(symtab_t table);
symtab_t get_root_table(void);
/* operate the symbol tables.  */
symtab_t symtab_create(type_t type);
symtab_t symtab_create_with_cb(type_t type, symbol_find_fn_t cb, symbol_find_notype_fn_t notype);
symtab_t symtab_insert_sibling(symtab_t symtab, symtab_t newtab);
symtab_t symtab_insert_child(symtab_t symtab, symtab_t newtab);
void change_old_table(symtab_t parent, symtab_t old, symtab_t new);
void symtab_free(symtab_t symtab, int table_num);

int pre_symbol_insert(pre_dml_t pre_dml);
pre_parse_symbol_t* pre_symbol_find(const char* name);
int symbol_defined(symtab_t table, const char* name);
symbol_t defined_symbol(symtab_t table, const char* name, type_t type);

#ifdef SYMBOL_DEBUG
typedef void (*symbol_callback)(symbol_t symbol);
void get_all_symbol(symtab_t symtab, symbol_callback f);
#endif

/* undefined symbol list.  */
symbol_t sym_undef_list_init();
symbol_t sym_undef_list_add (symbol_t head, symtab_t table,
        char *name, type_t type);
symbol_t sym_undef_list_pick(symbol_t head);
void sym_undef_list_free(symbol_t head);
void sym_undef_free(symbol_t node);

#endif /* __SYMBOL_COMMON_H__ */
