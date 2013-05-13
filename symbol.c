
/* Copyright (C) 
* 2012 - Michael.Kang blackfin.kang@gmail.com
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* as published by the Free Software Foundation; either version 2
* of the License, or (at your option) any later version.
* 
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
* 
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
* 
*/

/* 
* Symbol table management for gdml.
*/

/**
* @file symbol.c
* @brief 
* @author Michael.Kang blackfin.kang@gmail.com
* @version 7849
* @date 2013-05-13
*/

#include <stdio.h>				/* for (f)printf(), std{out,int} */
#include <stdlib.h>				/* for exit */
#include <string.h>				/* for strcmp, strdup & friends */
#include <assert.h>
#include "symbol.h"
#ifdef SYMBOL_DEBUG
#define DBG(fmt, ...) do { fprintf(stderr, fmt, ## __VA_ARGS__); } while (0)
#else
#define DBG(fmt, ...) do { } while (0)
#endif

//#define MAX_SYMBOLS 10000
static int str_hash (const char *str)
{
	if (!*str)
		return 0;
	int len = strlen (str);
	int i = 0, j = 0;
	int s[4] = { 0, 0, 0, 0 };
	for (; i < 4; i++) {
		j = i;
		while (j < len) {
			s[i] += (int) str[j];
			//printf("for s[%d]=%d, str[%d]=%d\n", i, s[i], j, str[j]);
			j += 4;
		}
		//printf("s[%d]=%d\n", i, s[i]);
	}
	int hash_value = (s[0] % 10) * 1000 + (s[1] % 10) * 100
		+ (s[2] % 10) * 10 + s[3] % 10;
	//printf("hash of %s is %d\n", str, hash_value);
	return hash_value;
}

/**
 * @brief find information of the symbol with same name and type on the hash table.
 *
 * @param symbol_table the hash table finding from.
 * @param name         the identifier name.
 * @param type         the symbol type
 *
 * @return return symbol structure pointer or NULL on error.
 */
static symbol_t _symbol_find(symbol_t* symbol_table, const char* name, type_t type) {
    symbol_t symbol = symbol_table[str_hash(name)];
	if (symbol) {
		/* hash conflict */
		while (((strcmp(symbol->name, name) != 0) || (symbol->type != type)) 
				&& (symbol != NULL)) {
			symbol = symbol->next;
		}
		if(symbol != NULL) {
			return symbol;
		}
	}
    return NULL;
}

/**
 * @brief find information of the symbol with same name on the hash table.
 *
 * @param symbol_table the hash table finding from.
 * @param name         the identifier name.
 *
 * @return return symbol structure pointer or NULL on error.
 */
static symbol_t _symbol_find_notype(symbol_t* symbol_table, const char* name) {
    symbol_t symbol = symbol_table[str_hash(name)];
	if (symbol) {
		/* hash conflict */
		while ((strcmp(symbol->name, name) != 0) 
				&& (symbol != NULL)) {
			symbol = symbol->next;
		}
		if(symbol != NULL) {
			return symbol;
		}
	}
    return NULL;
}

/**
 * @brief malloc a new table.
 *
 * @return return the table pointer.
 */
static symtab_t table_malloc()
{
    symtab_t new_symtab = (symtab_t) malloc(sizeof(struct symtab));
    assert(new_symtab != NULL);
    bzero(new_symtab, sizeof(struct symtab));
    return new_symtab;
}

/**
 * @brief malloc a new symbol and set number with name, type and attr.
 *
 * @param name the name of symbol.
 * @param type the type of symbol.
 * @param attr the pointer to attribute of symbol.
 *
 * @return return the table pointer.
 */
static symbol_t symbol_new(const char *name, type_t type, void *attr)
{
    symbol_t new_undef = (symbol_t) malloc(sizeof(struct symbol));
    assert(new_undef != NULL);
    if(name) {
        new_undef->name = strdup(name);
    }
    new_undef->type = type;
    new_undef->attr = attr;
    new_undef->next = NULL;
#ifdef SYMBOL_DEBUG
	new_undef->lnext = NULL;
#endif
    return new_undef;
}

/**
 * @brief find the symbol with same name from the current symbol table or parent.
 *
 * @param symtab the current symbol table.
 * @param name   the symbol name found.
 *
 * @return the pointer of the symbol with information.
 */
symbol_t symbol_find(symtab_t symtab, char *name, type_t type)
{
    assert(symtab != NULL && name != NULL);
    symtab_t tmp = symtab;
    symbol_t rt;
    while(tmp != NULL) {
        rt = _symbol_find(tmp->table, name, type);
        if(rt) {
            return rt;
        }
        tmp = tmp->parent;
    }
    return NULL;
}

/**
 * @brief insert a new symbol into the current table.
 *
 * @param symtab the current table.
 * @param name   the symbol name.
 * @param type   the symbol type.
 * @param attr   the pointer of the symbol attribute.
 *
 * @return return 0 or !0 on error.
 */
int symbol_insert(symtab_t symtab, const char* name, type_t type, void* attr)
{
    assert(name != NULL && symtab != NULL);

    symbol_t rt;
    rt = _symbol_find_notype(symtab->table, name);
    if(rt) {
        /* identifier have been defined while find stack top table
         * having identifier with same name and type.  */
        return -1;
    }

    symbol_t new_symbol = symbol_new(name, type, attr);

	DBG ("In %s, name = %s, type = %d, hash value = %d\n", __FUNCTION__, name, type,
			str_hash (name));
	int new_index = str_hash(name);
    symbol_t s = symtab->table[new_index];
    if(s == NULL){ /* blank slot */
        symtab->table[new_index] = new_symbol;
    }else{ /* conflict */
        while(s->next != NULL) {
            s = s->next;
		}
        s->next = new_symbol;
    }
#ifdef SYMBOL_DEBUG
	new_symbol->lnext = symtab->list;
	symtab->list = new_symbol;
#endif
    return 0;
}

/**
 * @brief create a symbol table root.
 *
 * @return return the symbol table.
 */
symtab_t symtab_create()
{
    return table_malloc();
}

/**
 * @brief create a symbol table into the sibling.
 *
 * @param the current symbol table.
 *
 * @return return the symbol table.
 */
symtab_t symtab_insert_sibling(symtab_t symtab)
{
    assert(symtab != NULL);
    symtab_t newtab = table_malloc();
    symtab_t tmp = symtab;
    while(tmp->sibling != NULL) {
        tmp = tmp->sibling;
    }
    tmp->sibling = newtab;
    newtab->parent = symtab->parent;

    return newtab;
}

/**
 * @brief create a symbol table into the child.
 *
 * @param the current symbol table.
 *
 * @return return the symbol table.
 */
symtab_t symtab_insert_child(symtab_t symtab)
{
    assert(symtab != NULL);
    symtab_t tmp = symtab->child;
    if(tmp) {
        return symtab_insert_sibling(tmp);
    }else{
        symtab_t newtab = table_malloc();
        symtab->child = newtab;
        newtab->parent = symtab;
        return newtab;
    }
}

/**
 * @brief free a symbol tree.
 *
 * @param root the tree root.
 */
void symtab_free(symtab_t root)
{
    assert(root != NULL);
    symtab_t tmp;
    while(root != NULL) {
        symtab_free(root->child);
        tmp = root;
        root = root->sibling;
        free(tmp);
    }
}

#ifdef SYMBOL_DEBUG
void get_all_symbol(symtab_t symtab, symbol_callback func_callback)
{
	symbol_t node = symtab->list;
	while(node != NULL) {
		func_callback(node);
		node = node->lnext;
	}
}
#endif

/**
 * @brief init a new symbol list undefined.
 *
 * @return return the symbol list head.
 */
symbol_t sym_undef_list_init()
{
    return symbol_new(NULL, 0, NULL);
}

/**
 * @brief add a new symbol undefind node into list.
 *
 * @param head   the list head.
 * @param symtab the symbol undefined on the symtab(symbol table).
 * @param name   the name of the symbol undefined.
 * @param type   the type of the symbol undefined.
 *
 * @return return the new node.
 */
symbol_t sym_undef_list_add(symbol_t head, symtab_t symtab, char *name, type_t type)
{
    assert(head != NULL && symtab != NULL);
    symbol_t new_undef = symbol_new(name, type, symtab);
    new_undef->next = head->next;
    head->next = new_undef;
    return new_undef;
}

/**
 * @brief get a symbol undefined, and delete it from list.
 *
 * @param head the list head.
 *
 * @return return a symbol undefined.
 */
symbol_t sym_undef_list_pick(symbol_t head)
{
    assert(head != NULL);
    symbol_t tmp = head->next;
    if(tmp) {
        head->next = tmp->next;
    }
    return tmp;
}

/**
 * @brief free the list.
 *
 * @param head the list head.
 */
void sym_undef_list_free(symbol_t head)
{
    symbol_t tmp = head->next;
    symbol_t pre;
    while(tmp != NULL) {
        pre = tmp;
        tmp = tmp->next;
        sym_undef_free(pre);
    }
    sym_undef_free(head);
}

/**
 * @brief free the a symbol undefined node.
 *
 * @param node the symbol undefined node.
 */
void sym_undef_free(symbol_t node)
{
    assert(node != NULL);
    free(node->name);
    free(node);
}

#ifdef SYMBOL_DEBUG
/* only for debug.  */
static void tree_travel(symtab_t node)
{
    static int depth = 0;
    depth++;
    while(node != NULL) {
        int i = 1;
        while(depth > i++) {
            printf("|   ");
        }
        printf("depth = %d\n", depth);
        tree_travel(node->child);
        node = node->sibling;
    }
    depth--;
}

static void list_travel(symbol_t head)
{
    symbol_t node = head->next;
    while(node != NULL) {
        printf("undef symbol: %s\n", node->name);
    }
}

static void found(symbol_t node)
{
    if(node) {
        printf("found: symbol->name: %s\n", node->name);
    }else{
        printf("not found.\n");
    }
}

static void callback_implement(symbol_t symbol)
{
	printf("symbol: name = %s, type = %d\n", symbol->name, symbol->type);
}
#endif
