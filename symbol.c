
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
#include "stack.h"
#include "ast.h"
#ifdef SYMBOL_DEBUG
#define DBG(fmt, ...) do { DEBUG_SYMBOL(stderr, fmt, ## __VA_ARGS__); } while (0)
#else
#define DBG(fmt, ...) do { } while (0)
#endif


/**
 * @brief str_hash : calculate the hash value of string
 *
 * @param str: pointer to string
 *
 * @return : hash value
 */
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
			j += 4;
		}
	}
	int hash_value = (s[0] % 10) * 1000 + (s[1] % 10) * 100
		+ (s[2] % 10) * 10 + s[3] % 10;
	return hash_value;
}

static pre_parse_symbol_t* pre_symbol_table[MAX_SYMBOLS];

pre_parse_symbol_t* pre_symbol_find(const char* name) {
	assert(name != NULL);
	pre_parse_symbol_t* symbol = pre_symbol_table[str_hash(name)];
	if (symbol) {
		while((symbol != NULL) && (strcmp(symbol->name, name) != 0)) {
			symbol = symbol->next;
		}
		if (symbol != NULL) {
			return symbol;
		}
	}

	return NULL;
}

int pre_symbol_insert(pre_dml_t pre_dml) {
	char* name = pre_dml.name;

	pre_parse_symbol_t* rt = pre_symbol_find(name);
	if (rt) {
		return -1;
	}

	pre_parse_symbol_t* new_symbol = (pre_parse_symbol_t*)gdml_zmalloc(sizeof(pre_parse_symbol_t));
	new_symbol->name = strdup(name);
	new_symbol->type = pre_dml.type;

	int index = str_hash(name);
	pre_parse_symbol_t* s = pre_symbol_table[index];
	if (s == NULL) {
		pre_symbol_table[index] = new_symbol;
	}
	else {
		while (s->next != NULL) {
			s = s->next;
		}
		s->next = new_symbol;
	}

	return 0;
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
symbol_t _symbol_find(symbol_t* symbol_table, const char* name, type_t type) {
    symbol_t symbol = symbol_table[str_hash(name)];
	if (symbol) {
		/* hash conflict */
		while ((symbol != NULL) &&
				((strcmp(symbol->name, name) != 0) || (symbol->type != type))) {
			symbol = symbol->next;
		}
		if(symbol != NULL) {
			return symbol;
		}
	}
    return NULL;
}

symbol_t _symbol_find_type(symtab_t table,  type_t type) {
	/* hash conflict */
	symbol_t sym = table->list;
	while ((sym != NULL) &&
			(sym->type != type)) {
		sym = sym->lnext;
	}
	return sym;
}

symbol_t _symbol_find_not(symbol_t* symbol_table, const char* name, type_t type) {
    symbol_t symbol = symbol_table[str_hash(name)];
	if (symbol) {
		/* hash conflict */
		while ((symbol != NULL) &&
				((strcmp(symbol->name, name) != 0) || (symbol->type == type))) {
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
symbol_t _symbol_find_notype(symbol_t* symbol_table, const char* name) {

    symbol_t symbol = symbol_table[str_hash(name)];
	if (symbol) {
		/* hash conflict */
		while ((symbol != NULL) && (strcmp(symbol->name, name) != 0)) {
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
static symtab_t table_new(type_t type)
{
	static int table_count = 0;

	symtab_t new_symtab = (symtab_t) gdml_zmalloc(sizeof(struct symtab));
    assert(new_symtab != NULL);
	new_symtab->type = type;
	new_symtab->table_num = table_count++;
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
	symbol_t new_undef = (symbol_t) gdml_zmalloc(sizeof(struct symbol));
    assert(new_undef != NULL);
    if(name) {
        new_undef->name = strdup(name);
    }
    new_undef->type = type;
    new_undef->attr = attr;
    new_undef->next = NULL;
	new_undef->lnext = NULL;
    return new_undef;
}

/**
 * @brief symbol_find_from_templates : find symbol with type from templates
 *
 * @param templates : templates of table
 * @param name : name of symbol
 * @param type : type of symbol
 *
 * @return : pointer to symbol
 */
symbol_t symbol_find_from_templates(struct template_table* templates, const char* name, type_t type) {
	symbol_t rt;
	symtab_t table;

        while (templates != NULL) {
                table = templates->table;
                if(table) {
			rt = symbol_find(table, name, type);
			//rt = _symbol_find(table->table, name, type);
		if(rt) {
			return rt;
		}
                }
                templates = templates->next;
        }

	return NULL;
}

/**
 * @brief find the symbol with same name and type from the current symbol table or parent.
 *
 * @param symtab the current symbol table.
 * @param name   the symbol name found.
 * @param type   the symbol type found.
 *
 * @return the pointer of the symbol with information.
 */
symbol_t default_symbol_find(symtab_t symtab, const char* name, type_t type)
{
    assert(symtab != NULL && name != NULL);
    symtab_t tmp = symtab;
    symbol_t rt;
    if(tmp != NULL) {
		TABLE_TRACE("try to search symbol %s table %p, table num %d\n", name, tmp, tmp->table_num);
		rt = _symbol_find(tmp->table, name, type);
		if(rt) {
			TABLE_TRACE("symbol %s found in table %p, num %d\n", name, tmp, tmp->table_num);
		    return rt;
		}
		TABLE_TRACE("try to search templates %s\n", name);
		rt = symbol_find_from_templates(tmp->template_table, name, type);
		if (rt) {
			return rt;
		}
		tmp = tmp->parent;
		if(tmp) {
			TABLE_TRACE("goto parent table: %d to find %s\n", tmp->table_num, name);
			return symbol_find(tmp, name, type);
		}
    }
    return NULL;
}

/**
 * @brief symbol_find : find symbol with type from table
 *
 * @param symtab : table that symbol start to find
 * @param name : name of symbol
 * @param type : type of symbol
 *
 * @return : finded symbol
 */
symbol_t symbol_find(symtab_t symtab, const char *name, type_t type) {
	return symtab->cb(symtab, name, type);
}

/**
 * @brief find the symbol with same name and type from the current symbol table.
 *
 * @param symtab the current symbol table.
 * @param name   the symbol name found.
 * @param type   the symbol type found.
 *
 * @return the pointer of the symbol with information.
 */
symbol_t symbol_find_curr(symtab_t symtab, const char *name, type_t type)
{
    assert(symtab != NULL && name != NULL);
	symbol_t rt = NULL;

    rt = _symbol_find(symtab->table, name, type);
	if (rt) {
		return rt;
	}

	rt = symbol_find_from_templates(symtab->template_table, name, type);

	return rt;
}

/**
 * @brief symbol_find_from_templates_notype : find symbol without type from templates' table
 *
 * @param templates : templates of object table
 * @param name : symbol name
 *
 * @return : pointer to symbol
 */
symbol_t symbol_find_from_templates_notype(struct template_table* templates, const char* name) {
	symbol_t rt = NULL;
	symtab_t table = NULL;
	int i = 0;

	while (templates != NULL) {
		//table = templates->table->table;
		table = templates->table;
		check_template_parsed(templates->template_name);
		if (table) {
			rt = symbol_find_notype(table, name);
		}
		if(rt) {
		    return rt;
		}

		templates = templates->next;
		i++;
	}

	return NULL;
}

/**
 * @brief find the symbol with same name from the current symbol table or parent.
 *
 * @param symtab the current symbol table.
 * @param name   the symbol name found.
 *
 * @return the pointer of the symbol with information.
 */
symbol_t default_symbol_find_notype(symtab_t symtab, const char *name)
{
    assert(symtab != NULL && name != NULL);
    symtab_t tmp = symtab;
    symbol_t rt;
    if(tmp != NULL) {
		TABLE_TRACE("[notype]: try to find symbol %s, in %p, table_num %d\n", name, tmp, tmp->table_num);
        rt = _symbol_find_notype(tmp->table, name);
        if(rt) {
			TABLE_TRACE("[notype]: symbol %s found in table %p, num %d\n", name, tmp, tmp->table_num);
            return rt;
        }
	check_undef_template(tmp);
	TABLE_TRACE("[notype]: try to find symbol in templates\n");
	rt = symbol_find_from_templates_notype(tmp->template_table, name);
	if (rt) {
		TABLE_TRACE("[notype]: symbol %s found in template\n", name);
		return rt;
	}
    tmp = tmp->parent;
	if(tmp && tmp->type) {
		TABLE_TRACE("[notype]: goto to parent %p, table num %d to find  symbol %s\n", tmp, tmp->table_num, name);
		return symbol_find_notype(tmp, name);
	}
    }
    return NULL;
}

/**
 * @brief symbol_find_notype : find symbol without type from symbol table
 *
 * @param symtab : table that start to find symbol
 * @param name : name of symbol
 *
 * @return : pointer of symbol
 */
symbol_t symbol_find_notype(symtab_t symtab, const char *name) {
	return symtab->notype_cb(symtab, name);
}

/**
 * @brief find the symbol with same name from the current symbol table.
 *
 * @param symtab the current symbol table.
 * @param name   the symbol name found.
 *
 * @return the pointer of the symbol with information.
 */
symbol_t symbol_find_curr_notype(symtab_t symtab, const char *name)
{
    assert(symtab != NULL && name != NULL);
	symbol_t rt = NULL;

    rt = _symbol_find_notype(symtab->table, name);
	if (rt) {
		return rt;
	}
	return symbol_find_from_templates_notype(symtab->template_table, name);
}

/**
 * @brief symbol_find_type_curr : find the symbol with same type from the current symbol table
 *
 * @param symtab : the current symbol table.
 * @param type : the symbol type found.
 * @param result : the pointer to result list.
 *
 * @return : the number of result
 */
int symbol_find_type_curr(symtab_t symtab, type_t type, symbol_t **result)
{
    assert(symtab != NULL);
	symbol_t node = symtab->list;
	symbol_t new_symbol;
	symbol_t *list = NULL, *t;
	int num = 0;

	while(node != NULL) {
		if(node->type == type) {
			num++;
			t = realloc(list, sizeof(symbol_t) * num);
			assert(t != NULL);
			list = t;
			list[num - 1] = node;
		}
		node = node->lnext;
	}
	*result = list;
	return num;
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
    rt = _symbol_find(symtab->table, name, type);
    if(rt) {
        /* identifier have been defined while find stack top table
         * having identifier with same name and type.  */
        return -1;
    }

    symbol_t new_symbol = symbol_new(name, type, attr);

	if ((symtab->table_num) == 1) {
		DEBUG_TOP_LEVEL("In %s, name = %s, type = %d, hash value = %d, table num: %d, attr: 0x%X\n",
				__FUNCTION__, name, type, str_hash (name), symtab->table_num, attr);
	}
	else {
		DEBUG_OTHER_LEVEL("\tIn %s, name = %s, type = %d, hash value = %d, table num: %d\n",
				__FUNCTION__, name, type, str_hash (name), symtab->table_num);
	}
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

#if 1
   symbol_t tmp_symbol;
   // insert into the front of link list
   if(symtab->list == NULL) {
      symtab->list = new_symbol;
   }else{
       tmp_symbol = symtab->list;
       while(tmp_symbol->lnext != NULL) {
           tmp_symbol = tmp_symbol->lnext;
       }
       tmp_symbol->lnext = new_symbol;
   }
#else
   // insert into the last of link list
    new_symbol->lnext = symtab->list;
    symtab->list = new_symbol;
#endif

    return 0;
}

#include "object.h"

/**
 * @brief symbol_set_value : set attribute about symbol
 *
 * @param sym : pointer to symbol
 * @param attr : new attribute to assign symbol
 */
void symbol_set_value(symbol_t sym, void *attr) {
	if(sym) {
		sym->attr = attr;
	}
}

/**
 * @brief symbol_set_type : set the symbol type
 *
 * @param sym : pointer to symbol
 * @param type : the type to assign symbol
 */
void symbol_set_type(symbol_t sym, type_t type) {
	if(sym) {
		sym->type = type;
	}
}


/**
 * @brief create a symbol table root.
 *
 * @return return the symbol table.
 */
symtab_t symtab_create(type_t type)
{
	symtab_t table;

    table = table_new(type);
	if(table) {
		table->cb = default_symbol_find;
		table->notype_cb = default_symbol_find_notype;
	}
	return table;
}

/**
 * @brief symtab_create_with_cb : create table with finding method
 *
 * @param type : type of table
 * @param cb : method of finding symbol with type
 * @param notype_cb : method of finding symbol without type
 *
 * @return : pointer to symbol table
 */
symtab_t symtab_create_with_cb(type_t type, symbol_find_fn_t cb, symbol_find_notype_fn_t notype_cb)
{
	symtab_t table;

    table = table_new(type);
	if(table) {
		table->cb = cb;
		table->notype_cb = notype_cb;
	}
	return table;
}

/**
 * @brief sibling_table_free : free sibling of table
 *
 * @param symtab : the table of free sibling
 * @param table_num : table number
 */
void sibling_table_free(symtab_t symtab, int table_num) {
	symtab_t tmp = symtab;

	while (tmp->sibling) {
		if ((tmp->sibling->table_num) == table_num) {
			free(tmp->sibling);
			tmp->sibling = NULL;
			return ;
		}
		tmp = tmp->sibling;
	}

	return;
}

/**
 * @brief symtab_free : free symbol table
 *
 * @param symtab : the symbol table to be freed
 * @param table_num : table number
 */
void symtab_free(symtab_t symtab, int table_num) {
	if ((symtab->child->table_num) == table_num) {
		free(symtab->child);
		symtab->child = NULL;
	}
	else {
		sibling_table_free(symtab->child, table_num);
	}

	return;
}

/**
 * @brief create a symbol table into the sibling.
 *
 * @param the current symbol table.
 *
 * @return return the symbol table.
 */
symtab_t symtab_insert_sibling(symtab_t symtab, symtab_t newtab)
{
    assert(symtab != NULL);
    assert(newtab != NULL);
    symtab_t tmp = symtab;
    while(tmp->sibling != NULL) {
        tmp = tmp->sibling;
    }
    tmp->sibling = newtab;
    newtab->parent = symtab->parent;
	if ((newtab->no_check) == 0) {
		newtab->no_check = symtab->parent->no_check;
	}
	if (newtab->type == TEMPLATE_TYPE) {
		newtab->parent = NULL;
	}

    return newtab;
}

/**
 * @brief change_old_table : change old symbol table with new symbol table
 *
 * @param parent : parent of old table
 * @param old : old symbol table
 * @param new : new symbol table
 */
void change_old_table(symtab_t parent, symtab_t old, symtab_t new) {
	assert(parent != NULL); assert(old != NULL); assert(new != NULL);
	symtab_t* tmp = &(parent->child);
	symtab_t* pre = &(parent->child);
	int i = 0;
	while (*tmp != NULL) {
		if (*tmp == old) {
			*tmp = new;
			new->parent = old->parent;
			new->no_check = old->no_check;
			(*pre)->sibling = new;
			break;
		}
		else {
			pre = tmp;
			tmp = &((*tmp)->sibling);
		}
	}
	if (*tmp == NULL) {
		fprintf(stderr, "no such table(%d)\n", old->table_num);
		/* TODO: handle the error */
		exit(-1);
	}

	return;
}

/**
 * @brief create a symbol table into the child.
 *
 * @param the current symbol table.
 *
 * @return return the symbol table.
 */
symtab_t symtab_insert_child(symtab_t symtab, symtab_t newtab)
{
    assert(symtab != NULL);
    assert(newtab != NULL);
    symtab_t tmp = symtab->child;
    if(tmp) {
        return symtab_insert_sibling(tmp, newtab);
    }else{
        symtab->child = newtab;
        newtab->parent = symtab;
		if ((newtab->no_check) == 0) {
			newtab->no_check = symtab->no_check;
		}
        return newtab;
    }
}

/**
 * @brief : a static variable to store root table
 */
static symtab_t root_table;

/**
 * @brief set_root_table : assign root table with table
 *
 * @param table : symbol table
 */
void set_root_table(symtab_t table) {
	root_table = table;
}

/**
 * @brief get_root_table : get the root symbol table
 *
 * @return : pointer to root symbol table
 */
symtab_t get_root_table(void) {
	return root_table;
}

/**
 * @brief free a symbol tree.
 *
 * @param root the tree root.
 */
void symtab_free_all(symtab_t root)
{
    assert(root != NULL);
    symtab_t tmp;
    while(root != NULL) {
        symtab_free_all(root->child);
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
 * @brief print_all_symbol : print all symbol that in symbol table
 *
 * @param table : symbol table with symbol
 */
void print_all_symbol(symtab_t table) {
#if 0
	fprintf(stderr, "------------------start print table %d -----------------------\n", table->table_num);
	symbol_t symbol = table->list;
	const char *type;
	while(symbol != NULL) {
		type = sym_type2str[symbol->type];
		fprintf(stderr, "symbol: %s, type %s\n", symbol->name, type);
		symbol = symbol->lnext;
	}
	fprintf(stderr, "------------------finish print-----------------------\n");
#endif

	return ;
}

/**
 * @brief symbol_list_match : create a list chain to store all symbol that
 * match condition in symbol table
 *
 * @param tab : symbol table
 * @param match : the function to match symbol
 * @param arg : args about function
 *
 * @return : head of list
 */
static symbol_list_t *symbol_list_match(symtab_t tab, match_func_t match, void *arg) {
	symbol_list_t *first = NULL;
	symbol_list_t *cur = NULL;
	symbol_list_t *tmp = NULL;

	if (!tab) {
		return NULL;
	}

	symbol_t sym = tab->list;

	while(sym) {
		if (match(sym, arg)) {
			if(!first) {
				first = (symbol_list_t *)malloc(sizeof(*first));
				first->next = NULL;
				first->sym = sym;
				cur = first;
			} else {
				tmp = (symbol_list_t *)malloc(sizeof(*tmp));
				tmp->next = NULL;
				cur->next = tmp;
				tmp->sym = sym;
				cur = tmp;
			}
		}
		sym = sym->lnext;
	}
	return first;
}

/**
 * @brief list_join : add new symbol list to old list tail
 *
 * @param first : the old symbol list
 * @param new : new symbol list
 *
 * @return : head of merged list
 */
static symbol_list_t* list_join(symbol_list_t *first, symbol_list_t *new) {
	symbol_list_t *tmp;
	symbol_list_t *next;
	symbol_list_t *last;

	if(first) {
		last = first;
		tmp = first->next;
		while(tmp) {
			last = tmp;
			tmp = tmp->next;
		}
		last->next = new;
		return first;
	} else if(new) {
		return new;
	}
	return first;
}

/**
 * @brief _symbol_list_find : crate list to store symbol that match condition symbol
 *
 * @param tab : symbol table
 * @param match : function pointer for match function
 * @param arg : args of match function
 * @param depth: the depth of finding symbol, the 1, only find symbol in table self
 * else find symbol from table and templates
 *
 * @return : head of merged list
 */
symbol_list_t *_symbol_list_find(symtab_t tab, match_func_t match, void *arg, int depth) {
	symbol_list_t *first = NULL;
	symbol_list_t *tmp = NULL;
	struct template_table *tmpl;
	symtab_t table;

	/*find in table*/
	if(!tab) {
		TABLE_LIST_TRACE("table empty return NULL\n");	
		return first;
	}
	TABLE_LIST_TRACE("try local table %p, table num %d\n", tab, tab->table_num);	
	first = symbol_list_match(tab, match, arg);
	if(depth == 1) {
		TABLE_LIST_TRACE("just find in local table , with depth(1)\n");
		return first;
	}
	TABLE_LIST_TRACE("try table templates, first %p\n", first);
	tmpl = tab->template_table;
	while(tmpl) {
		table = tmpl->table;
		//tmp = symbol_list_match(table, match, arg);
		TABLE_LIST_TRACE("try to find in template %p, table num %d, name %s\n", table, table->table_num, tmpl->template_name);
		//print_all_symbol(table);
		tmp = _symbol_list_find(table, match, arg, depth+1);
		if(tmp) {
			first = list_join(first, tmp);
		}
		tmpl = tmpl->next;
	}
	TABLE_LIST_TRACE("end search table templates\n");
	return first;
}

/**
 * @brief match_type : check the symbol type is the same with args
 *
 * @param sym : pointer to symbol
 * @param arg : type that for matching
 *
 * @return : 1 - same type
 *			0 - no same type
 */
static int match_type(symbol_t sym, void *arg) {
	type_t type = (type_t )arg;
	if(sym->type == type) {
		return 1;
	} else {
		return 0;
	}
}

/**
 * @brief match_name : check symbol name is the same with args
 *
 * @param sym : pointer to symbol
 * @param arg : pointer to name that matching
 *
 * @return : 1 - same name
 *			0 - no same name
 */
static int match_name(symbol_t sym, void *arg) {
	const char *name = arg;

	if(!strcmp(sym->name, name)) {
		return 1;
	} else {
		return 0;
	}
}

/**
 * @brief match_specific : check the type and name are same with args
 *
 * @param sym : pointer to symbol
 * @param arg : args with name and type
 *
 * @return  : 1 - same type and name
 *			0 - not same with type or name
 */
static int match_specific(symbol_t sym, void *arg) {
	search_arg_t *sarg = arg;

	if(sarg->type == sym->type && !strcmp(sarg->name, sym->name)) {
		return 1;
	}
	return 0;
}

/**
 * @brief symbol_list_find_type : create a list chain to store all symbols
 * with same type from table and its templates
 *
 * @param tab : symbol table to find symbol
 * @param type : type for finding symbol
 *
 * @return : head of list chain
 */
symbol_list_t *symbol_list_find_type(symtab_t tab, type_t type) {
	return _symbol_list_find(tab, match_type, (void *) type, 2);
}

/**
 * @brief symbol_list_find_name : create a list chain to store all symbols
 * with same name from table and its templates.
 *
 * @param tab : symbol table to find symbol
 * @param name : name for finding symbol
 *
 * @return  : head of list chain
 */
symbol_list_t *symbol_list_find_name(symtab_t tab, const char *name) {
	return _symbol_list_find(tab, match_name, (void*)name, 2);
}

/**
 * @brief symbol_list_find_full : create a list chain to store all symbols
 * with same type and name from table and its templates. this thain always have only one element
 *
 * @param tab : symbol tale to find symbol
 * @param name: name for finding
 * @param type : type for finding
 *
 * @return  : head os list chain
 */
symbol_list_t *symbol_list_find_full(symtab_t tab, const char *name, type_t type) {
	search_arg_t arg = {.name = name, .type = type};

	return _symbol_list_find(tab, match_specific, (void *)&arg, 2);
}

/**
 * @brief local_list_find_type : create a list chain to store all symbols
 * with same type from table itself
 *
 * @param tab : symbol table to find symbol
 * @param type : type for finding symbol
 *
 * @return : head of list chain
 */
symbol_list_t *local_list_find_type(symtab_t tab, type_t type) {
	return _symbol_list_find(tab, match_type, (void *)type, 1);
}

/**
 * @brief : local_list_find_name create a list chain to store all symbols
 * with same name from table itself. this chain always only have
 * one element
 *
 * @param tab : symbol table to find symbol
 * @param name : name for finding symbol
 *
 * @return  : head of list chain
 */
symbol_list_t* local_list_find_name(symtab_t tab, const char* name) {
	return _symbol_list_find(tab, match_name, (void*)name, 1);
}

/**
 * @brief local_list_find_full: create a list chain to store all symbols
 * with same type and name from table itself. this chain always have
 * only one element
 *
 * @param tab : symbol tale to find symbol
 * @param name: name for finding
 * @param type : type for finding
 *
 * @return  : head os list chain
 */
symbol_list_t* local_list_find_full(symtab_t tab, const char* name, type_t type) {
	search_arg_t arg = {.name = name, .type = type};

	return _symbol_list_find(tab, match_specific, (void *)&arg, 1);
}

/**
 * @brief symbol_list_free : free symbol list
 *
 * @param list : head of symbol list
 */
void symbol_list_free(symbol_list_t *list) {
	symbol_list_t *tmp;
	symbol_list_t *old;

	tmp = list;
	while(tmp) {
		old = tmp;
		tmp = tmp->next;
		free(old);
	}
}

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

/**
 * @brief params_insert_table : insert method parameters into method table
 *
 * @param table : method table
 * @param method_params : method parameters
 */
void params_insert_table(symtab_t table, method_params_t* method_params) {
	assert(table != NULL);
	if (method_params == NULL) {
		return;
	}
	int i = 0;
	int in_argc = method_params->in_argc;
	int ret_argc = method_params->ret_argc;
	params_t** in_list = method_params->in_list;
	params_t** ret_list = method_params->ret_list;
	cdecl_t* param_decl = NULL;

	for (i = 0; i < in_argc; i++) {
		param_decl = in_list[i]->decl;
		if (symbol_insert(table, in_list[i]->var_name, PARAM_TYPE, in_list[i]) == -1) {
			DEBUG_SYMBOL("method param : %s redefined\n", in_list[i]->var_name);
			/* FIXME: handle the error */
			exit(-1);
		}
	}

	for (i = 0; i < ret_argc; i++) {
		param_decl = ret_list[i]->decl;
		if (symbol_insert(table, ret_list[i]->var_name, PARAM_TYPE, ret_list[i]) == -1) {
			DEBUG_SYMBOL("method param : %s redefined\n", in_list[i]->var_name);
			/* FIXME: handle the error */
			exit(-1);
		}
	}

	return;
}

/**
 * @brief change_table : change current table with new table, and push current table into table stack
 *
 * @param current_table : the current table in parsing
 * @param table_stack : stack of table
 * @param current_table_num : current table number
 * @param type : new table type
 *
 * @return : pointer to new table
 */
symtab_t change_table(symtab_t current_table, stack_t* table_stack, type_t type) {
	assert(current_table != NULL);
	assert(table_stack != NULL);

	symtab_t table = symtab_create(type);
	symtab_insert_child(current_table, table);
	push(table_stack, current_table);

	return table;
}

/**
 * @brief undef_var_insert : insert undefined variable node into table.
 * in simics, varibale and be used before defined, so we should insert
 * the undefined variable into table for second checking.
 *
 * @param table : table of block
 * @param node : tree node of undefined symbol
 */
void undef_var_insert(symtab_t table, tree_t* node) {
	assert(table != NULL); assert(node != NULL);

	undef_var_t* var = (undef_var_t*)gdml_zmalloc(sizeof(undef_var_t));
	var->node= node;

	if (table->undef_list == NULL) {
		table->undef_list = var;
	}
	else {
		undef_var_t* tmp = table->undef_list;
		while (tmp->next != NULL) {
			tmp = tmp->next;
		}
		tmp->next = var;
	}

	return;
}

/**
 * @brief symbol_defined : check symbol is defined in symbol table itself
 *
 * @param table : table that defined symbol
 * @param name : name of symbol
 *
 * @return 1: symbol is defined in table
 *			0: symbol is no defined in table
 */
int symbol_defined(symtab_t table, const char* name) {
    assert(table != NULL);
    assert(name != NULL);

    symbol_t symbol = _symbol_find_notype(table->table, name);

    if (symbol)
        return 1;
    else
        return 0;
}

int symbol_defined_type(symtab_t table, const char* name, type_t type) {
    assert(table != NULL);
    assert(name != NULL);

    symbol_t symbol = _symbol_find(table->table, name, type);

    if (symbol)
        return 1;
    else
        return 0;
}

/**
 * @brief defined_symbol : get the defined symbols in table
 *
 * @param table : table that defined symbol
 * @param name : name of symbol
 * @param type : type of symbol, if type is 0, find symbol without type
 *
 * @return : pointer to symbol
 */
symbol_t defined_symbol(symtab_t table, const char* name, type_t type) {
       assert(table != NULL); assert(name != NULL);
       symbol_t symbol = NULL;
	if (type) {
		symbol = _symbol_find(table->table, name, type);
	} else {
		symbol = _symbol_find_notype(table->table, name);
	}

       return symbol;
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

#endif
