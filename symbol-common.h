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

#define MAX_SYMBOLS 10000
#define SYMBOL_DEBUG

typedef struct symtab *symtab_t;

/* store identifier name, type and pointer to attribute_node.  */
typedef struct symbol {
    char          *name;  /* identifier name.  */
    type_t         type;  /* identifier type.  */
    struct symbol *next;  /* the other symbol with the same hash value */
	struct symbol *lnext; /* the symbol list for output. */
    union {
        void    *attr;   /* identifier declaration attribute.  */
        symtab_t belong; /* for undefined symbol list.  */
    };
}*symbol_t;

/* a hash table for storing symbols. it have a pointer to a brother,
 * have a pointer to a child.  */
struct symtab {
    struct symtab *parent;
    struct symtab *sibling;
    struct symtab *child;
	symbol_t list;
    symbol_t table[MAX_SYMBOLS];
	int table_num;
};

/* find and insert symbol from the symbol table.  */
symbol_t symbol_find(symtab_t symtab, char* name, type_t type);
symbol_t symbol_find_curr(symtab_t symtab, char* name, type_t type);
symbol_t symbol_find_notype(symtab_t symtab, char* name);
symbol_t symbol_find_curr_notype(symtab_t symtab, char* name);
int symbol_find_type_curr(symtab_t symtab, type_t type, symbol_t **result);
int symbol_insert(symtab_t symtab, const char* name, type_t type, void* attr);
/* operate the symbol tables.  */
symtab_t symtab_create();
symtab_t symtab_insert_sibling(symtab_t symtab, symtab_t newtab);
symtab_t symtab_insert_child(symtab_t symtab, symtab_t newtab);
void symtab_free(symtab_t symtab, int table_num);
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
